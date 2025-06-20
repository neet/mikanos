#include "font.hpp"

extern const uint8_t _binary_hankaku_bin_start;
extern const uint8_t _binary_hankaku_bin_end;
extern const uint8_t _binary_hankaku_bin_size;

const uint8_t *GetFont(char c)
{
	auto index = 16 * static_cast<unsigned int>(c);
	if (index >= reinterpret_cast<uintptr_t>(&_binary_hankaku_bin_size))
	{
		return nullptr;
	}

	return &_binary_hankaku_bin_start + index;
}

void WriteAscii(PixelWriter &writer, Vector2D<int> pos, char c, const PixelColor &color)
{
	const uint8_t *font = GetFont(c);
	if (font == nullptr)
	{
		return;
	}
	for (int dy = 0; dy < 16; ++dy)
	{
		for (int dx = 0; dx < 8; ++dx)
		{
			if ((font[dy] << dx) & 0b10000000)
			{

				writer.Write(Vector2D<int>{pos.x + dx, pos.y + dy}, color);
			}
		}
	}
}

void WriteString(PixelWriter &writer, Vector2D<int> pos, const char *text, const PixelColor &color)
{
	for (int k = 0; text[k] != '\0'; ++k)
	{
		const char c = text[k];
		WriteAscii(writer, pos + Vector2D<int>{k * 8, 0}, c, color);
	}
}