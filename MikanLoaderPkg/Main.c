#include <Uefi.h>
#include <elf.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>
#include <Guid/FileInfo.h>
#include "frame_buffer_config.hpp"

void Halt(void)
{
	while (1)
	{
		__asm__("hlt");
	}
}

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root)
{
	EFI_STATUS status;
	EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs;

	status = gBS->OpenProtocol(
		image_handle,
		&gEfiLoadedImageProtocolGuid,
		(VOID **)&loaded_image,
		image_handle,
		NULL,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (EFI_ERROR(status))
	{
		return status;
	}

	status = gBS->OpenProtocol(
		loaded_image->DeviceHandle,
		&gEfiSimpleFileSystemProtocolGuid,
		(VOID **)&fs,
		image_handle,
		NULL,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (EFI_ERROR(status))
	{
		return status;
	}

	return fs->OpenVolume(fs, root);
}

EFI_STATUS OpenGOP(EFI_HANDLE image_handle,
				   EFI_GRAPHICS_OUTPUT_PROTOCOL **gop)
{
	EFI_STATUS status;
	UINTN num_gop_handles = 0;
	EFI_HANDLE *gop_handles = NULL;

	status = gBS->LocateHandleBuffer(
		ByProtocol,
		&gEfiGraphicsOutputProtocolGuid,
		NULL,
		&num_gop_handles,
		&gop_handles);
	if (EFI_ERROR(status))
	{
		return status;
	}

	status = gBS->OpenProtocol(
		gop_handles[0],
		&gEfiGraphicsOutputProtocolGuid,
		(VOID **)gop,
		image_handle,
		NULL,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (EFI_ERROR(status))
	{
		return status;
	}

	FreePool(gop_handles);

	return EFI_SUCCESS;
}

void CalcLoadAddressRange(Elf64_Ehdr *ehdr, UINT64 *first, UINT64 *last)
{
	Elf64_Phdr *phdr = (Elf64_Phdr *)((UINT64)ehdr + ehdr->e_phoff);
	*first = MAX_UINT64;
	*last = 0;

	for (Elf64_Half i = 0; i < ehdr->e_phnum; ++i)
	{
		if (phdr[i].p_type != PT_LOAD)
		{
			continue;
		}

		*first = MIN(*first, phdr[i].p_vaddr);
		*last = MAX(*last, phdr[i].p_vaddr + phdr[i].p_memsz);
	}
}

void CopyLoadSegments(Elf64_Ehdr *ehdr)
{
	Elf64_Phdr *phdr = (Elf64_Phdr *)((UINT64)ehdr + ehdr->e_phoff);
	for (Elf64_Half i = 0; i < ehdr->e_phnum; ++i)
	{
		if (phdr[i].p_type != PT_LOAD)
		{
			continue;
		}

		UINT64 segm_in_file = (UINT64)ehdr + phdr[i].p_offset;
		CopyMem((VOID *)phdr[i].p_vaddr, (VOID *)segm_in_file, phdr[i].p_filesz);

		UINTN remain_bytes = phdr[i].p_memsz - phdr[i].p_filesz;
		SetMem((VOID *)(phdr[i].p_vaddr + phdr[i].p_filesz), remain_bytes, 0);
	}
}

EFI_STATUS EFIAPI UefiMain(
	EFI_HANDLE image_handle,
	EFI_SYSTEM_TABLE *system_table)
{
	Print(L"Hello, Mikan World!\n");

	EFI_STATUS status;

	// ファイルプロトコルの準備
	Print(L"Preparing file protocol\n");
	EFI_FILE_PROTOCOL *root_dir;
	status = OpenRootDir(image_handle, &root_dir);
	if (EFI_ERROR(status))
	{
		Print(L"failed to open root directory: %r\n", status);
		Halt();
	}

	// GOPの準備
	Print(L"Preparing GOP\n");
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	status = OpenGOP(image_handle, &gop);
	if (EFI_ERROR(status))
	{
		Print(L"failed to open GOP: %r\n", status);
		Halt();
	}

	// EFIファイルの読み込み
	Print(L"Reading the EFI file\n");

	Print(L"  Reading kernel.elf\n");
	EFI_FILE_PROTOCOL *kernel_file;
	status = root_dir->Open(root_dir, &kernel_file, L"\\kernel.elf", EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status))
	{
		Print(L"failed to open file '\\kernel.elf': %r\n", status);
		Halt();
	}

	UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
	UINT8 file_info_buffer[file_info_size];
	status = kernel_file->GetInfo(
		kernel_file, &gEfiFileInfoGuid,
		&file_info_size, file_info_buffer);
	if (EFI_ERROR(status))
	{
		Print(L"failed to get file information: %r\n", status);
		Halt();
	}

	EFI_FILE_INFO *file_info = (EFI_FILE_INFO *)file_info_buffer;
	UINTN kernel_file_size = file_info->FileSize;

	VOID *kernel_buffer;
	Print(L"  Allocating the pool\n");
	status = gBS->AllocatePool(EfiLoaderData, kernel_file_size, &kernel_buffer);
	if (EFI_ERROR(status))
	{
		Print(L"Failed to allocate pool: %r\n", status);
		Halt();
	}

	Print(L"  Reading the kernel file onto a buffer area\n");
	status = kernel_file->Read(kernel_file, &kernel_file_size, kernel_buffer);
	if (EFI_ERROR(status))
	{
		Print(L"error: %r", status);
		Halt();
	}

	Print(L"  Reading header info from the kernel file\n");
	Elf64_Ehdr *kernel_ehdr = (Elf64_Ehdr *)kernel_buffer;
	UINT64 kernel_first_addr, kernel_last_addr;
	CalcLoadAddressRange(kernel_ehdr, &kernel_first_addr, &kernel_last_addr);

	Print(L"  Allocating pages according to the header info\n");
	UINTN num_pages = (kernel_last_addr - kernel_first_addr + 0xfff) / 0x1000;
	status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, num_pages, &kernel_first_addr);

	if (EFI_ERROR(status))
	{
		Print(L"Faield to allocate pages: %r\n", status);
		Halt();
	}

	Print(L"  Copying kernel onto the allocated pages\n");
	CopyLoadSegments(kernel_ehdr);
	Print(L"Kernel: 0x%0lx - 0x%0lx\n", kernel_first_addr, kernel_last_addr);

	status = gBS->FreePool(kernel_buffer);
	if (EFI_ERROR(status))
	{
		Print(L"Failed to free kernel_buffer: %r\n", status);
		Halt();
	}

	UINT64 entry_addr = *(UINT64 *)(kernel_first_addr + 24);

	// GOPの諸情報の受け渡し
	struct FrameBufferConfig config = {
		(UINT8 *)gop->Mode->FrameBufferBase,
		gop->Mode->Info->PixelsPerScanLine,
		gop->Mode->Info->HorizontalResolution,
		gop->Mode->Info->VerticalResolution,
		0};
	switch (gop->Mode->Info->PixelFormat)
	{
	case PixelRedGreenBlueReserved8BitPerColor:
		config.pixel_format = kPixelRGBResv8BitPerColor;
		break;
	case PixelBlueGreenRedReserved8BitPerColor:
		config.pixel_format = kPixelBGRResv8BitPerColor;
		break;
	default:
		Print(L"Unimplemented pixel format: %d\n", gop->Mode->Info->PixelFormat);
		Halt();
	}

	// カーネル起動
	Print(L"Executing the kernel...");
	typedef void EntryPointType(const struct FrameBufferConfig *);
	EntryPointType *entry_point = (EntryPointType *)entry_addr;
	entry_point(&config);

	Print(L"All done\n");

	while (1)
		;
	return EFI_SUCCESS;
}