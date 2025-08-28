TARGET = blocks
OBJS =   blocks.o
include ../Makefile.elfapp

%.o: %.c Makefile
	clang $(CPPFLAGS) $(CFLAGS) -c $< -o $@