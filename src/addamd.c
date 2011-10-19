#include <stdio.h>
#include <stdlib.h>
#include "topology.h"

int main(int argc, char* argv[]) {
    /* Check arguments */
    if (argc != 2) {
        printf("Usage: %s <topology file>\n", argv[0]);
        return (1);
    }

    /* Initialize the topology */
    Topology* topo = Topology_init(2, 4);

    /* Parse the static topology file into the topo */
    Topology_parseStatic(topo, argv[1]);

    printf("Topology parsed: %d links, %d nodes\n", topo->nodeCount, topo->linkCount);

    Topology_destroy(topo);
    return (0);
}
