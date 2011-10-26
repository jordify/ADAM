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

typedef struct Link {
    unsigned int linkID;
} Link;

typedef struct Ham {
    void* ctx;
    void* listener;
    void* notifier;
    Network** links;
} Ham;

Ham* Ham_init(Network** links);

void Ham_destroy(Ham* ham);

#endif
