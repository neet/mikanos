#include <cstdint>
#include <cstddef>
#include <cstdio>

#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "console.cpp"

const PixelColor kDesktopBGColor = {6, 32, 43};
const PixelColor kDesktopFGColor = {245, 238, 221};
const PixelColor kDesktopAccentColor = {122, 226, 207};
const PixelColor kDesktopAccent2Color = {7, 122, 125};

void *operator new(size_t size, void *buf)
{
  return buf;
}

void operator delete(void *obj) noexcept
{
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

  RenderMouseCursorAt(kFrameWidth / 2, kFrameHeight / 2);

  printk("Hello world\n");

  while (1)
    __asm__("hlt");
}
