TARGET = winhello
OBJS =  winhello.o
include ../Makefile.elfapp

%.o: %.c Makefile
	clang $(CPPFLAGS) $(CFLAGS) -c $< -o $@