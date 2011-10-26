CC=gcc
CFLAGS=-g -Wall -I/usr/include/libxml2
LDFLAGS=-lz -lm -lxml2 -lzmq -lczmq

all: addamd logDB

addamd: topology.o ham.o

clean:
	rm -f core *.o *.db addamd logDB
