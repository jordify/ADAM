#include "ham.h"

Ham* Ham_init(Topology* topo, int myID) {
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
    debug("notifier %s", notifierBindAddress);

    // Bind address for sub socket (needs to come from topology
    char listenerBindAddress[50];
    if(myID) {
        snprintf(listenerBindAddress, 49, "tcp://localhost:%d", basePort + myID-1);
        debug("listener %s", listenerBindAddress);
    }
    else {
        snprintf(listenerBindAddress, 49, "tcp://localhost:%d", basePort + topo->nodeCount-1);
        debug("listener %s", listenerBindAddress);
    }

    // Make sub socket 
    void* listener = zmq_socket(ctx, ZMQ_SUB);
    check(listener, "listener failed");
    zmq_setsockopt(listener, ZMQ_IDENTITY, myIdentity, strlen(myIdentity));
    zmq_setsockopt(listener, ZMQ_SUBSCRIBE, "", 0);
    zmq_connect(listener, listenerBindAddress);

    // Make pub socket
    void* notifier = zmq_socket(ctx, ZMQ_PUB);
    check(listener, "notifier failed");
    zmq_bind(notifier, notifierBindAddress);

    // Set new ham structure
    newHam->myID = myID;
    newHam->ctx = ctx;
    newHam->listener = listener;
    newHam->notifier = notifier;
    newHam->topo = topo;

    // return ham
    return(newHam);
error:
    if (newHam) free(newHam);
    return(0);
}

char* Ham_recv(Ham* ham) {
    return(NULL);
}

int Ham_beat(Ham* ham) {
    char hb[50]; 
    snprintf(hb, 49, "%d", ham->myID);
    debug("Beated");
    return(s_send(ham->notifier, hb));
}

void Ham_poll(Ham* ham, int timeout) {
    // Set up poll item
    zmq_pollitem_t items[] = {
        { ham->listener, 0, ZMQ_POLLIN, 0 }
    };

    zmq_msg_t message;
    zmq_poll(items, 1, timeout);
    if(items[0].revents & ZMQ_POLLIN) {
        zmq_msg_init(&message);
        zmq_recv(ham->listener, &message, 0);
        int size = zmq_msg_size(&message);
        char* string = malloc(size+1);
        memcpy(string, zmq_msg_data(&message), size);
        zmq_msg_close(&message);
        string[size] = 0;
        printf("%s\n", string);
        free(string);
    }
}

void Ham_destroy(Ham* ham) {
    zmq_close(ham->notifier);
    zmq_close(ham->listener);
    zmq_term(ham->ctx);
}

//  Sleep for a number of milliseconds
void mSleep (int msecs) {
    struct timespec t;
    t.tv_sec  =  msecs / 1000;
    t.tv_nsec = (msecs % 1000) * 1000000;
    nanosleep(&t, NULL);
}

//  Convert C string to 0MQ string and send to socket
int s_send (void *socket, char *string) {
    int rc;
    zmq_msg_t message;
    zmq_msg_init_size(&message, strlen (string));
    memcpy(zmq_msg_data (&message), string, strlen (string));
    rc = zmq_send(socket, &message, 0);
    zmq_msg_close(&message);
    return(rc);
}
