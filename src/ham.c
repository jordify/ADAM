#include "ham.h"

// Need node ID of my node
// Parse Topology
// Set-up sockets on this node's links
// set-up the poller
Ham* Ham_init(Network** links) {
    Ham* newHam = malloc(sizeof(Ham));
    assert(newHam != NULL);

    newHam->ctx = zmq_init(1);
    newHam->listener = zmq_socket(newHam->ctx, ZMQ_SUB);
    newHam->notifier = zmq_socket(newHam->ctx, ZMQ_PUB);
    newHam->links = links;
    return(newHam);
}

void Ham_destroy(Ham* ham) {
    zmq_close(ham->notifier);
    zmq_close(ham->listener);
    zmq_term(ham->ctx);
}
