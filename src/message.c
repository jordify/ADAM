#include "message.h"

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

