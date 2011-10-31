#include <stdlib.h>
#include <stdio.h>
#include "dbg.h"

typedef enum Opcodes {
  o_HEARTBEAT,
  o_VOTEREQUEST,
  o_VOTEYES,
  o_VOTENO
} Opcodes;

typedef struct msgHeader {
  unsigned char source;
  unsigned char destination;
  unsigned char opcode;
  unsigned char checkSum;
} Header;

Header* Header_init(unsigned char source, unsigned char destination, Opcodes opcode) {
  Header* newHeader = (Header*)malloc(sizeof(Header));
  newHeader->source = source;
  newHeader->destination = destination;
  newHeader->opcode = (unsigned char)opcode;
  newHeader->checkSum = newHeader->source + newHeader->destination + newHeader->opcode;

  return(newHeader);
}

void Header_destroy(Header* header) {
  if(header) free(header);
}

