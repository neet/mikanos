BASEDIR := $(HOME)/osbook/devenv/x86_64-elf
EDK2DIR := $(HOME)/edk2"

CLANG := /home/neet/Desktop/clang+llvm-7.1.0-aarch64-linux-gnu/bin/clang++
LLD := /home/neet/Desktop/clang+llvm-7.1.0-aarch64-linux-gnu/bin/ld.lld

CXX := $(CLANG)
LD := $(LLD)

CXXFLAGS := -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -std=c++17 \
	-I$(BASEDIR)/include/c++/v1 -I$(BASEDIR)/include -I$(BASEDIR)/include/freetype2 \
    -I$(EDK2DIR)/MdePkg/Include -I$(EDK2DIR)/MdePkg/Include/X64 \
	-nostdlibinc -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS \
    -DEFIAPI='__attribute__((ms_abi))'

LDFLAGS := --entry KernelMain -z norelro --image-base 0x100000 --static \
	-L$(BASEDIR)/lib

main: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

kernel: main.o
	$(LD) $(LDFLAGS) -o kernel.elf main.o

clean:
	rm -rf main.o kernel.elf