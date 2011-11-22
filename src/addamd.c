#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "logDB.h"
#include "topology.h"
#include "ham.h"
#include "dbg.h"

static int s_interrupted = 0;

static void s_signal_handler(int signal_value) {
    debug("Signal %d caught", signal_value);
    s_interrupted = 1;
}

static void s_catch_signals(void) {
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}

int main(int argc, char* argv[]) {
    /* Some vars */
    Topology* topo = NULL;
    Ham* ham = NULL;
    int myID = 0;
    int logID = 1;
    char message[51];
    int databaseActive = 0;
    int rc = 0;
    int pid = 0;

    /* Check arguments */
    if (argc != 3) {
        sentinel("\n\tUsage: %s <topology file> <node ID>\n", argv[0]);
        return(1);
    }

    /* Initialize the topology */
    topo = Topology_init();
    check(topo, "Couldn't initialize the topology.");

    /* Parse the static topology file into the topo */
    rc = Topology_parseStatic(topo, argv[1]);
    check(!rc, "Failed to parse the static topology file.");

    /* Get my ID */
    myID = (int)atoi(argv[2]);

    /* If -1 then try to automagically start system */
    if(myID==-1) {
        myID = Topology_detectID(topo); // Detect my ID from hostname
        if(myID==-1)
            myID = 0;
        KillNodes_startAll(topo->nodeCount, myID); // Start the other nodes
    }

    /* Create the log */
    rc = Database_access('c', myID);
    check(rc==0, "Log creation failed");
    databaseActive = 1;

    /* Initialize the HAM layer */
    ham = Ham_init(topo, myID);
    check(ham, "Failed to initialize the HAM");

    /* Log my PID to help kill me */
    pid = getpid();
    debug("My PID is %d", pid);
    snprintf(message, 50, "[%d] pid %d", myID, pid);
    rc = logSomething(ham, message);

    /* Log the topology parsing */
    snprintf(message, 50, "[%d] Topology parsed: %d links, %d nodes",
            myID, topo->linkCount, topo->nodeCount);
    rc = Database_access('s', myID, logID, message);
    check(rc==0, "Log set failed");
    logID++;

    /* Poll for events and heartbeat */
    struct timespec now, target;
    unsigned int i = 0;
    s_catch_signals();
    while(1) {
        /* Increment everyone's heartbeats */
        Ham_timeoutHBs(ham);

        /* Poll for incoming for one second */
        clock_gettime(CLOCK_MONOTONIC, &now);
        memcpy(&target, &now, sizeof(struct timespec));
        target.tv_sec = now.tv_sec + 1;
        while(cmpTime(&now, &target)==-1) {
            Ham_poll(ham, 1000000);
            clock_gettime(CLOCK_MONOTONIC, &now);
#ifndef NDEBUG
            /* Print the current hbState */
            for(i=0; i<topo->nodeCount; i++)
                printf("%d\t", ham->hbStates[i]);
            printf("\n");
#endif
        }
        /* Heart beat */
        Ham_beat(ham);

        if(s_interrupted)
            sentinel("Caught sigterm or sigint, nobly killing self...");
    }

    /* Kill the Ham and topology, won't ever really get here */
    Ham_destroy(ham);
    Topology_destroy(topo);
    return(0);

error:
    snprintf(message, 10, "[%d] Died", myID);
    /* Log my own death */
    if(databaseActive) {
        Database_access('s', myID, logID, message);
        logID++;
    }
    if(topo) Topology_destroy(topo);
    if(ham) Ham_destroy(ham);
    return(-1);
}

