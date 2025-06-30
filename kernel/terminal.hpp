#pragma once

#include <memory>

#include "window.hpp"
#include "graphics.hpp"
#include "message.hpp"

class Terminal
{
public:
	static const int kLineMax = 128;
	static const int kRows = 15, kColumns = 60;

	Terminal();
	unsigned int LayerID() const { return layer_id_; }
	Rectangle<int> BlinkCursor();
	Rectangle<int> InputKey(uint8_t modifier, uint8_t keycode, char ascii);

private:
	std::shared_ptr<ToplevelWindow> window_;
	unsigned int layer_id_;

	Vector2D<int> cursor_{0, 0};
	bool cursor_visible_{false};
	void DrawCursor(bool visible);
	Vector2D<int> CalcCursorPos() const;

	int linebuf_index_{0};
	std::array<char, kLineMax> linebuf_{};
	void Scroll1();
};

void TaskTerminal(uint64_t task_id, int64_t data);

constexpr Message MakeLayerMessage(uint64_t task_id, unsigned int layer_id, LayerOperation op, const Rectangle<int> &area)
{
	Message msg{Message::kLayer, task_id};
	msg.arg.layer.layer_id = layer_id;
	msg.arg.layer.op = op;
	msg.arg.layer.x = area.pos.x;
	msg.arg.layer.y = area.pos.y;
	msg.arg.layer.w = area.size.x;
	msg.arg.layer.h = area.size.y;
	return msg;
}