//
// Report ØMQ version
//
#include "zhelpers.h"

int main(void){
  int major, minor, patch;
  zmq_version(&major, &minor, &patch);
  printf("Current version of 0MQ is %d.%d.%d\n", major, minor, patch);
  return EXIT_SUCCESS;
}
