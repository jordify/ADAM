#ifndef HAM_H_
#define HAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/un.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <queue>
#include <iostream>
#include <zmq.h>
#include <zmq_utils.h>


enum Except {eTIMEOUT, eERROR};


class Connection {
	public:
		Connection();
		~Connection();
		void initI(bool isServer, unsigned short port, char* hostName);
		int send(const void* data, int msgSize = -1);
		//void* receive(Message& msg, double timeout = 0, clock_t start = clock()) throw (Except);
		void* receive(int dataLength, double timeout = 0, clock_t start = clock()) throw (Except);
		void finish();
	private:
		int socketID;
		int sockfd;
};


timespec diff(timespec end, timespec begin);

#endif /* HAM_H_ */

/* {{{
 * Debug:
 *
	printf("msg is: %d bytes\n", strlen(msgBody));
				printf("Received %d bytes\n", msgSizeRec);
				printf("Receiver %s\n", recvMsg);
			printf(".");
			fflush( stdout);
			printf("Sending %d bytes: %c%c%c%c\n", msgSizeRec, msgBody[msgSizeRec-4], msgBody[msgSizeRec-3], msgBody[msgSizeRec-2], msgBody[msgSizeRec-1]);
			printf("Sent %d bytes\n", n);
			try {
			} catch (Except err)
			{
				if (err == eTIMEOUT)
					printf("Failed with timeout\n");
				if (err == eERROR)
					printf("Failed with error\n");
				exit(1);
			}
 *
 * Timing:
 *
        timespec begin, end;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin); 
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
        timespec ts_elapsed = diff(end, begin);
        elapsed = (long)ts_elapsed.tv_sec*1000000 + (long)ts_elapsed.tv_nsec/1000;
        latency = (double) elapsed/(roundtripCount * 2); 
        }}}*/
