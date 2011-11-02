#include "vote_coord.h"

VoteCoord* Vote_Startup(int systemSize) {
    int i;
    VoteCoord* self = malloc(sizeof(VoteCoord));
    check_mem(self);
    self->systemSize = systemSize;
    self->nextVoteID = 0;
    self->numActiveVotes = 0;
    self->activeVotes = malloc(systemSize*sizeof(Vote*));
    for(i=0; i<systemSize; i++) {
        self->activeVotes[i] = malloc(sizeof(Vote));
        check_mem(self->activeVotes[i]);
        self->activeVotes[i]->voteID = 0;
        self->activeVotes[i]->votesYay = 0;
        self->activeVotes[i]->votesNay = 0;
        self->activeVotes[i]->commitQuorum = 0;
        self->activeVotes[i]->abortQuorum = 0;
    }

    return(self);
error:
    for(i=0; i<self->systemSize; i++)
        if(self->activeVotes[i]) free(self->activeVotes[i]);
    if(self->activeVotes) free(self->activeVotes);
    if(self) free(self);
    return(NULL);
}

int Vote_Shutdown(VoteCoord* self) {
    int i;
    if(self->numActiveVotes) {
        sentinel("Votes still being resolved!");
    }
    for(i=0; i<self->systemSize; i++)
        free(self->activeVotes[i]);
    free(self->activeVotes);
    free(self);
    return(0);

error:
    for(i=0; i<self->systemSize; i++)
        if(self->activeVotes[i]) free(self->activeVotes[i]);
    if(self->activeVotes) free(self->activeVotes);
    if(self) free(self);
    return(-1);
}

void Vote_Coord_Init(VoteCoord* self, unsigned int deadID, int commitQuorum, int abortQuorum) {
    self->nextVoteID++;
    self->activeVotes[deadID]->voteID = self->nextVoteID;
    self->activeVotes[deadID]->votesYay = 0;
    self->activeVotes[deadID]->votesNay = 0;
    self->activeVotes[deadID]->commitQuorum = commitQuorum;
    self->activeVotes[deadID]->abortQuorum = abortQuorum;
    self->numActiveVotes++;

    // Send out O_VOTEREQ with deadID
}

int Vote_Coord_Yay(VoteCoord* self, int deadID) {
    self->activeVotes[deadID]->votesYay++;
    // Check if commit quorum reached
    if(self->activeVotes[deadID]->votesYay==self->activeVotes[deadID]->commitQuorum)
        Vote_Coord_Restart_Node(self, deadID);
    return(0);
}

int Vote_Coord_Nay(VoteCoord* self, int deadID) {
    self->activeVotes[deadID]->votesNay++;
    // Check if abort quorum reached
    if(self->activeVotes[deadID]->votesNay==self->activeVotes[deadID]->abortQuorum)
        Vote_Coord_Abort(self, deadID);
    return(0);
}

int Vote_Coord_Restart_Node(VoteCoord* self, int deadID) {
    // Actuall kill the node
    //printf("Killing node %d\n", deadID);
    // Acknoweldge that the node has been restarted
    // Reset vote
    self->numActiveVotes--;
    return(deadID);
}

int Vote_Coord_Abort(VoteCoord* self, int deadID) {
    // Send out abort
    // Reset vote
    self->numActiveVotes--;
    return(deadID);
}

/*
void Vote_Voter_Request(int voteID, unsigned int deadID) {
    // Check hamState
    // Send O_VOTEYAY or O_VOTENAY depending on state
}

void Vote_Voter_Abort(int voteID, unsigned int notDeadID) {
    // Do nothing (for logging purposes and future developement)
}

void Vote_Voter_Commit(int voteID, unsigned int deadID) {
    // Set Ham state of deadID to -1
}
*/

