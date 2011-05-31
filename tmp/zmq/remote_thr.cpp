// vim:noexpandtab:ts=4:sw=4:
#include "../ham.h"

int main (int argc, char* argv[]){
    const char* connect_to;
    int message_count;
    int message_size;
    void* ctx;
    void* s;
    int rc;
    int i;
    zmq_msg_t msg;

    if (argc != 4) {
        printf("usage: remote_thr <connect-to> <tests> <message-count>\n");
        return 1;
    }
    connect_to = argv [1];
    message_size = atoi (argv [2]);
    message_count = atoi (argv [3]);

    ctx = zmq_init(1);
    if (!ctx) {
        printf ("error in zmq_init: %s\n", zmq_strerror (errno));
        return -1;
    }

    if (isServer)
	{
		timespec begin, end;
        unsigned long elapsed;
        double megabits;

		s = zmq_socket (ctx, ZMQ_SUB);
		if (!s) {
			printf("error in zmq_socket: %s\n", zmq_strerror (errno));
			return -1;
		}
		rc = zmq_setsockopt (s, ZMQ_SUBSCRIBE , "", 0);
		rc = zmq_bind (s, bind_to);

		// iteration
        for (int test = 0; test < testCnt; test++) {
			printf("Test %d\nMessage Size (Bytes), Realized Throughput (Mbit/s)\n", test);
			fflush(stdout);
			for (int j = 0; j < 26; j++) {
				// Make msg
				rc = zmq_msg_init (&msg);
				rc = zmq_recv (s, &msg, 0);
				if (zmq_msg_size (&msg) != message_size) {
					printf ("message of incorrect size received\n");
					return -1;
				}

				// Run test
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin); 
				for (i = 0; i != message_count - 1; i++) {
					rc = zmq_recv (s, &msg, 0);
					if (zmq_msg_size (&msg) != message_size) {
						printf ("message of incorrect size received\n");
						return -1;
					}
				}
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
				rc = zmq_msg_close(&msg);
				timespec ts_elapsed = diff(end, begin);
				elapsed = (long)ts_elapsed.tv_sec*1000000 + (long)ts_elapsed.tv_nsec/1000;
				
				// Return test metrics
				megabits = (double) (((double) messageCount * msgSize * 8)/((double) elapsed));
				printf("%d,%.2f\n", msgSize, (double) megabits);
	}
	else
	{
		s = zmq_socket(ctx, ZMQ_PUB);
		if (!s) {
			printf ("error in zmq_socket: %s\n", zmq_strerror (errno));
			return -1;
		}

		rc = zmq_connect(s, connect_to);
		if (rc != 0) {
			printf("error in zmq_connect: %s\n", zmq_strerror (errno));
			return -1;
		}

		for (i = 0; i != message_count; i++) {
			rc = zmq_msg_init_size(&msg, message_size);
			if (rc != 0) {
				printf("error in zmq_msg_init_size: %s\n", zmq_strerror (errno));
				return -1;
			}
			memset(zmq_msg_data (&msg), 0, message_size);

			rc = zmq_send(s, &msg, 0);
			if (rc != 0) {
				printf("error in zmq_send: %s\n", zmq_strerror (errno));
				return -1;
			}
			rc = zmq_msg_close(&msg);
			if (rc != 0) {
				printf("error in zmq_msg_close: %s\n", zmq_strerror (errno));
				return -1;
			}
		}
	}
    rc = zmq_close(s);
    if (rc != 0) {
        printf("error in zmq_close: %s\n", zmq_strerror (errno));
        return -1;
    }
    rc = zmq_term(ctx);
    if (rc != 0) {
        printf ("error in zmq_term: %s\n", zmq_strerror (errno));
        return -1;
    }
    return 0;
}
