#ifndef _topology_h
#define _topology_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct Node {
    unsigned int id;
    char* name;
    unsigned char cpuCount;
    bool radHard;
    unsigned int numLinks;
    Network** links;
} Node;

typedef struct Network {
    unsigned int id;
    char* name;
    char* type;
    unsigned int speed;
} Network;

Node Node_init(unsigned int id, char* name, unsigned char cpuCount, bool radHard);

void Node_destroy(Node* replica);

void Node_addLinks(Node* replica, unsigned int numLinks, unsigned int* linkIDs, Network** allLinks);

Network* Network_init(unsigned int id; char* name; char* type; unsigned int speed);

void Network_destroy(Network* link);

#endif
