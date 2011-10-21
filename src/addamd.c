#include <stdio.h>
#include <stdlib.h>
#include "topology.h"
#include "logDB.h"
#include "dbg.h"

int main(int argc, char* argv[]) {
    /* Check arguments */
    if (argc != 2) {
        printf("Usage: %s <topology file>\n", argv[0]);
        return(1);
    }

    /* Create the log */
    int rc = Database_access('c');
    check(rc==0, "Log creation failed");

    /* Initialize the topology */
    Topology* topo = Topology_init(2, 4);

    /* Parse the static topology file into the topo */
    Topology_parseStatic(topo, argv[1]);

    /* Log the parsing */
    char message[51];
    snprintf(message, 50, "Topology parsed: %d links, %d nodes", topo->nodeCount, topo->linkCount);
    rc = Database_access('s', 1, message);
    check(rc==0, "Log access failed");

    /* Print the log */
    Database_access('l');

    Topology_destroy(topo);
    return(0);
error:
    if(topo) Topology_destroy(topo);
    return(-1);
}
