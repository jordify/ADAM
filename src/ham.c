#include "ham.h"

Ham* Ham_init(Topology* topo, unsigned int myID) {
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

    // Create intial health states (-1:=Not Monitored, 0,1,2,...
    // :=Number of missed heartbeats)
    int* hbStates = malloc(topo->nodeCount*sizeof(int));
    for(i=0; i<topo->nodeCount; i++)
        hbStates[i] = -1;
    hbStates[myID] = 0;

    // Initialize a vote coordinator for this node's hb timeouts
    VoteCoord* coord = Vote_Startup(topo->nodeCount);
    check_mem(coord);

    // Set new ham structure
    newHam->myID = myID;
    newHam->ctx = ctx;
    newHam->listener = listener;
    newHam->notifier = notifier;
    newHam->topo = topo;
    newHam->hbStates = hbStates;
    newHam->coord = coord;

    // return ham
    return(newHam);
error:
    if (coord) Vote_Shutdown(coord);
    if (hbStates) free(hbStates);
    if (newHam) free(newHam);
    return(0);
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

void Ham_poll(Ham* ham, int timeout) {
// TODO: Add ipc REP socket for communicating with libAddam

    /* Set up poll item */
    zmq_pollitem_t items[] = {
        { ham->listener, 0, ZMQ_POLLIN, 0 }
    };

    zmq_msg_t message;
    zmq_poll(items, 1, timeout);
    if(items[0].revents & ZMQ_POLLIN) {
        zmq_msg_init(&message);
        zmq_recv(ham->listener, &message, 0);
        int size = zmq_msg_size(&message);
        Header* beat = malloc(sizeof(Header));
        memcpy(beat, zmq_msg_data(&message), size);
        zmq_msg_close(&message);
        Ham_procHB(ham, beat->source);
        free(beat);
    }
}

void Ham_timeoutHBs(Ham* ham) {
    unsigned int i;
    for(i=0; i<ham->topo->nodeCount; i++)
        if(ham->hbStates[i]>=0) {
            ham->hbStates[i]++;
            if(ham->hbStates[i] >= HBTIMEOUT) {
                debug("Node %d is dead", i);
                Vote_Coord_Init(ham->coord, i, 2, 1);
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
