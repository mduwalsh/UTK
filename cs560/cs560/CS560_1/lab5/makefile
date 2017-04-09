# CS 360 Lab 5 Makefile
# Drew Nash -- anash4@eecs.utk.edu
# 6 March 2013

CC = gcc 

INCLUDES = -I/home/plank/cs360/include

CFLAGS = -g $(INCLUDES)

LIBDIR = /home/plank/cs360/objs

LIBS = $(LIBDIR)/libfdr.a 

EXECUTABLES: jtar

all: $(EXECUTABLES)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c

jtar: jtar.c
	$(CC) $(CFLAGS)  -o jtar jtar.c $(LIBS)

clean:
	rm fakemake

