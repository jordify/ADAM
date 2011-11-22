#include "logDB.h"

static void Address_print(Address* addr) {
    printf("%d: %s\n", addr->id, addr->message);
}

static void Database_load(Connection* conn) {
    int rc = fread(conn->db, sizeof(Database), 1, conn->file);
    check(rc == 1, "Failed to load database.");

error:
    return;
}

static Connection* Database_open(const char* filename, char mode) {
    Connection* conn = (Connection*)malloc(sizeof(Connection));
    check(conn, "Memory error")

    conn->db = (Database*)malloc(sizeof(Database));
    check(conn->db, "Memory error");

    if (mode == 'c') {
        conn->file = fopen(filename, "w");
    } else {
        conn->file = fopen(filename, "r+");
        if (conn->file) {
            Database_load(conn);
        }
    }

    check(conn->file, "Failed to open the file");
    return conn;

error:
    if(conn->db) free(conn->db);
    conn->db = NULL;
    if(conn) free(conn);
    conn = NULL;
    return(NULL);
}

static void Database_close(Connection* conn) {
    if (conn) {
        if (conn->file) fclose(conn->file);
        if (conn->db) free(conn->db);
        conn->db = NULL;
        conn->file = NULL;
        free(conn);
        conn = NULL;
    } else {
        sentinel("No conn pointer");
    }

error:
    free(conn);
    conn = NULL;
}

static int Database_write(Connection* conn) {
    rewind(conn->file);

    int rc = fwrite(conn->db, sizeof(Database), 1, conn->file);
    check(rc==1, "Failed to write database.");

    rc = fflush(conn->file);
    check(!(rc == -1), "Cannot flush database RC=%d", rc);

    return(0);

error:
    return(-1);
}

static void Database_create(Connection* conn) {
    int i;
    for(i = 0; i < MAX_ROWS; i++) {
        // make a prototype to initialize it
        Address addr = {.id = i, .set = 0};
        // then assign it
        conn->db->rows[i] = addr;
    }
}

static int Database_set(Connection* conn, int id, const char* message) {
    Address* addr = &(conn->db->rows[id]);
    check(!(addr->set), "Already set, delete it first.");

    addr->set = 1;
    char* res = strncpy(addr->message, message, MAX_DATA);
    check(res, "Message copy failed.");
    return(0);

error:
    return(1);
}

static void Database_get(Connection *conn, int id) {
    Address* addr = &(conn->db->rows[id]);

    if (addr->set) {
        Address_print(addr);
    } else {
        sentinel("Row is not set");
    }

error:
    return;
}

static void Database_delete(Connection* conn, int id) {
    Address addr = {.id = id, .set = 0};
    conn->db->rows[id] = addr;
}

static void Database_list(Connection* conn) {
    int i;
    Database* db = conn->db;

    for(i = 0; i < MAX_ROWS; i++) {
        Address* cur = &db->rows[i];
        if (cur->set) {
            Address_print(cur);
        }
    }
}

int Database_access(char command, ...) {
    va_list argp;
    va_start(argp, command);
    char filename[50];
    char* message = NULL;
    int id = 0;
    int rc = -1;
    Connection* conn = NULL;

    int filenameID = va_arg(argp, int);
    //debug("Log file ID is %d", filenameID);
    if(filenameID < 0)
        sentinel("Need a fileID to use log.");
    snprintf(filename, 49, "node%d.db", filenameID);

    conn = Database_open(filename, command);
    check(conn, "Couldn't open db");

    switch(command) {
        case 'c':
            Database_create(conn);
            Database_write(conn);
            break;
        case 'g':
            id = va_arg(argp, int);
            if(id<=0) {
                sentinel("Need an id to get.");
            } else {
                Database_get(conn, id);
            }
            break;
        case 's':
            id = va_arg(argp, int);
            if(id<=0) {
                sentinel("Need an id to set.");
            } else {
                message = va_arg(argp, char*);
                if(!message) {
                    sentinel("Need an id and message to set.");
                } else {
                    rc = Database_set(conn, id, message);
                    check(rc==0, "Failed to set memory.");
                    Database_write(conn);
                }
            }
            break;
        case 'd':
            id = va_arg(argp, int);
            if(id<=0) {
                sentinel("Need an id to delete.");
            } else {
                Database_delete(conn, id);
                Database_write(conn);
            }
            break;
        case 'l':
            Database_list(conn);
            break;
        default:
            sentinel("Invalid action, only: c=create, g=get, s=set, d=del, l=list");
    }

    Database_close(conn);

    return(0);

error:
    if(conn) Database_close(conn);
    return(-1);
}
