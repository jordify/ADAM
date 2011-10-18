#include "topology.h"

Node* Node_init(unsigned int id, char* name, unsigned char cpuCount, bool radHard) {
    Node* replica = malloc(sizeof(Node));
    assert(replica != NULL);

    replica->id = id;
    replica->name = strdup(name);
    replica->cpuCount = cpuCount;
    replica->radHard = radHard;
    replica->numLinks = 0;
    replica->links = NULL;

    return (replica);
}

void Node_destroy(Node* replica) {
    assert(replica != NULL);

    free(replica->links);
    free(replica->name);
    free(replica);
}

void Node_addLinks(Node* replica, unsigned int numLinks, unsigned int* linkIDs, Network** allLinks) {
    replica->numLinks = numLinks;

    newLinks = malloc(numLinks*sizeof(Network*));
    assert(newLinks != NULL);
    
    int i;
    for(i=0; i<numLinks; i++)
        replica->links[i] = allLinks[linkIDs[i]];
}

Network* Network_init(unsigned int id; char* name; char* type; unsigned int speed) {
    Network* thisNetwork = malloc(sizeof(Network));
    assert(thisNetwork != NULL);
    
    thisNetwork->id = id;
    thisNetwork->name = strdup(name);
    thisNetwork->type = strdup(type);
    
    return(thisNetwork);
}

void Network_destroy(Network* link) {
    assert(link != NULL);

    free(link->name);
    free(link->type);
    free(link);
}
