/***
 * File: ham.h
 * Author: Jorge Gomez
 * License: meh
 ***/
#ifndef __ham_h__
#define __ham_h__

#include <stdlib.h>
#include <time.h>
#include <zmq.h>
#include "topology.h"
#include "dbg.h"

typedef struct Link {
    unsigned int linkID;
} Link;

typedef struct Ham {
    int myID;
    void* ctx;
    void* listener;
    void* notifier;
    Topology* topo;
} Ham;

Ham* Ham_init(Topology* topo, int myID);

char* Ham_recv(Ham* ham);

int Ham_beat(Ham* ham);

void Ham_poll(Ham* ham, int timeout);

void Ham_destroy(Ham* ham);

void mSleep(int msecs);

int s_send(void *socket, char *string);

int cmpTime(const struct timespec* tp1, const struct timespec* tp2);
// Returns 1, 0, or -1 if tp1 is >, =, or < tp2 respectively

#endif

