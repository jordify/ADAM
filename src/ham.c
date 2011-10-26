#include "ham.h"

Ham* Ham_init(Network** links) {
    Ham* newHam = malloc(sizeof(Ham));
    assert(newHam != NULL);

    newHam->ctx = zmq_init(1);
    newHam->listener = zmq_socket(newHam->ctx, ZMQ_SUB);
    newHam->notifier = zmq_socket(newHam->ctx, ZMQ_PUB);
    newHam->links = links;

    /*
    newHam->ctx = zctx_new();
    assert(newHam->ctx);
    zctx_set_iothreads(newHam->ctx, 1);
    zctx_set_linger(newHam->ctx, 5);
    newHam->listner = zctx__socket_new(newHam->ctx, ZMQ_SUB);
    newHam->notifier = zctx__socket_new(newHam->ctx, ZMQ_PUB);
    newHam->links = links;
    */

    return(newHam);
}

void Ham_destroy(Ham* ham) {
    //zctx_destroy(&ham->ctx);
    zmq_close(ham->notifier);
    zmq_close(ham->listener);
    zmq_term(ham->ctx);
}
