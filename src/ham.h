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

typedef struct Ham {
    int myID;
    void* ctx;
    void* listener;
    void* notifier;
    Topology* topo;
    int* hbStates;
} Ham;

// Open up sockets on network interfaces
Ham* Ham_init(Topology* topo, unsigned int myID);

// Send out a heart beat to all subscribers
int Ham_beat(Ham* ham);

// Polls for incoming on listener socket
void Ham_poll(Ham* ham, int timeout);

// Increment everyone's hbState
void Ham_timeoutHBs(Ham* ham);

// Process an incoming heartbeat message
void Ham_procHB(Ham* ham, char* message);

// Tear down a system
void Ham_destroy(Ham* ham);

// Sleep for a number of milliseconds
void mSleep(int msecs);

// Convert C string to 0MQ string and send to socket
int s_send(void *socket, char *string);

// Returns 1, 0, or -1 if tp1 is >, =, or < tp2 respectively
int cmpTime(const struct timespec* tp1, const struct timespec* tp2);

#endif

