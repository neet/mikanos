#include "mouse.hpp"
#include "logger.hpp"

const PixelColor kMouseCursorBorderColor = {6, 32, 43};
const PixelColor kMouseCursorFillColor = {245, 238, 221};

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

void DrawMouseCursor(PixelWriter *writer, Vector2D<int> position)
{
	for (int dy = 0; dy < kMouseCursorHeight; ++dy)
	{
		for (int dx = 0; dx < kMouseCursorWidth; ++dx)
		{
			const char c = mouse_cursor_shape[dy][dx];

			if (c == '@')
			{
				writer->Write(position + Vector2D<int>{dx, dy}, kMouseCursorBorderColor);
			}
			else if (c == '.')
			{
				writer->Write(position + Vector2D<int>{dx, dy}, kMouseCursorFillColor);
			}
			else
			{
				writer->Write(position + Vector2D<int>{dx, dy}, kMouseTransparentColor);
			}
		}
	}
}