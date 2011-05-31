#ifndef HAM_H_
#define HAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
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


enum message_t {mFAILURE, mUNKNOWN, mEMPTY, mHEARTBEAT, mDATA};
enum Except {eTIMEOUT, eBADDATA, eFAILURE, eERROR};


class Connection {
  public:
    Connection();
    ~Connection();
    void initI(bool isServer, unsigned short port, char* hostName);
    int send(const void* data, int msgSize = -1);
    //void* receive(Message& msg, double timeout = 0, clock_t start = clock()) throw (Except);
    void* receive(double timeout = 0, clock_t start = clock()) throw (Except);
    void finish();
  private:
    int socketID;
    int sockfd;
};


timespec diff(timespec end, timespec begin);

#endif /* HAM_H_ */
