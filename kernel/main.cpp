#include <cstdint>
#include <cstddef>
#include <cstdio>

#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "console.cpp"

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

  console = new (console_buf)
      Console{*pixel_writer, {0, 0, 0}, {255, 255, 255}};

  console->PutString(
      "sirokani pe ran ran piskan, konkani pe\n"
      "ran ran piskan.\" ari an rekpo ci=ki kane\n"
      "pet esoro sap=as ayne, aynu kotan enkasike\n"
      "ci=kus kor si-corpok un inkar=as ko\n"
      "teeta wenkur tane nispa ne, teeta nispa\n"
      "tane wenkur ne kotom siran.\n"
      "atuyteksam ta aynu hekattar ak-sinot-pon-ku\n"
      "ak-sinot-pon-ay euesinot kor okay.\n"
      "sirokani pe ran ran piskan,\n"
      "konkani pe ran ran piskan.\" ari an rekpo\n"
      "ci=ki kane hekaci utar enkasike\n"
      "ci=kus awa, un=corpoke ehoyuppa\n"
      "'ene hawokay i:'\n"
      "pirka cikappo! kamuy cikappo!\n"
      "keke hetak, ak=as wa toan cikappo\n"
      "kamuy cikappo tukan wa an kur, hoski uk kur\n"
      "sonno rametok sino cipapa ne ruwe tapan\"\n"
      "hawokay kane, teeta wenkur tane nispa ne p\n"
      "poutari, konkani pon ku konkani pon ay\n"
      "ueunu pa un=tukan ko, konkani pon ay\n"
      "si-corpok ci=kuste si-enka ci=kuste.\n"
      "ci=kus awa, un=corpoke ehoyuppa\n"
      "'ene hawokay i:'\n"
      "pirka cikappo! kamuy cikappo!\n"
      "keke hetak, ak=as wa toan cikappo\n"
      "kamuy cikappo tukan wa an kur, hoski uk kur\n"
      "sonno rametok sino cipapa ne ruwe tapan\"\n");

  // for (int i = 0; i < 16; ++i)
  // {
  //   printk("printk: %d\n", i);
  // }

  while (1)
    __asm__("hlt");
}
