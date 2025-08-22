TARGET = stars 
OBJS =   stars.o
include ../Makefile.elfapp

%.o: %.c Makefile
	clang $(CPPFLAGS) $(CFLAGS) -c $< -o $@