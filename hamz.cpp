#include "ham.h"


// Connection --------------------------------------------------------
Connection::Connection() {
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
    socklen_t servlen = (socklen_t)sizeof(serv_addr);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
      perror("ERROR on binding");
    listen(sockfd,5);
    socklen_t clilen = sizeof(cli_addr);
    socketID = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
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
    int failNum = 0;
    while (connect(socketID,(struct sockaddr *)&serv_addr, servlen) < 0) {
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
  if (msgSize == -1) msgSize = strlen((char*)data);
  int n = write(socketID, &msgSize, 4);
  int m = write(socketID, data, msgSize);
  if (n != 4 || m != msgSize) {
    printf("N = %i, M = %i\n", n, m);
    printf("Bad msgSize is %i\n", msgSize);
    printf("Bad data is %s\n", (char*)data);
    perror("ERROR writing to socket");
    return -1;
  } else return m;
}

void* Connection::receive(double timeout, clock_t start) throw (Except) {
  int dataLength = 0;
  // Grab header
  while (1) {
    int n = read(socketID, &dataLength, 4);
    if (n == -1) {
      if (clock()-start >= CLOCKS_PER_SEC*timeout && timeout != 0) throw(eTIMEOUT);
      else usleep(1000);
    } else if (n <= 0) throw(eERROR);
    else if (n != 4) {
      printf("dataLength error: %d\n", dataLength);
      throw(eBADDATA);
    } else break;
  }
  printf("dataLength: %d\n", dataLength);
  // Grab message
  void* data = malloc(dataLength);
  while (1) {
    int n = read(socketID, data, dataLength);
    if (n == -1) {
      if (clock()-start >= CLOCKS_PER_SEC*timeout && timeout != 0) throw(eTIMEOUT);
      else usleep(1000);
    } else if (n <= 0) throw(eERROR);
    else if (n != dataLength) {
      printf("data error: %d: %s\n", dataLength, (char*)data);
      throw(eBADDATA);
    } else break;
  }
  //printf("data: %d: %s\n", dataLength, (char*)data);
  return data;
}

void Connection::finish() {
  close(socketID);
  close(sockfd);
}


// MAIN --------------------------------------------------------------
int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Usage:\n\t%s <0 or 1 (server==1)> <message size>\n", argv[0]); 
    exit(1);
  }
  bool isServer = false;
  if (atoi(argv[1])==1) isServer = true;
  int msgSize = atoi(argv[2]);
  printf("Message Size: %d\n", msgSize);

  int n;

  // Make msg
  char* msgBody = (char*) malloc(msgSize+1);
  if (msgBody == NULL){
    printf("Couldn't malloc msgBody\n");
    exit(1);
  }
  memset(msgBody, 'a', msgSize);
  msgBody[msgSize+1] = '\0';
  printf("msg is: %d bytes\n", strlen(msgBody));

  if (isServer) {
    timespec begin, end;
    printf("Hi! I'm a HAM node who is setting up a socket.\n");
    Connection* pipe;
    pipe = new Connection();
    pipe->initI(isServer, 54321, "");
    printf("Communicating on port 54321\n");
    sleep(1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);
    for(int i=0; i<3; i++){
      try {
        char* recvMsg = (char*)pipe->receive();
        printf("Received %d bytes\n", strlen(recvMsg));
        printf("Receiver %s\n", recvMsg);
      }
      catch(Except err){
        if (err==eTIMEOUT) printf("Failed with timeout\n");
        if (err==eBADDATA) printf("Failed with bad data\n");
        if (err==eFAILURE) printf("Failed with failure\n");
        if (err==eERROR) printf("Failed with error\n");
        exit(1);
      }
      printf(".");
      fflush(stdout);
      n = pipe->send(msgBody);
      printf("Sent %d bytes\n", n);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    printf("\n");
    printf("Done communicating\n");
    timespec elapsed = diff(end, begin);
    printf("Time: %d.%d\n", elapsed.tv_sec, elapsed.tv_nsec);
    sleep(1);
    delete(pipe);
  } else {
    timespec begin, end;
    printf("Hi! I'm a HAM node connecting to a socket.\n");
    Connection* pipe;
    pipe = new Connection();
    pipe->initI(isServer, 54321, "compute-0-14.local");
    printf("Communicating on port 54321\n");
    sleep(1);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);
    for(int i=0; i<3; i++){
      n = pipe->send(msgBody);
      printf("Sent %d bytes\n", n);
      try {
        char* recvMsg = (char*)pipe->receive();
        printf("Received %d bytes\n", strlen(recvMsg));
        printf("Receiver %s\n", recvMsg);
      }
      catch(Except err){
        if (err==eTIMEOUT) printf("Failed with timeout\n");
        if (err==eBADDATA) printf("Failed with bad data\n");
        if (err==eFAILURE) printf("Failed with failure\n");
        if (err==eERROR) printf("Failed with error\n");
      }
      printf(".");
      fflush(stdout);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    printf("\n");
    printf("Done communicating\n");
    timespec elapsed = diff(end, begin);
    printf("Time: %d.%d\n", elapsed.tv_sec, elapsed.tv_nsec);
    sleep(1);
    delete(pipe);
  }
  free(msgBody);
  return 0;
}


// Helper Functions --------------------------------------------------
timespec diff(timespec end, timespec start){
  timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}
