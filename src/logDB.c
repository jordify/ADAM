#include "logDB.h"

static void Address_print(Address* addr) {
    printf("%d %s\n", addr->id, addr->message);
}

static void Database_load(Connection* conn) {
    int rc = fread(conn->db, sizeof(Database), 1, conn->file);
    check(rc == 1, "Failed to load database.");

error:
    return;
}

static Connection* Database_open(const char* filename, char mode) {
    Connection* conn = malloc(sizeof(Connection));
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
    if(conn) free(conn);
    return(NULL);
}

static void Database_close(Connection* conn) {
    if (conn) {
        if (conn->file) fclose(conn->file);
        if (conn->db) free(conn->db);
        free(conn);
    }
}

static void Database_write(Connection* conn) {
    rewind(conn->file);

    int rc = fwrite(conn->db, sizeof(Database), 1, conn->file);
    check(rc==1, "Failed to write database.");

    rc = fflush(conn->file);
    check(!(rc == -1), "Cannot flush database RC=%d", rc);

error:
    return;
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

static void Database_set(Connection* conn, int id, const char* message) {
    Address* addr = &conn->db->rows[id];
    check(!(addr->set), "Already set, delete it first.");

    addr->set = 1;
    // WARNING: bug, read the "How To Break It" and fix this
    char* res = strncpy(addr->message, message, MAX_DATA);
    // demonstrate the strncpy bug
    check(res, "Message copy failed.");

error:
    return;
}

static void Database_get(Connection *conn, int id) {
    Address* addr = &conn->db->rows[id];

    if (addr->set) {
        Address_print(addr);
    } else {
        sentinel("ID is not set");
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
    char* filename = "log.db";
    char* message = NULL;
    int id = 0;

    Connection* conn = Database_open(filename, command);
    check(conn, "Couldn't open db");

    switch(command) {
        case 'c':
            Database_create(conn);
            Database_write(conn);
            break;
        case 'g':
            id = va_arg(argp, int);
            check(id > 0, "Failed to get ID");
            Database_get(conn, id);
            break;
        case 's':
            id = va_arg(argp, int);
            check(id > 0, "Failed to get ID");
            message = va_arg(argp, char*);
            check(message, "Failed to get Message to write");
            Database_set(conn, id, message);
            Database_write(conn);
            break;
        case 'd':
            id = va_arg(argp, int);
            check(id > 0, "Failed to get ID");
            Database_delete(conn, id);
            Database_write(conn);
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
