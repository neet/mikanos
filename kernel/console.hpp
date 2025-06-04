#pragma once

#include "graphics.hpp"

class Console
{
public:
	void SetWriter(PixelWriter *writer);
	static const int kRows = 25, kColumns = 80;
	Console(const PixelColor &fg_color, const PixelColor &bg_color);
	void PutString(const char *s);

private:
	void Newline();
	void Refresh();

	const PixelColor fg_color_, bg_color_;
	PixelWriter *writer_;
	char buffer_[kRows][kColumns + 1];
	int cursor_row_, cursor_column_;
};