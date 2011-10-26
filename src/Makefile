CC=gcc
CFLAGS=-g -Wall -O0 -I/usr/include/libxml2 -I/usr/local/lib/
LDFLAGS=-lz -lm -lxml2 -lzmq

all: addamd

addamd: topology.o ham.o logDB.o

clean:
	rm -f *.o core *.db
	rm -f addamd
