#include <cstdint>
#include <cstddef>
#include <cstdio>

#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "console.hpp"
#include "pci.hpp"
#include "logger.hpp"
#include "usb/memory.hpp"
#include "usb/device.hpp"
#include "usb/classdriver/mouse.hpp"
#include "usb/xhci/xhci.hpp"
#include "usb/xhci/trb.hpp"

const PixelColor kDesktopBGColor = {6, 32, 43};
const PixelColor kDesktopFGColor = {245, 238, 221};
const PixelColor kDesktopAccentColor = {122, 226, 207};
const PixelColor kDesktopAccent2Color = {7, 122, 125};

void operator delete(void *obj) noexcept
{
}

extern "C" void __cxa_pure_virtual()
{
  while (1)
    __asm__("hlt");
}

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter *pixel_writer;

char console_buf[sizeof(Console)];
Console *console;

int printk(const char *format, ...)
{
  va_list ap;
  int result;
  char s[1024];

  va_start(ap, format);
  result = vsprintf(s, format, ap);
  va_end(ap);

  console->PutString(s);
  return result;
}

const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;

const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth + 1] = {
    "@              ",
    "@@             ",
    "@.@            ",
    "@..@           ",
    "@...@          ",
    "@....@         ",
    "@.....@        ",
    "@......@       ",
    "@.......@      ",
    "@........@     ",
    "@.........@    ",
    "@..........@   ",
    "@...........@  ",
    "@............@ ",
    "@......@@@@@@@@",
    "@......@       ",
    "@....@@.@      ",
    "@...@ @.@      ",
    "@..@   @.@     ",
    "@.@    @.@     ",
    "@@      @.@    ",
    "@       @.@    ",
    "         @.@   ",
    "         @@@   ",
};

void RenderMouseCursorAt(int x, int y)
{
  for (int dy = 0; dy < kMouseCursorHeight; ++dy)
  {
    for (int dx = 0; dx < kMouseCursorWidth; ++dx)
    {
      const char c = mouse_cursor_shape[dy][dx];

      switch (c)
      {
      case '@':
        pixel_writer->Write(x + dx, y + dy, kDesktopFGColor);
        break;
      case '.':
        pixel_writer->Write(x + dx, y + dy, kDesktopBGColor);
        break;
      default:
        break;
      }
    }
  }
}

void SwitchEhci2Xhci(const pci::Device &xhc_dev)
{
  bool intel_ehc_exist = false;
  for (int i = 0; i < pci::num_device; ++i)
  {
    if (pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x20u) && 0x8086 == pci::ReadVendorId(pci::devices[i]))
    {
      intel_ehc_exist = true;
      break;
    }
  }
  if (!intel_ehc_exist)
  {
    return;
  }

  uint32_t superspeed_ports = pci::ReadConfReg(xhc_dev, 0xdc);
  pci::WriteConfReg(xhc_dev, 0xd8, superspeed_ports);
  uint32_t ehci2xhci_ports = pci::ReadConfReg(xhc_dev, 0xd4);
  pci::WriteConfReg(xhc_dev, 0xd0, ehci2xhci_ports);
  printk("SwitchEhci2Xhci: SS = %02x, xHCI = %02x\n", superspeed_ports, ehci2xhci_ports);
}

extern "C" void
KernelMain(const FrameBufferConfig &frame_buffer_config)
{
  switch (frame_buffer_config.pixel_format)
  {
  case kPixelRGBResv8BitPerColor:
    pixel_writer = new (pixel_writer_buf)
        RGBResv8BitPerColorPixelWriter{frame_buffer_config};
    break;
  case kPixelBGRResv8BitPerColor:
    pixel_writer = new (pixel_writer_buf)
        BGRResv8BitPerColorPixelWriter{frame_buffer_config};
    break;
  }

  console = new (console_buf)
      Console{*pixel_writer, kDesktopFGColor, kDesktopBGColor};

  const int kFrameWidth = frame_buffer_config.horizontal_resolution;
  const int kFrameHeight = frame_buffer_config.vertical_resolution;

  // 背景
  FillRectangle(*pixel_writer, {0, 0}, {kFrameWidth, kFrameHeight - 50}, kDesktopBGColor);
  // ツールバー
  FillRectangle(*pixel_writer, {0, kFrameHeight - 50}, {kFrameWidth, 50}, kDesktopAccentColor);
  // 「スタート」ボタン
  FillRectangle(*pixel_writer, {0, kFrameHeight - 50}, {kFrameWidth / 5, 50}, kDesktopAccent2Color);
  // Windowsアイコン
  DrawRectangle(*pixel_writer, {10, kFrameHeight - 40}, {30, 30}, kDesktopAccentColor);

  auto err = pci::ScanAllBus();
  printk("ScanAllBus: %s\n", err.Name());

  for (int i = 0; i < pci::num_device; ++i)
  {
    const auto &dev = pci::devices[i];
    auto vendor_id = pci::ReadVendorId(dev.bus, dev.device, dev.function);
    auto class_code = pci::ReadClassCode(dev.bus, dev.device, dev.function);

    printk("%d.%d.%d: vend %04x, class %08x, head: %02x\n", dev.bus, dev.device, dev.function, vendor_id, class_code, dev.header_type);
  }

  pci::Device *xhc_dev = nullptr;
  for (int i = 0; i < pci::num_device; ++i)
  {
    if (pci::devices[i].class_code.Match(0x0cu, 0x03u, 0x30u))
    {
      xhc_dev = &pci::devices[i];

      if (0x8086 == pci::ReadVendorId(*xhc_dev))
      {
        break;
      }
    }
  }

  if (xhc_dev)
  {
    // Log(kInfo, "xHC has been found: %d.%d.%d\n", xhc_dev->bus, xhc_dev->device, xhc_dev->function);
    printk("xHC has been found => %d.%d.%d\n", xhc_dev->bus, xhc_dev->device, xhc_dev->function);
  }

  const WithError<uint64_t> xhc_bar = pci::ReadBar(*xhc_dev, 0);
  printk("ReadBar: %s\n", xhc_bar.error.Name());
  const uint64_t xhc_mmio_base = xhc_bar.value * ~static_cast<uint64_t>(0xf);
  printk("xHC mmio_base = %08lx\n", xhc_mmio_base);

  usb::xhci::Controller xhc{xhc_mmio_base};

  if (0x8086 == pci::ReadVendorId(*xhc_dev))
  {
    SwitchEhci2Xhci(*xhc_dev);
  }
  {
    auto err = xhc.Initialize();
    Log(kDebug, "xhc.Initialize: %s\n", err.Name());
  }

  Log(kInfo, "xHC starting\n");
  xhc.Run();

  while (1)
    __asm__("hlt");
}
