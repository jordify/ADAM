#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "logDB.h"
#include "topology.h"
#include "ham.h"
#include "dbg.h"

int main(int argc, char* argv[]) {
    /* Some vars */
    Topology* topo = NULL;
    Ham* ham = NULL;
    int myID = 0;
    int logID = 1;
    char message[51];
    int databaseActive = 0;

    /* Check arguments */
    if (argc != 3) {
        sentinel("\n\tUsage: %s <topology file> <node ID>\n", argv[0]);
        return(1);
    }
    myID = atoi(argv[2]);

    /* Create the log */
    int rc = Database_access('c', myID);
    check(rc==0, "Log creation failed");
    databaseActive = 1;

    /* Initialize the topology */
    topo = Topology_init(1, 2);
    check(topo, "Couldn't initialize the topology.");

    /* Parse the static topology file into the topo */
    rc = Topology_parseStatic(topo, argv[1]);
    check(!rc, "Failed to parse the static topology file.");

    /* Initialize the HAM layer */
    ham = Ham_init(topo, myID);
    check(ham, "Failed to initialize the HAM");

    /* Log the parsing */
    snprintf(message, 50, "[%d] Topology parsed: %d links, %d nodes",
            myID, topo->nodeCount, topo->linkCount);
    rc = Database_access('s', myID, logID, message);
    check(rc==0, "Log set failed");
    logID++;

    /* Initiate time */
    struct timespec now, target;
    int i = 0;
    while(i<10) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        memcpy(&target, &now, sizeof(struct timespec));
        target.tv_sec = now.tv_sec + 1;
        while(cmpTime(&now, &target)==-1) {
            Ham_poll(ham, 1000000);
            clock_gettime(CLOCK_MONOTONIC, &now);
        }
        Ham_beat(ham);
        i++;
    }

    /* Print the log */
    rc = Database_access('l', myID);
    check(rc==0, "Log list failed");

    /* Kill the Ham and topology */
    Ham_destroy(ham);
    Topology_destroy(topo);
    return(0);

error:
    snprintf(message, 10, "[%d] Died", myID);
    if(databaseActive) Database_access('s', myID, logID, message);
    logID++;
    if(topo) Topology_destroy(topo);
    if(ham) Ham_destroy(ham);
    return(-1);
}

