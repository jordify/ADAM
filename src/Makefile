CC=gcc
CFLAGS=-g -Wall -I/usr/include/libxml2
LDFLAGS=-lz -lm -lxml2

all: addamd

addamd: topology.o logDB.o

clean:
	rm -f *.o
	rm -f addamd
