#pragma once

#include <algorithm>

#include "frame_buffer_config.hpp"

struct PixelColor
{
	uint8_t r, g, b;
};

inline bool operator==(const PixelColor &lhs, const PixelColor &rhs)
{
	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}

inline bool operator!=(const PixelColor &lhs, const PixelColor &rhs)
{
	return !(lhs == rhs);
}

template <typename T>
struct Vector2D
{
	T x, y;

	template <typename U>
	Vector2D<T> &operator+=(const Vector2D<U> &rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
};

inline Vector2D<int> ElementMin(Vector2D<int> lhs, Vector2D<int> rhs)
{
	return Vector2D<int>{std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y)};
}

inline Vector2D<int> ElementMax(Vector2D<int> lhs, Vector2D<int> rhs)
{
	return Vector2D<int>{std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y)};
}

template <typename T, typename U>
auto operator+(const Vector2D<T> &lhs, const Vector2D<U> &rhs)
	-> Vector2D<decltype(lhs.x + rhs.x)>
{
	return {lhs.x + rhs.x, lhs.y + rhs.y};
}

template <typename T, typename U>
auto operator-(const Vector2D<T> &lhs, const Vector2D<U> &rhs)
	-> Vector2D<decltype(lhs.x - rhs.x)>
{
	return {lhs.x - rhs.x, lhs.y - rhs.y};
}

template <typename T>
struct Rectangle
{
	Vector2D<T> pos;
	Vector2D<T> size;
};

template <typename T, typename U>
Rectangle<T> operator&(const Rectangle<T> &lhs, const Rectangle<U> &rhs)
{
	const auto lhs_end = lhs.pos + lhs.size;
	const auto rhs_end = rhs.pos + rhs.size;

	if (lhs_end.x < rhs.pos.x || lhs_end.y < rhs.pos.y || rhs_end.x < lhs.pos.x || rhs_end.y < lhs.pos.y)
	{
		return {{0, 0}, {0, 0}};
	}

	auto new_pos = ElementMax(lhs.pos, rhs.pos);
	auto new_size = ElementMin(lhs_end, rhs_end) - new_pos;
	return {new_pos, new_size};
}

class PixelWriter
{
public:
	virtual ~PixelWriter() = default;
	virtual void Write(Vector2D<int> pos, const PixelColor &c) = 0;
	virtual int Width() const = 0;
	virtual int Height() const = 0;
};

class FrameBufferWriter : public PixelWriter
{
public:
	FrameBufferWriter(const FrameBufferConfig &config) : config_{config}
	{
	}
	virtual ~FrameBufferWriter() = default;
	virtual int Width() const override { return config_.horizontal_resolution; }
	virtual int Height() const override { return config_.vertical_resolution; }

protected:
	uint8_t *PixelAt(Vector2D<int> pos)
	{
		return config_.frame_buffer + 4 * (config_.pixels_per_scan_line * pos.y + pos.x);
	}

private:
	const FrameBufferConfig &config_;
};

class RGBResv8BitPerColorPixelWriter : public FrameBufferWriter
{
public:
	using FrameBufferWriter::FrameBufferWriter;
	virtual void Write(Vector2D<int> pos, const PixelColor &c) override;
};

class BGRResv8BitPerColorPixelWriter : public FrameBufferWriter
{
public:
	using FrameBufferWriter::FrameBufferWriter;
	virtual void Write(Vector2D<int> pos, const PixelColor &c) override;
};

void FillRectangle(PixelWriter &writer, const Vector2D<int> &pos, const Vector2D<int> &size, const PixelColor &c);
void DrawRectangle(PixelWriter &writer, const Vector2D<int> &pos, const Vector2D<int> &size, const PixelColor &c);
void DrawDesktop(PixelWriter &writer);

const PixelColor kDesktopBGColor = {6, 32, 43};
const PixelColor kDesktopFGColor = {245, 238, 221};
const PixelColor kDesktopAccentColor = {122, 226, 207};
const PixelColor kDesktopAccent2Color = {7, 122, 125};
