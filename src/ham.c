#include "ham.h"

Ham* Ham_init(Topology* topo, unsigned int myID) {
    VoteCoord* coord = NULL;
    void* logger = NULL;
    int* hbStates = NULL;
    unsigned int i = 0;
    // make a new Ham
    Ham* newHam = malloc(sizeof(Ham));
    check_mem(newHam);

    // ZMQ Context for all proceedings
    void* ctx = zmq_init(1);
    check(ctx, "Context Failed"); 
    // Base port for network 1
    int basePort = 5600;

    // Identity for sub socket
    char myIdentity[50];
    snprintf(myIdentity, 49, "Node %d", myID);

    // Bind address for pub socket
    char notifierBindAddress[50];
    snprintf(notifierBindAddress, 49, "tcp://*:%d", basePort + myID);

    // Bind pub socket
    void* notifier = zmq_socket(ctx, ZMQ_PUB);
    check(notifier, "Notifier creation failed");
    i = 1000;
    zmq_setsockopt(notifier, ZMQ_LINGER, &i, sizeof(i));
    zmq_bind(notifier, notifierBindAddress); // Need to check rc
    debug("Notifier bound to %s", notifierBindAddress);

    // Make sub socket 
    void* listener = zmq_socket(ctx, ZMQ_SUB);
    check(listener, "Listener creation failed");
    zmq_setsockopt(listener, ZMQ_IDENTITY, myIdentity, strlen(myIdentity));
    zmq_setsockopt(listener, ZMQ_SUBSCRIBE, "", 0);

    // Connect addresses for sub socket (needs to come from topology)
    char listenerConnAddress[100];
    Node* thisNode = NULL;
    for(i=0; i<topo->nodeCount; i++) {
        thisNode = topo->allNodes[i];
        if(thisNode->id == myID) continue;
        snprintf(listenerConnAddress, 49, "tcp://%s:%d", thisNode->name, basePort+thisNode->id);
        zmq_connect(listener, listenerConnAddress); // Need to check rc
        debug("Listener connected to %s", listenerConnAddress);
    }

#ifndef NLOGGING
    // Make logging socket
    logger = zmq_socket(ctx, ZMQ_PUSH);
    check(logger, "Logger creation failed");
    zmq_connect(logger, "tcp://iota:54321");
#endif

    // Create intial health states (-1:=Not Monitored, 0,1,2,...
    // :=Number of missed heartbeats)
    hbStates = malloc(topo->nodeCount*sizeof(int));
    for(i=0; i<topo->nodeCount; i++)
        hbStates[i] = -1;
    hbStates[myID] = 0;

    // Initialize a vote coordinator for this node's hb timeouts
    coord = Vote_Startup(topo->nodeCount);
    check_mem(coord);

    // Set new ham structure
    newHam->myID = myID;
    newHam->ctx = ctx;
    newHam->listener = listener;
    newHam->notifier = notifier;
    newHam->logger = logger;
    newHam->topo = topo;
    newHam->hbStates = hbStates;
    newHam->coord = coord;

    // return ham
    return(newHam);
error:
    if(newHam->notifier) zmq_close(newHam->notifier);
    if(newHam->listener) zmq_close(newHam->listener);
    if(newHam->logger) zmq_close(newHam->logger);
    if(newHam->ctx) zmq_term(newHam->ctx);
    if (coord) Vote_Shutdown(coord);
    if (hbStates) free(hbStates);
    if (newHam) free(newHam);
    return(0);
}

int Ham_sendVoteReq(Ham* ham, unsigned char deadID) {
    Header* voteHeader = Header_init(ham->myID, deadID, o_VOTEREQ);
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(Header));
    memcpy(zmq_msg_data(&message), voteHeader, sizeof(Header));
    int rc = zmq_send(ham->notifier, &message, 0);
    zmq_msg_close(&message);
    Header_destroy(voteHeader);

    // Log the vote req
    char data[50];
    snprintf(data, 49, "[%d] Requested Vote for %d", ham->myID, deadID);
    logSomething(ham, data);
    return(rc);
}

int Ham_beat(Ham* ham) {
    Header* beatHeader = Header_init(ham->myID, (unsigned char) -1, o_HEARTBEAT);
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(Header));
    memcpy(zmq_msg_data(&message), beatHeader, sizeof(Header));
    int rc = zmq_send(ham->notifier, &message, 0);
    zmq_msg_close(&message);
    Header_destroy(beatHeader);

    return(rc);
}

int Ham_countLive(Ham* ham) {
    unsigned int i;
    int count = 0;
    for(i=0; i<ham->topo->nodeCount; i++)
        if(ham->hbStates[i]>=0)
            count++;
    return(count);
}

int Ham_poll(Ham* ham, int timeout) {
// TODO: Add ipc REP socket for communicating with libAddam

    /* Set up poll item */
    zmq_pollitem_t items[] = {
        { ham->listener, 0, ZMQ_POLLIN, 0 }
    };

    zmq_msg_t message;
    Header* incoming = malloc(sizeof(Header));

    zmq_poll(items, 1, timeout);
    if(items[0].revents & ZMQ_POLLIN) {
        zmq_msg_init(&message);
        zmq_recv(ham->listener, &message, 0);
        int size = zmq_msg_size(&message);
        memcpy(incoming, zmq_msg_data(&message), size);
        zmq_msg_close(&message);
        // Detect message type and print it
        switch(incoming->opcode) {
            case o_HEARTBEAT:
                //debug("HB Message Received");
                break;
            case o_VOTEREQ:
                debug("Vote Request Message Received for dead node %d", incoming->destination);
                Ham_procVoteReq(ham, incoming->destination, incoming->source);
                break;
            case o_VOTEYES:
                debug("Vote Yay Message Received");
                Vote_Coord_Yay(ham->coord, incoming->destination, ham);
                break;
            case o_VOTENO:
                debug("Vote Nay Message Received");
                Vote_Coord_Nay(ham->coord, incoming->destination, ham);
                break;
            case o_KILLACK:
                debug("Kill ACK Message Received");
                Ham_procKill(ham, incoming->destination);
                break;
            case o_KILLNACK:
                debug("Kill abort Message Received");
                Ham_procKillAbort(ham, incoming->destination);
                break;
            default:
                sentinel("Unknown Message Type %d", incoming->opcode);
                break;
        }
        Ham_procHB(ham, incoming->source);
    }
    if(incoming) free(incoming);
    return(0);
error:
    if(incoming) free(incoming);
    return(1);
}

void Ham_procKill(Ham* ham, unsigned char deadID) {
    ham->coord->participatingVotes[deadID] = 0;
    ham->hbStates[deadID] = -1;
}

void Ham_procKillAbort(Ham* ham, unsigned char deadID) {
    ham->coord->participatingVotes[deadID] = 0;
}

void Ham_procVoteReq(Ham* ham, unsigned char deadID, unsigned char coordID) {
    // Am I coordinating a vote for this node?
    if(ham->coord->activeVotes[deadID]->voteID) {
        // Ignore vote request if coordID > myID
        if(coordID>ham->myID) {
            // Ignore
            debug("Ignoring vote request from node %d", coordID);
        } else {
            // Reset my vote coordination
            ham->coord->activeVotes[deadID]->voteID = 0;
            ham->coord->numActiveVotes--;
            // Participate in vote
            ham->coord->participatingVotes[deadID] = 1;
            if(ham->hbStates[deadID] >= HBTIMEOUT-1) {
                debug("I vote %d is dead", deadID);
                Ham_sendVoteYay(ham, deadID);
            } else {
                debug("I vote %d is not dead", deadID);
                Ham_sendVoteNay(ham, deadID);
            }
        }
    } else {
        // Participate in vote
        ham->coord->participatingVotes[deadID] = 1;
        if(ham->hbStates[deadID] >= HBTIMEOUT-1 || ham->hbStates[deadID] < 0) {
            debug("I vote %d is dead", deadID);
            Ham_sendVoteYay(ham, deadID);
        } else {
            debug("I vote %d is not dead", deadID);
            Ham_sendVoteNay(ham, deadID);
        }
    }
}

int Ham_sendVoteYay(Ham* ham, unsigned char deadID) {
    Header* voteHeader = Header_init(ham->myID, deadID, o_VOTEYES);
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(Header));
    memcpy(zmq_msg_data(&message), voteHeader, sizeof(Header));
    int rc = zmq_send(ham->notifier, &message, 0);
    zmq_msg_close(&message);
    Header_destroy(voteHeader);
    return(rc);
}

int Ham_sendVoteNay(Ham* ham, unsigned char deadID) {
    Header* voteHeader = Header_init(ham->myID, deadID, o_VOTENO);
    zmq_msg_t message;
    zmq_msg_init_size(&message, sizeof(Header));
    memcpy(zmq_msg_data(&message), voteHeader, sizeof(Header));
    int rc = zmq_send(ham->notifier, &message, 0);
    zmq_msg_close(&message);
    Header_destroy(voteHeader);
    return(rc);
}

void Ham_timeoutHBs(Ham* ham) {
    unsigned int i;
    for(i=0; i<ham->topo->nodeCount; i++)
        if(ham->hbStates[i]>=0) {
            ham->hbStates[i]++;
            if(ham->hbStates[i] >= HBTIMEOUT) {
                debug("Node %d is dead", i);
                if(!(ham->coord->participatingVotes[i])) {
                    int alive = Ham_countLive(ham);
                    Vote_Coord_Init(ham->coord, i, alive/2, 1);
                    Ham_sendVoteReq(ham, (unsigned char) i);
                }
            }
        }
    ham->hbStates[ham->myID] = 0;
}

void Ham_procHB(Ham* ham, unsigned int source) {
    ham->hbStates[source] = 0;
}

void Ham_destroy(Ham* ham) {
    zmq_close(ham->notifier);
    zmq_close(ham->listener);
    zmq_close(ham->logger);
    zmq_term(ham->ctx);
    if(ham->coord) Vote_Shutdown(ham->coord);
    if(ham->hbStates) free(ham->hbStates);
    if(ham) free(ham);
}

void mSleep (int msecs) {
    struct timespec t;
    t.tv_sec  =  msecs / 1000;
    t.tv_nsec = (msecs % 1000) * 1000000;
    nanosleep(&t, NULL);
}

int s_send(void *socket, char *string) {
    int rc;
    zmq_msg_t message;
    zmq_msg_init_size(&message, strlen (string));
    memcpy(zmq_msg_data (&message), string, strlen (string));
    rc = zmq_send(socket, &message, 0);
    zmq_msg_close(&message);
    return(rc);
}

int cmpTime(const struct timespec* tp1, const struct timespec* tp2) {
    if (tp1->tv_sec == tp2->tv_sec) {
        if (tp1->tv_nsec > tp2->tv_nsec)
            return(1);
        else if (tp1->tv_nsec < tp2->tv_nsec)
            return(-1);
        else
            return(0);
    }
    else if (tp1->tv_sec > tp2->tv_sec)
        return(1);
    else
        return(-1);
}
