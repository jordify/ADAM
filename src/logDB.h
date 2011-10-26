/***
 * File: logDB.h
 * Author: Jorge Gomez
 * License: meh
 * Last Modified: Wed Oct 26, 2011 at 14:51
 ***/
#ifndef __logDB_h__
#define __logDB_h__

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "dbg.h"

#define MAX_ROWS 512
#define MAX_DATA 100

typedef struct Address {
    int id;
    int set;
    char message[MAX_DATA];
} Address;

typedef struct Database {
    Address rows[MAX_ROWS];
} Database;

typedef struct Connection {
    FILE* file;
    Database* db;
} Connection;

int Database_access(char command, ...);
/* Commands (for now):
 *      'c'                     - create a new database
 *      'g', int id             - get message at id
 *      's', int id, char* msg  - set id with message "msg"
 *      'd', int id             - delete entry at id
 *      'l'                     - list all id, message pairs
 *
 * Should add some logic to delete stale db values, search, etc.
 */

#endif
