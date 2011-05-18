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


class Message {
  public:
    Message();
    Message(message_t type, void* data, int dataLengt);
    ~Message();
    void set(message_t type, void* data, int dataLengt);
    message_t getType();
    int getDataLength();
    void* getData();
  private:
    message_t type;
    int dataLength;
    void* data;
};


class Connection {
  public:
    Connection();
    ~Connection();
    void initU(bool isServer, const char* path);
    void initI(bool isServer, unsigned short port, char* hostName);
    int send(const void* data, int msgSize = -1);
    void receive(Message& msg, double timeout = 0, clock_t start = clock()) throw (Except);
    void finish();
  private:
    int socketID;
};


//class Ham {
//  public:
//    Ham();
//    ~Ham();
//    void init(bool isServer);
//  private:
//    Connection link;
//};


timespec diff(timespec end, timespec begin);

#endif /* HAM_H_ */
