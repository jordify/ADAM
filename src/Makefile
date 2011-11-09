CC=gcc
CFLAGS=-Wall -Wextra -O2 -DNDEBUG -I/usr/include/libxml2 -I/usr/local/lib/
LDFLAGS=-lz -lm -lxml2 -lzmq

all: addamd

dev: CFLAGS=-g -O0 -Wall -Wextra -I/usr/include/libxml2 -I/usr/local/lib/
dev: all

addamd: topology.o ham.o logDB.o vote_coord.o message.o killNodes.o

tests: topology.o ham.o vote_coord.o message.o killNodes.o

local:
	cp .killNodes.local killNodes.c

iota:
	cp .killNodes.iota killNodes.c

clean:
	rm -f *.o core node*.db
	rm -f addamd tests

pristene: 
	rm -f killNodes.c
pristene: clean
