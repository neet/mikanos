#include <sys/types.h>

#include "memory_map.hpp"
#include "logger.hpp"
#include "memory_manager.hpp"
#include "error.hpp"

BitmapMemoryManager::BitmapMemoryManager()
	: alloc_map_{}, range_begin_{FrameID{0}}, range_end_{FrameID{kFrameCount}}
{
}

WithError<FrameID> BitmapMemoryManager::Allocate(size_t num_frames)
{
	size_t start_frame_id = range_begin_.ID();
	while (true)
	{
		size_t i = 0;
		for (; i < num_frames; ++i)
		{
			if (start_frame_id + i >= range_end_.ID())
			{
				return {kNullFrame, MAKE_ERROR(Error::kNoEnoughMemory)};
			}
			if (GetBit(FrameID{start_frame_id + i}))
			{
				break;
			}
		}
		if (i == num_frames)
		{
			MarkAllocated(FrameID{start_frame_id}, num_frames);
			return {
				FrameID{start_frame_id},
				MAKE_ERROR(Error::kSuccess),
			};
		}
		start_frame_id += i + 1;
	}
}

Error BitmapMemoryManager::Free(FrameID start_frame, size_t num_frames)
{
	for (size_t i = 0; i < num_frames; ++i)
	{
		SetBit(FrameID{start_frame.ID() + 1}, false);
	}
	return MAKE_ERROR(Error::kSuccess);
}

void BitmapMemoryManager::MarkAllocated(FrameID start_frame, size_t num_frames)
{
	for (size_t i = 0; i < num_frames; ++i)
	{
		SetBit(FrameID{start_frame.ID() + i}, true);
	}
}

void BitmapMemoryManager::SetMemoryRange(FrameID range_begin, FrameID range_end)
{
	range_begin_ = range_begin;
	range_end_ = range_end;
}

bool BitmapMemoryManager::GetBit(FrameID frame) const
{
	auto line_index = frame.ID() / kBitsPerMapLine;
	auto bit_index = frame.ID() % kBitsPerMapLine;

	return (alloc_map_[line_index] & (static_cast<MapLineType>(1) << bit_index)) != 0;
}

void BitmapMemoryManager::SetBit(FrameID frame, bool allocated)
{
	auto line_index = frame.ID() / kBitsPerMapLine;
	auto bit_index = frame.ID() % kBitsPerMapLine;

	if (allocated)
	{
		alloc_map_[line_index] |= (static_cast<MapLineType>(1) << bit_index);
	}
	else
	{
		alloc_map_[line_index] &= ~(static_cast<MapLineType>(1) << bit_index);
	}
}

extern "C" caddr_t program_break, program_break_end;

namespace
{
	char memory_manager_buf[sizeof(BitmapMemoryManager)];
	BitmapMemoryManager *memory_manager;

	Error InitializeHeap(BitmapMemoryManager &memory_manager)
	{
		const int kHeapFrames = 64 * 512;
		const auto heap_start = memory_manager.Allocate(kHeapFrames);
		if (heap_start.error)
		{
			return heap_start.error;
		}

		program_break = reinterpret_cast<caddr_t>(heap_start.value.ID() * kBytesPerFrame);
		program_break_end = program_break + kHeapFrames * kBytesPerFrame;
		return MAKE_ERROR(Error::kSuccess);
	}
}

void InitializeMemoryManager(const MemoryMap &memory_map)
{
	::memory_manager = new (memory_manager_buf) BitmapMemoryManager;

	const auto memory_map_base = reinterpret_cast<uintptr_t>(memory_map.buffer);
	// 最後の未使用領域の末尾のアドレス
	uintptr_t available_end = 0;

	for (uintptr_t iter = memory_map_base; iter < memory_map_base + memory_map.map_size; iter += memory_map.descriptor_size)
	{
		auto desc = reinterpret_cast<const MemoryDescriptor *>(iter);
		if (available_end < desc->physical_start)
		{
			// 「メモリマップ上で歯抜けになっている領域」は利用中
			memory_manager->MarkAllocated(
				// 領域の先頭のページフレーム番号。1フレーム4KiB=4096なので、0〜4095がフレーム0
				FrameID{available_end / kBytesPerFrame},
				// 領域の大きさ（ページフレーム単位）
				(desc->physical_start - available_end) / kBytesPerFrame);
		}

		const auto physical_end = desc->physical_start + desc->number_of_pages * kUEFIPageSize;
		if (IsAvailable(static_cast<MemoryType>(desc->type)))
		{
			available_end = physical_end;
		}
		else
		{
			// 「IsAvailableが偽を返す領域」は利用中
			memory_manager->MarkAllocated(
				FrameID{desc->physical_start / kBytesPerFrame},
				(desc->number_of_pages * kUEFIPageSize / kBytesPerFrame));
		}
	}

	// これをやらないと、メモリマネージャは際限なく大きな物理メモリが積まれてると勘違いする
	memory_manager->SetMemoryRange(FrameID{1}, FrameID{available_end / kBytesPerFrame});

	// ヒープ領域を初期化（newを使えるようにする）
	if (auto err = InitializeHeap(*memory_manager))
	{
		Log(kError, "failed to allocate pages: %s at %s:%d\n", err.Name(), err.File(), err.Line());
		// exit(1);
	}
}