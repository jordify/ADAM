#include "../include/zmq.h"
#include "../include/zmq_utils.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv []) {
    const char* bind_to;
    int message_count;
    size_t message_size;
    void* ctx;
    void* s;
    int rc;
    int i;
    zmq_msg_t msg;

    void* watch;
    unsigned long elapsed;
    unsigned long throughput;
    double megabits;

    if(argc != 4) {
        printf("usage: local_thr <1 for server> <message-size> <message-count> <bind-to>\n");
        return 1;
    }
    bind_to = argv [1];
    message_size = atoi (argv [2]);
    message_count = atoi (argv [3]);

    ctx = zmq_init (1);
    s = zmq_socket (ctx, ZMQ_SUB);
    rc = zmq_setsockopt (s, ZMQ_SUBSCRIBE , "", 0);
    rc = zmq_bind (s, bind_to);

    rc = zmq_msg_init (&msg);
    rc = zmq_recv (s, &msg, 0);
    if (zmq_msg_size (&msg) != message_size) {
        printf ("message of incorrect size received\n");
        return -1;
    }

    watch = zmq_stopwatch_start ();
    for (i = 0; i != message_count - 1; i++) {
        rc = zmq_recv (s, &msg, 0);
        if (zmq_msg_size (&msg) != message_size) {
            printf ("message of incorrect size received\n");
            return -1;
        }
    }
    elapsed = zmq_stopwatch_stop (watch);
    if (elapsed == 0) elapsed = 1;

    rc = zmq_msg_close (&msg);

    throughput = (unsigned long)
        ((double) message_count / (double) elapsed * 1000000);
    megabits = (double) (throughput * message_size * 8) / 1000000;

    printf ("message size: %d [B]\n", (int) message_size);
    printf ("message count: %d\n", (int) message_count);
    printf ("mean throughput: %d [msg/s]\n", (int) throughput);
    printf ("mean throughput: %.3f [Mb/s]\n", (double) megabits);

    rc = zmq_close (s);
    rc = zmq_term (ctx);

    return 0;
}
