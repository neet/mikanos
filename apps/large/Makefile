TARGET = large 
OBJS = large.o
include ../Makefile.elfapp

%.o: %.c Makefile
	clang $(CPPFLAGS) $(CFLAGS) -c $< -o $@