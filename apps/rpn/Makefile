TARGET = rpn

CPPFLAGS += -I.
CFLAGS   += -O2 -Wall -g --target=x86_64-elf -ffreestanding
CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mcmodel=large \
            -fno-exceptions -fno-rtti -std=c++17
LDFLAGS += --entry main -z norelro --image-base 0xffff800000000000 --static

.PHONY: all
all: $(TARGET)

rpn: rpn.o Makefile
	ld.lld $(LDFLAGS) -o rpn rpn.o -lc -lc++ -lc++abi

%.o: %.cpp Makefile
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@