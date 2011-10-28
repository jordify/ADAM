/***
 * File: ham.h
 * Author: Jorge Gomez
 * License: meh
 ***/
#ifndef __ham_h__
#define __ham_h__

#include <stdlib.h>
#include "../utils/zhelpers.h"
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

void Ham_destroy(Ham* ham);

#endif

