#include "../include/zmq.h"
#include "../include/zmq_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv []) {
    const char* connect_to;
    int roundtrip_count;
    size_t message_size;
    void* ctx;
    void* s;
    int rc;
    int i;
    zmq_msg_t msg;

    void* watch;
    unsigned long elapsed;
    double latency;

    if (argc != 4) {
        printf("usage: remote_lat <connect-to> <message-size> <roundtrip-count>\n");
        return 1;
    }
    connect_to = argv[1];
    message_size = atoi(argv [2]);
    roundtrip_count = atoi(argv [3]);

    ctx = zmq_init(1);
    s = zmq_socket(ctx, ZMQ_REQ);
    rc = zmq_connect(s, connect_to);
    rc = zmq_msg_init_size(&msg, message_size);
    memset(zmq_msg_data(&msg), 0, message_size);

    watch = zmq_stopwatch_start();
    for (i = 0; i != roundtrip_count; i++) {
        rc = zmq_send(s, &msg, 0);
        rc = zmq_recv(s, &msg, 0);
        if (zmq_msg_size(&msg) != message_size) {
            printf("message of incorrect size received\n");
            return -1;
        }
    }
    elapsed = zmq_stopwatch_stop(watch);

    rc = zmq_msg_close(&msg);

    latency = (double) elapsed/(roundtrip_count * 2);

    printf("message size: %d [B]\n", (int) message_size);
    printf("roundtrip count: %d\n", (int) roundtrip_count);
    printf("average latency: %.3f [us]\n", (double) latency);

    rc = zmq_close(s);
    rc = zmq_term(ctx);

    return 0;
}
