#include <cstdint>
#include <cstddef>

#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"

void *operator new(size_t size, void *buf)
{
  return buf;
}

void operator delete(void *obj) noexcept
{
}

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter *pixel_writer;

extern "C" void KernelMain(const FrameBufferConfig &frame_buffer_config)
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

  for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x)
  {
    for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y)
    {
      pixel_writer->Write(x, y, {255, 255, 255});
    }
  }

  WriteAscii(*pixel_writer, 0, 0, 'H', {0, 0, 0});
  WriteAscii(*pixel_writer, 8, 0, 'e', {0, 0, 0});
  WriteAscii(*pixel_writer, 16, 0, 'l', {0, 0, 0});
  WriteAscii(*pixel_writer, 24, 0, 'l', {0, 0, 0});
  WriteAscii(*pixel_writer, 32, 0, 'o', {0, 0, 0});
  WriteAscii(*pixel_writer, 40, 0, ' ', {0, 0, 0});
  WriteAscii(*pixel_writer, 48, 0, 'W', {0, 0, 0});
  WriteAscii(*pixel_writer, 56, 0, 'o', {0, 0, 0});
  WriteAscii(*pixel_writer, 64, 0, 'r', {0, 0, 0});
  WriteAscii(*pixel_writer, 72, 0, 'l', {0, 0, 0});
  WriteAscii(*pixel_writer, 80, 0, 'd', {0, 0, 0});
  WriteAscii(*pixel_writer, 88, 0, '!', {0, 0, 0});

  while (1)
    __asm__("hlt");
}
