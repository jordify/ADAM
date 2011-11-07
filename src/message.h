/***
 * File: message.h
 * Author: Jorge Gomez
 * License: meh
 ***/
#ifndef __message_h__
#define __message_h__

#include <stdlib.h>
#include <stdio.h>
#include "dbg.h"

typedef enum Opcodes {
  o_HEARTBEAT,
  o_VOTEREQ,
  o_VOTEYES,
  o_VOTENO,
  o_KILLACK,
  o_KILLNACK
} Opcodes;

typedef struct Header {
  unsigned char source;
  unsigned char destination;
  unsigned char opcode;
  unsigned char checkSum;
} Header;

Header* Header_init(unsigned char source, unsigned char destination, Opcodes opcode);

void Header_destroy(Header* header);

#endif

