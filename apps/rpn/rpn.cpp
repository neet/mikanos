#include <cstring>
#include <cstdlib>

#include "../../kernel/graphics.hpp"

auto &printk = *reinterpret_cast<int (*)(const char *, ...)>(0x000000000010b000);
auto &fill_rect = *reinterpret_cast<decltype(FillRectangle) *>(0x000000000010c320);
auto &scrn_writer = *reinterpret_cast<decltype(screen_writer) *>(0x000000000024e078);

int stack_ptr;
long stack[100];

long Pop()
{
	long value = stack[stack_ptr];
	--stack_ptr;
	return value;
}

void Push(long value)
{
	++stack_ptr;
	stack[stack_ptr] = value;
}

extern "C" int main(int argc, char **argv)
{
	stack_ptr = -1;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "+") == 0)
		{
			long b = Pop();
			long a = Pop();
			Push(a + b);
		}
		else if (strcmp(argv[i], "-") == 0)
		{
			long b = Pop();
			long a = Pop();
			Push(a - b);
		}
		else
		{
			long a = atol(argv[i]);
			Push(a);
		}
	}

	fill_rect(*scrn_writer, Vector2D<int>{100, 10}, Vector2D<int>{200, 200}, ToColor(0x00ff00));

	if (stack_ptr < 0)
	{
		return 0;
	}
	return static_cast<int>(Pop());
}