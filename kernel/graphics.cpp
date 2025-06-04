#include "graphics.hpp"

void RGBResv8BitPerColorPixelWriter::Write(int x, int y, const PixelColor &c)
{
	auto p = PixelAt(x, y);
	p[0] = c.r;
	p[1] = c.g;
	p[2] = c.b;
};

void BGRResv8BitPerColorPixelWriter::Write(int x, int y, const PixelColor &c)
{
	auto p = PixelAt(x, y);
	p[0] = c.b;
	p[1] = c.g;
	p[2] = c.r;
};

void FillRectangle(PixelWriter &writer, const Vector2D<int> &pos, const Vector2D<int> &size, const PixelColor &c)
{
	for (int dy = 0; dy < size.y; ++dy)
	{
		for (int dx = 0; dx < size.x; ++dx)
		{
			writer.Write(pos.x + dx, pos.y + dy, c);
		}
	}
};

void DrawRectangle(PixelWriter &writer, const Vector2D<int> &pos, const Vector2D<int> &size, const PixelColor &c)
{
	for (int dx = 0; dx < size.x; ++dx)
	{
		writer.Write(pos.x + dx, pos.y, c);
		writer.Write(pos.x + dx, pos.y + size.y - 1, c);
	}

	for (int dy = 0; dy < size.y - 1; ++dy)
	{
		writer.Write(pos.x, pos.y + dy, c);
		writer.Write(pos.x + size.x - 1, pos.y + dy, c);
	}
}

void DrawDesktop(PixelWriter &writer)
{
	const auto width = writer.Width();
	const auto height = writer.Height();

	// 背景
	FillRectangle(writer, {0, 0}, {width, height - 50}, kDesktopBGColor);
	// ツールバー
	FillRectangle(writer, {0, height - 50}, {width, 50}, kDesktopAccentColor);
	// 「スタート」ボタン
	FillRectangle(writer, {0, height - 50}, {width / 5, 50}, kDesktopAccent2Color);
	// Windowsアイコン
	DrawRectangle(writer, {10, height - 40}, {30, 30}, kDesktopAccentColor);
}