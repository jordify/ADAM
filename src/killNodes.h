/***
 * File: killNodes.h
 * Author: Jorge Gomez
 * License: meh
 ***/
#ifndef __killNodes_h__
#define __killNodes_h__

#include <stdlib.h>
#include <stdio.h>
#include "dbg.h"

int KillNodes_kill(int systemSize, unsigned char nodeID);

int KillNodes_startOne(int systemSize, int targetID);

int KillNodes_startAll(int systemSize, int myID);

#endif //__killNodes_h__

