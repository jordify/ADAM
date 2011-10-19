/***
 * File: topology.h
 * Author: Jorge Gomez
 * License: meh
 * Last Modified: Wed Oct 19, 2011 at 02:28
 ***
 */
#ifndef _logDB_h
#define _logDB_h

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Be less lazy and malloc these */
#define MAX_DATA 512
#define MAX_ROWS 100

typedef struct Address {
    int id;
    int set;
    char name[MAX_DATA];
    char email[MAX_DATA];
} Address;

typedef struct Database {
    struct Address rows[MAX_ROWS];
} Database;

typedef struct Connection {
    FILE* file;
    struct Database* db;
} Connection;


void Address_print(Address* addr);

void Database_load(Connection* conn);

Connection* Database_open(const char* filename, char mode);

void Database_close(Connection* conn);

void Database_write(Connection* conn);

void Database_create(Connection* conn);

void Database_set(Connection* conn, int id, const char* name, const char* email);

void Database_get(Connection *conn, int id);

void Database_delete(Connection* conn, int id);

void Database_list(Connection* conn);

/* Make all but this static, and make this varargs */
int Database_access(int argc, char* argv[]);

#endif
