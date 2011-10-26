#include <stdio.h>
#include <stdlib.h>
#include "logDB.h"
#include "topology.h"
#include "ham.h"
#include "dbg.h"

int main(int argc, char* argv[]) {
    /* Three phases here:
     * 1)       Set-up and initialization
     *                  Parse topo
     *                  init log
     *                  set-up ham
     *                  activate sockets on links
     *                  set-up poller
     * 2)       Event loop
     *                  Beat heart
     *                  poll sockets
     *                          Handle events
     *                  pause until next hb event
     * 3)       Error handling and shutdown
     *                  gracefully shutdown
     *                  announce failure to log
     *                  CLOSE LOG
     *                  exit
     */
    /* Some vars */
    Topology* topo = NULL;
    Ham* ham = NULL;
    int myID = 0;
    int logID = 1;

    /* Check arguments */
    if (argc != 3) {
        sentinel("\n\tUsage: %s <topology file> <node ID>\n", argv[0]);
        return(1);
    }
    myID = atoi(argv[2]);

    /* Create the log */
    int rc = Database_access('c');
    check(rc==0, "Log creation failed");

    /* Initialize the topology */
    topo = Topology_init(2, 4);
    check(topo, "Couldn't initialize the topology.");

    /* Parse the static topology file into the topo */
    rc = Topology_parseStatic(topo, argv[1]);
    check(!rc, "Failed to parse the static topology file.");

    /* Initialize the HAM layer */
    ham = Ham_init(topo->allLinks);
    check(ham, "Failed to initialize the HAM");

    /* Log the parsing */
    char message[51];
    snprintf(message, 50, "Topology parsed: %d links, %d nodes", topo->nodeCount, topo->linkCount);
    rc = Database_access('s', logID, message);
    check(rc==0, "Log set failed");
    logID++;

    /* Print the log */
    rc = Database_access('l');
    check(rc==0, "Log list failed");

    /* Kill the Ham and topology */
    Ham_destroy(ham);
    Topology_destroy(topo);
    return(0);

error:
    if(topo) Topology_destroy(topo);
    if(ham) Ham_destroy(ham);
    return(-1);
}
