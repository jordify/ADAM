// vim:noexpandtab:ts=4:sw=4:
#include "ham.h"

// Connection --------------------------------------------------------
Connection::Connection() { //{{{
}

Connection::~Connection() {
	finish();
}

void Connection::initI(bool isServer, unsigned short portNum, char* hostName) {
	if (isServer) {
		struct sockaddr_in serv_addr, cli_addr;
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			perror("ERROR opening socket");
		memset(&serv_addr, '\0', sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portNum);
		socklen_t servlen = (socklen_t) sizeof(serv_addr);
		if (bind(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
			perror("ERROR on binding");
// 		int flag = 1; 
// 		setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)); 
		listen(sockfd, 5);
		socklen_t clilen = sizeof(cli_addr);
		socketID = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (socketID < 0)
			perror("ERROR on accept");
		int flags = fcntl(socketID, F_GETFL, 0);
		fcntl(socketID, F_SETFL, flags | O_NONBLOCK);
	} else {
		struct sockaddr_in serv_addr;
		struct hostent *server;
		memset(&serv_addr, '\0', sizeof(serv_addr));
		if ((server = gethostbyname(hostName)) == NULL)
			perror("ERROR, no such host");
		serv_addr.sin_family = AF_INET;
		memmove(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
		serv_addr.sin_port = htons(portNum);
		socklen_t servlen = sizeof(serv_addr);
		if ((socketID = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			perror("ERROR opening socket");
// 		int flag = 1; 
// 		setsockopt(socketID, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)); 
		int failNum = 0;
		while (connect(socketID, (struct sockaddr *) &serv_addr, servlen) < 0) {
			if (failNum >= 50) {
				perror("ERROR connecting");
				break;
			}
			failNum++;
			usleep(100000);
		}
	}
}

int Connection::send(const void* data, int msgSize) {
	if (msgSize == -1) {
		printf("ERRRR\n");
	}
	// Send length
	unsigned int n = 0;
	while (true) {
		//needs a time out mechanizm
		unsigned char *tempData = (unsigned char*) &msgSize;
		int written = 0;
		written = write(socketID, &tempData[n], 4 - n);
		if (written >= 0) {
			n += written;
			if (n >= 4) {
				break;
			}
		} else {
// 			perror("ERROR writing to socket"); 
		}
	}
	// Send msg
	int m = 0;
	while (true) {
		//needs a time out mechanizm
		int written = 0;
		unsigned char *tempData = (unsigned char*) data;
		written = write(socketID, &tempData[m], msgSize - m);
		if (written >= 0) {
			m += written;
			if (m >= msgSize) {
				break;
			}
		}
		else {
// 			perror("ERROR writing to socket"); 
		}
	}
	if (n != 4 || m != msgSize) {
		printf("N = %i, M = %i\n", n, m);
		printf("Bad msgSize is %i\n", msgSize);
		return -1;
	} else
		return m;
}

void* Connection::receive(int &dataLen, double timeout, clock_t start) throw (Except) {
	unsigned int dataLength = 0;
	// Grab header
	unsigned int n = 0;
	while (1) {
		unsigned char *tempData = (unsigned char*) &dataLength;
		int bytesRead = read(socketID, &tempData[n], 4-n);
		if (bytesRead == -1) {
			if (clock() - start >= CLOCKS_PER_SEC * timeout && timeout != 0) {
				throw(eTIMEOUT);
			} else {
				continue;
			}
		} else if (bytesRead < 0) {
			throw(eERROR);
		} else {
			n += bytesRead;
		}
		if (n >= 4) {
			break;
		}
	}
// 	printf("dataLength in Hex: %x\n", dataLength); 

	// Grab message
	void* data = malloc(dataLength);
	memset(data, 0, dataLength);

	n = 0;
	unsigned char *tempData = (unsigned char*) data;
	while (1) {
		int bytesRead = read(socketID, &tempData[n], dataLength - n);
		if (bytesRead == -1) {
			if (clock() - start >= CLOCKS_PER_SEC * timeout && timeout != 0)
				throw(eTIMEOUT);
			else {
				continue;
			}
		} else if (bytesRead < 0)
			throw(eERROR);
		else {
			n += bytesRead;
		}
		if (n >= dataLength) {
			break;
		}
	}
	dataLen = dataLength;
	return data;
}

void Connection::finish() {
	close( socketID);
	close( sockfd);
} // }}}

// MAIN --------------------------------------------------------------
int main(int argc, char* argv[]) {
	if (argc < 4) {
		printf("Usage:\n\t%s <0 or 1 (server==1)> <test count> <roundtrip count> <server hostname>\n", argv[0]);
		exit(1);
	}
	bool isServer = false;
	if (atoi(argv[1]) == 1)
		isServer = true;
	int testCnt = atoi(argv[2]);
	int messageCount = atoi(argv[3]);

	int n;
    int msgSize;
    char* msgBody;

    // Set make a connection object
    Connection* pipe;
    pipe = new Connection();

	if (isServer)
	{
		timespec begin, end;
        unsigned long elapsed;
        double megabits;

		pipe->initI(isServer, 54321, "");
		sleep(1);
        
        for (int test = 0; test < testCnt; test++) {
			printf("Test %d\nMessage Size (Bytes), Realized Throughput (Mbit/s)\n", test);
			fflush(stdout);
			for (int j = 0; j < 26; j++) {
				// Make msg
				msgSize = (int) pow(2,j);

				// Run test
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin); 
				for (int i = 0; i < messageCount; i++) {
					int msgSizeRec = 0;
					char* recvMsg = (char*) pipe->receive(msgSizeRec);
					if (msgSizeRec != msgSize) {
						printf("Message of incorrect size received\n");
						return -1;
					}
					free(recvMsg);
				}
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
				timespec ts_elapsed = diff(end, begin);
				elapsed = (long)ts_elapsed.tv_sec*1000000 + (long)ts_elapsed.tv_nsec/1000;
				
				// Return test metrics
				megabits = (double) (((double) messageCount * msgSize * 8)/((double) elapsed));
				printf("%d,%.2f\n", msgSize, (double) megabits);
			}
        }
	}
	else
	{
		pipe->initI(isServer, 54321, argv[4]);
		sleep(1);

        for (int test = 0; test < testCnt; test++) {
			for (int j = 0; j < 26; j++) {
				// Make msg
				msgSize = (int) pow(2,j);
				msgBody = (char*) malloc(msgSize + 1);
				if (msgBody == NULL) {
					printf("Couldn't malloc msgBody\n");
					exit(1);
				}
				memset(msgBody, 'a', msgSize);
				msgBody[msgSize + 1] = '\0';

				// Run test
				for (int i = 0; i < messageCount; i++) {
					n = pipe->send(msgBody, msgSize);
					if (n != msgSize) {
						printf("%s\n", msgBody);
						printf("Message of incorrect size sent\n");
						// ^^ Error msg that make you look like an idiot if is pops up
						return -1;
					}
				}
			}
		}
		free(msgBody);
	}
    delete(pipe);
	return 0;
}

// Helper Functions --------------------------------------------------
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
