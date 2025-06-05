#include "frame_buffer.hpp"
#include "error.hpp"

Error FrameBuffer::Initialize(const FrameBufferConfig &config)
{
	config_ = config;

	const auto bits_per_pixel = BitsPerPixel(config_.pixel_format);
	if (bits_per_pixel <= 0)
	{
		return MAKE_ERROR(Error::kUnknownPixelFormat);
	}

	if (config_.frame_buffer)
	{
		buffer_.resize(0);
	}
	else
	{
		// (x+7)/8 はビットからバイトに変換する処理
		buffer_.resize(((bits_per_pixel + 7) / 8) * config_.horizontal_resolution * config_.vertical_resolution);
		config_.frame_buffer = buffer_.data();
		config_.pixels_per_scan_line = config.horizontal_resolution;
	}

	switch (config_.pixel_format)
	{
	case kPixelRGBResv8BitPerColor:
		writer_ = std::make_unique<RGBResv8BitPerColorPixelWriter>(config_);
		break;
	case kPixelBGRResv8BitPerColor:
		writer_ = std::make_unique<BGRResv8BitPerColorPixelWriter>(config_);
		break;
	default:
		return MAKE_ERROR(Error::kUnknownPixelFormat);
	}

	return MAKE_ERROR(Error::kSuccess);
};

Error FrameBuffer::Copy(Vector2D<int> pos, const FrameBuffer &src)
{
	if (config_.pixel_format != src.config_.pixel_format)
	{
		return MAKE_ERROR(Error::kUnknownPixelFormat);
	}

	const auto bits_per_pixel = BitsPerPixel(config_.pixel_format);
	if (bits_per_pixel <= 0)
	{
		return MAKE_ERROR(Error::kUnknownPixelFormat);
	}

	const auto dst_width = config_.horizontal_resolution;
	const auto dst_height = config_.vertical_resolution;
	const auto src_width = src.config_.horizontal_resolution;
	const auto src_height = src.config_.vertical_resolution;

	const int copy_start_dst_x = std::max(pos.x, 0);
	const int copy_start_dst_y = std::max(pos.y, 0);
	const int copy_end_dst_x = std::min(pos.x + src_width, dst_width);
	const int copy_end_dst_y = std::min(pos.y + src_height, dst_height);

	// 休憩ここまで

	return MAKE_ERROR(Error::kSuccess);
};

int BitsPerPixel(PixelFormat format)
{
	switch (format)
	{
	case kPixelRGBResv8BitPerColor:
		return 32;
	case kPixelBGRResv8BitPerColor:
		return 32;
	}
	return -1;
}