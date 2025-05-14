#include "mouse.hpp"
#include "logger.hpp"

const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;

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

			switch (c)
			{
			case '@':
				writer->Write(position.x + dx, position.y + dy, kMouseCursorBorderColor);
				break;
			case '.':
				writer->Write(position.x + dx, position.y + dy, kMouseCursorFillColor);
				break;
			default:
				break;
			}
		}
	}
}

void EraseMouseCursor(PixelWriter *writer, Vector2D<int> position, PixelColor erase_color)
{
	for (int dy = 0; dy < kMouseCursorHeight; ++dy)
	{
		for (int dx = 0; dx < kMouseCursorWidth; ++dx)
		{
			writer->Write(position.x + dx, position.y + dy, erase_color);
		}
	}
}

MouseCursor::MouseCursor(PixelWriter *writer, PixelColor erase_color, Vector2D<int> initial_position) : pixel_writer_{writer}, erase_color_{erase_color}, position_{initial_position}
{
	Log(kDebug, "Mouse cursor is initialising\n");
	DrawMouseCursor(pixel_writer_, position_);
};

void MouseCursor::MoveRelative(Vector2D<int> displacement)
{
	EraseMouseCursor(pixel_writer_, position_, erase_color_);
	position_ += displacement;
	DrawMouseCursor(pixel_writer_, position_);
}