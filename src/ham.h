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
#include "killNodes.h"
#include "topology.h"
#include "message.h"
#include "dbg.h"

#define HBTIMEOUT 5

typedef struct Vote {
    int voteID;
    int votesYay;
    int votesNay;
    int commitQuorum;
    int abortQuorum;
    // Should include here a time of last action for time outs
} Vote;

typedef struct VoteCoord {
    int systemSize;
    int nextVoteID;
    /* Needs to be set to 0 before any votes take place
     * Will act like Lamport clock, being incremented when new vote
     * takes place outside of node to that latest vote's ID
     */
    int numActiveVotes;
    /* Number of active votes the node is coordinating */
    Vote** activeVotes;
    /* Array of active votes the node is coordinating */
    int* participatingVotes;
    int numParticpatingVotes;
} VoteCoord;


typedef struct Ham {
    int myID;
    void* ctx;
    void* listener;
    void* notifier;
    Topology* topo;
    int* hbStates;
    VoteCoord* coord;
} Ham;

/* Starting and destroying the Voter */
VoteCoord* Vote_Startup(int systemSize);

int Vote_Shutdown(VoteCoord* self);

/* Signals that can be received by Coordinator are all messages with a
 * certain opcode. The coordinator takes action by setting variables in
 * the vote or sending out messages and taking action on a foreign node. 
 */
void Vote_Coord_Init(VoteCoord* self, unsigned int deadID, int commitQuorum, int abortQuorum);

int Vote_Coord_Yay(VoteCoord* self, int deadID, Ham* ham);

int Vote_Coord_Nay(VoteCoord* self, int deadID, Ham* ham);

//int Vote_Coord_Restart_Node(VoteCoord* self, int deadID);
//
//int Vote_Coord_Abort(VoteCoord* self, int deadID);

/* Ham methods */
// Open up sockets on network interfaces
Ham* Ham_init(Topology* topo, unsigned int myID);

// Request vote *SHOULD BE IN MESSAGE CLASS*
int Ham_sendVote(Ham* ham, unsigned char deadID);

void Ham_procKill(Ham* ham, unsigned char deadID);

void Ham_procKillAbort(Ham* ham, unsigned char deadID);

// Send out a heart beat to all subscribers
int Ham_beat(Ham* ham);

// Polls for incoming on listener socket
int Ham_poll(Ham* ham, int timeout);

// Process an incoming vote request
void Ham_procVoteReq(Ham* ham, unsigned char deadID, unsigned char coordID);

int Ham_sendVoteYay(Ham* ham, unsigned char deadID);

int Ham_sendVoteNay(Ham* ham, unsigned char deadID);

// Increment everyone's hbState
void Ham_timeoutHBs(Ham* ham);

// Process an incoming heartbeat message
void Ham_procHB(Ham* ham, unsigned int source);

// Tear down a system
void Ham_destroy(Ham* ham);

// Sleep for a number of milliseconds
void mSleep(int msecs);

// Convert C string to 0MQ string and send to socket
int s_send(void *socket, char *string);

// Returns 1, 0, or -1 if tp1 is >, =, or < tp2 respectively
int cmpTime(const struct timespec* tp1, const struct timespec* tp2);

#endif // __ham_h__

