// vim:noexpandtab:ts=4:sw=4:
#include "../ham.h"

timespec diff(timespec end, timespec start) {
	timespec temp;
	if ((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return temp;
}

int main (int argc, char* argv[]){
	bool isServer = false;
    const char* connect_to;
    int message_count;
	int testCnt;
    int message_size;
    void* ctx;
    void* s;
    int rc;
    int i;
    zmq_msg_t msg;

    if (argc < 4) {
        printf("usage: remote_thr <1 for server> <test-cnt> <message-count> connect-to>\n");
        return 1;
    }
	if (atoi(argv[1])==1) isServer = true;
	testCnt = atoi(argv[2]);
    message_count = atoi (argv [3]);
    connect_to = argv[4];

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
		rc = zmq_bind (s, "tcp://*:54321");

        for (int test = 0; test < testCnt; test++) {
			printf("Test %d\nMessage Size (Bytes), Realized Throughput (Mbit/s)\n", test);
			fflush(stdout);
			for (int j = 0; j < 26; j++) {
				// Make msg
				message_size = (int) pow(2,j);
				rc = zmq_msg_init (&msg);
				rc = zmq_recv (s, &msg, 0);
				if (zmq_msg_size(&msg) != message_size) {
					printf ("message of incorrect size received\n");
					return -1;
				}

				// Run test
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin); 
				for (i = 0; i != message_count - 1; i++) {
					rc = zmq_recv (s, &msg, 0);
					if (zmq_msg_size(&msg) != message_size) {
						printf ("message of incorrect size received\n");
						return -1;
					}
				}
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
				rc = zmq_msg_close(&msg);
				timespec ts_elapsed = diff(end, begin);
				elapsed = (long)ts_elapsed.tv_sec*1000000 + (long)ts_elapsed.tv_nsec/1000;
				
				// Return test metrics
				megabits = (double) (((double) message_count * message_size * 8)/((double) elapsed));
				printf("%d,%.2f\n", message_size, (double) megabits);
			}
		}
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

        for (int test = 0; test < testCnt; test++) {
			fflush(stdout);
			for (int j = 0; j < 26; j++) {
				// Make msg
				message_size = (int) pow(2,j);

				// Run test
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
