CC=gcc
CFLAGS=-g -Wall -I/usr/include/libxml2
LDFLAGS=-lz -lm -lxml2

all: addamd logDB

addamd: topology.o

clean:
	rm -f core *.o *.db addamd logDB
