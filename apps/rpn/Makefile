TARGET = rpn
OBJS = rpn.o syscall.o newlib_support.o
include ../Makefile.elfapp

%.o: %.c Makefile
	clang $(CPPFLAGS) $(CFLAGS) -c $< -o $@