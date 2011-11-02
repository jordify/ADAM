/***
 * File: vote_coord.h
 * Author: Jorge Gomez
 * License: meh
 ***/
#ifndef __vote_coord_h__
#define __vote_coord_h__

#include <stdlib.h>
#include "dbg.h"

typedef struct Vote {
    int voteID;
    int votesYay;
    int votesNay;
    int commitQuorum;
    int abortQuorum;
    /* Need ham to tell us how many nodes active and what these should
     * be set to
     */
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
} VoteCoord;

/* Starting and destroying the Voter */
VoteCoord* Vote_Startup(int systemSize);

int Vote_Shutdown(VoteCoord* self);

/* Signals that can be received by Coordinator are all messages with a
 * certain opcode. The coordinator takes action by setting variables in
 * the vote or sending out messages and taking action on a foreign node. 
 */
void Vote_Coord_Init(VoteCoord* self, unsigned int deadID, int commitQuorum, int abortQuorum);

int Vote_Coord_Yay(VoteCoord* self, int voteID);

int Vote_Coord_Nay(VoteCoord* self, int voteID);

int Vote_Coord_Restart_Node(VoteCoord* self, int voteID);

int Vote_Coord_Abort(VoteCoord* self, int deadID);

/* Signals received by Voters are messages from coordinator. The
 * messages force the voters to investigate their current state and
 * respond with another message.
 * All of this can be done in the HAM when it process the message... Ham
 * has better scope at that time than this will.
 */
/*
void Vote_Voter_Request(int voteID, unsigned int deadID);

void Vote_Voter_Abort(int voteID, unsigned int notDeadID);

void Vote_Voter_Commit(int voteID, unsigned int deadID);
*/

#endif //__vote_coord_h__

