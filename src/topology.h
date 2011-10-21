/***
 * File: topology.h
 * Author: Jorge Gomez
 * License: meh
 * Last Modified: Fri Oct 21, 2011 at 01:37
 ***/
#ifndef _topology_h
#define _topology_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/**
 * Requires libxml2
 * To compile use the output of `xml2-config --cflags --libs`:
 * -I/usr/include/libxml2
 * -lxml2 -lz -lm
 */
#include <libxml/parser.h>
#include <libxml/tree.h>

typedef struct Network {
    unsigned int id;
    char* name;
    char* type;
    unsigned int speed;
} Network;

typedef struct Node {
    unsigned int id;
    char* name;
    unsigned char cpuCount;
    bool radHard;
    unsigned int numLinks;
    Network** links;
} Node;

typedef struct Topology {
    unsigned int nodeCount;
    unsigned int linkCount;
    Node** allNodes;
    Network** allLinks;
} Topology;

Network* Network_init(unsigned int id, char* name, char* type, unsigned int speed);

void Network_destroy(Network* link);

Node* Node_init(unsigned int id, char* name, unsigned char cpuCount, bool radHard);

void Node_destroy(Node* replica);

void Node_addLinks(Node* replica, unsigned int numLinks, unsigned int* linkIDs, Network** allLinks);

Topology* Topology_init(unsigned int numLinks, unsigned int numNodes);

int Topology_parseStatic(Topology* topo, const char* topoFileName);

void Topology_destroy(Topology* topo);

#endif
