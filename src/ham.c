#include "ham.h"

Ham* Ham_init(Topology* topo, int myID) {
    Ham* newHam = malloc(sizeof(Ham));
    check_mem(newHam);

    void* ctx = zmq_init(1);
    check(ctx, "Context Failed");

    int basePort = 5600;

    char myIdentity[50];
    snprintf(myIdentity, 49, "Node %d", myID);

    char notifierBindAddress[50];
    snprintf(notifierBindAddress, 49, "tcp://*:%d", basePort + myID);
    debug("notifier %s", notifierBindAddress);

    char listenerBindAddress[50];
    if(myID) {
        snprintf(listenerBindAddress, 49, "tcp://localhost:%d", basePort + myID-1);
        debug("listener %s", listenerBindAddress);
    }
    else {
        snprintf(listenerBindAddress, 49, "tcp://localhost:%d", basePort + topo->nodeCount-1);
        debug("listener %s", listenerBindAddress);
    }

    void* listener = zmq_socket(ctx, ZMQ_SUB);
    check(listener, "listener failed");
    zmq_setsockopt(listener, ZMQ_IDENTITY, myIdentity, strlen(myIdentity));
    zmq_setsockopt(listener, ZMQ_SUBSCRIBE, "", 0);
    zmq_connect(listener, listenerBindAddress);

    void* notifier = zmq_socket(ctx, ZMQ_PUB);
    check(listener, "notifier failed");
    zmq_bind(notifier, notifierBindAddress);

    newHam->myID = myID;
    newHam->ctx = ctx;
    newHam->listener = listener;
    newHam->notifier = notifier;
    newHam->topo = topo;
    return(newHam);

error:
    if (newHam) free(newHam);
    return(0);
}

char* Ham_recv(Ham* ham) {
    return(s_recv(ham->listener));
}

int Ham_beat(Ham* ham) {
    char hb[50]; 
    snprintf(hb, 49, "%d", ham->myID);
    return(s_send(ham->notifier, hb));
}

void Ham_destroy(Ham* ham) {
    zmq_close(ham->notifier);
    zmq_close(ham->listener);
    zmq_term(ham->ctx);
}

