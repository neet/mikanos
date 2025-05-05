#include <memory.h>

#include "console.hpp"
#include "font.hpp"

const int FONT_WIDTH = 8;
const int FONT_HEIGHT = 16;

Console::Console(PixelWriter &writer, const PixelColor &fg_color, const PixelColor &bg_color) : writer_{writer}, fg_color_{fg_color}, bg_color_{bg_color}
{
	cursor_row_ = 0;
	cursor_column_ = 0;

	for (int x = 0; x < kColumns * FONT_WIDTH; ++x)
	{
		for (int y = 0; y < kRows * FONT_HEIGHT; ++y)
		{
			writer_.Write(x, y, bg_color_);
		}
	}
};

void Console::PutString(const char *s)
{
	for (int k = 0; s[k] != '\0'; ++k)
	{
		const char c = s[k];
		if (c == '\n')
		{
			Newline();
		}
		else
		{
			WriteAscii(writer_, cursor_column_ * FONT_WIDTH, cursor_row_ * FONT_HEIGHT, c, fg_color_);
			buffer_[cursor_row_][cursor_column_] = c;
			++cursor_column_;
		}
	}
};

void Console::Newline()
{
	cursor_column_ = 0;

	if (cursor_row_ < kRows - 1)
	{
		++cursor_row_;
	}
	else
	{
		for (int x = 0; x < kColumns * FONT_WIDTH; ++x)
		{
			for (int y = 0; y < kRows * FONT_HEIGHT; ++y)
			{
				writer_.Write(x, y, bg_color_);
			}
		}

		for (int row = 0; row < kRows - 1; ++row)
		{
			memcpy(buffer_[row], buffer_[row + 1], kColumns + 1);
			WriteString(writer_, 0, FONT_HEIGHT * row, buffer_[row], fg_color_);
		}

		memset(buffer_[kRows - 1], 0, kColumns + 1);
	}
};
