TARGET := kernel.elf
OBJS := main.o font.o graphics.o hankaku.o

BASEDIR := $(HOME)/osbook/devenv/x86_64-elf
EDK2DIR := $(HOME)/edk2"

CXXFLAGS := -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -std=c++17 \
			-I$(BASEDIR)/include/c++/v1 -I$(BASEDIR)/include -I$(BASEDIR)/include/freetype2 \
 			-I$(EDK2DIR)/MdePkg/Include -I$(EDK2DIR)/MdePkg/Include/X64 \
			-nostdlibinc -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS \
    		-DEFIAPI='__attribute__((ms_abi))'

LDFLAGS := --entry KernelMain -z norelro --image-base 0x100000 --static \
			-L$(BASEDIR)/lib

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET)

kernel.elf: $(OBJS) Makefile
	ld.lld $(LDFLAGS) -o kernel.elf $(OBJS)

hankaku.bin: hankaku.txt
	../tools/makefont.py -o hankaku.bin hankaku.txt

hankaku.o: hankaku.bin
	x86_64-linux-gnu-objcopy -I binary -O elf64-x86-64 -B i386:x86-64 hankaku.bin hankaku.o

%.o: %.cpp Makefile
	clang++ $(CXXFLAGS) -c $<
