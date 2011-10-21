#include <stdio.h>
#include <stdlib.h>
#include "topology.h"
#include "dbg.h"

int main(int argc, char* argv[]) {
    /* Check arguments */
    if (argc != 2) {
        sentinel("\n\tUsage: %s <topology file>\n", argv[0]);
        return(1);
    }

    /* Create the log */
    int rc = system("./logDB c");
    check(rc==0, "Log creation failed");

    /* Initialize the topology */
    Topology* topo = Topology_init(2, 4);
    check(topo, "Couldn't initialize the topology.");

    /* Parse the static topology file into the topo */
    rc = Topology_parseStatic(topo, argv[1]);
    check(!rc, "Failed to parse the static topology file.");

    /* Log the parsing */
    char message[51];
    snprintf(message, 50, "./logDB s 1 \"Topology parsed: %d links, %d nodes\"", topo->nodeCount, topo->linkCount);
    rc = system(message);
    check(rc==0, "Log access failed");

    /* Print the log */
    rc = system("./logDB l");
    check(rc==0, "Log access failed");

    Topology_destroy(topo);
    return(0);

error:
    if(topo) Topology_destroy(topo);
    return(-1);
}
