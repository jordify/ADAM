#include "ham.h"

VoteCoord* Vote_Startup(int systemSize) {
    int i;
    VoteCoord* self = malloc(sizeof(VoteCoord));
    check_mem(self);
    self->systemSize = systemSize;
    self->nextVoteID = 0;
    self->numActiveVotes = 0;
    self->activeVotes = malloc(systemSize*sizeof(Vote*));
    self->participatingVotes = malloc(systemSize*sizeof(int));
    for(i=0; i<systemSize; i++) {
        self->participatingVotes[i] = 0;
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
}

static int Vote_Coord_Restart_Node(VoteCoord* self, int deadID, Ham* ham) {
    // Actuall kill the node
    KillNodes_kill(ham->topo->nodeCount, deadID);

    // Log the kill
    char data[10];
    snprintf(data, 9, "Kill %d", deadID);
    data[10] = '\0';
    int rc = logSomething(ham, data);
    ham->hbStates[deadID] = -1;
    
    // Acknoweldge that the node has been restarted
    Header* killHeader = Header_init(ham->myID, deadID, o_KILLACK);
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(Header));
    memcpy(zmq_msg_data(&message), killHeader, sizeof(Header));
    rc = zmq_send(ham->notifier, &message, 0);
    zmq_msg_close(&message);
    Header_destroy(killHeader);

    // Reset vote
    self->activeVotes[deadID]->voteID = 0;
    self->numActiveVotes--;
    return(rc);
}

static int Vote_Coord_Abort(VoteCoord* self, int deadID, Ham* ham) {
    // Send out abort
    Header* killHeader = Header_init(ham->myID, deadID, o_KILLNACK);
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(Header));
    memcpy(zmq_msg_data(&message), killHeader, sizeof(Header));
    int rc = zmq_send(ham->notifier, &message, 0);
    zmq_msg_close(&message);
    Header_destroy(killHeader);

    // Reset vote
    self->activeVotes[deadID]->voteID = 0;
    self->numActiveVotes--;
    return(rc);
}

int Vote_Coord_Yay(VoteCoord* self, int deadID, Ham* ham) {
    self->activeVotes[deadID]->votesYay++;
    // Check if commit quorum reached
    if(self->activeVotes[deadID]->votesYay==self->activeVotes[deadID]->commitQuorum)
        Vote_Coord_Restart_Node(self, deadID, ham);
    return(0);
}

int Vote_Coord_Nay(VoteCoord* self, int deadID, Ham* ham) {
    self->activeVotes[deadID]->votesNay++;
    // Check if abort quorum reached
    if(self->activeVotes[deadID]->votesNay==self->activeVotes[deadID]->abortQuorum)
        Vote_Coord_Abort(self, deadID, ham);
    return(0);
}

int logSomething(Ham* ham, char* data) {
    int rc = 0;
#ifndef NEXTLOG
    zmq_msg_t message;
    zmq_msg_init_size(&message, strlen(data));
    memcpy(zmq_msg_data(&message), data, strlen(data));
    rc = zmq_send(ham->logger, &message, 0);
    zmq_msg_close(&message);
#endif
    return (rc);
}

