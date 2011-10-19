#include "topology.h"

Network* Network_init(unsigned int id, char* name, char* type, unsigned int speed) {
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

    replica->links = malloc(numLinks*sizeof(Network*));
    assert(replica->links != NULL);
    
    int i;
    for(i=0; i<numLinks; i++)
        replica->links[i] = allLinks[linkIDs[i]];
}

Topology* Topology_init(unsigned int numLinks, unsigned int numNodes) {
    int i;
    /* Make all nodes */
    Node** allNodes = malloc(numNodes*sizeof(Node*));
    for (i=0; i<numNodes; i++)
        allNodes[i] = malloc(sizeof(Node));

    /* Make all links */
    Network** allLinks = malloc(numLinks*sizeof(Network*));
    for (i=0; i<numLinks; i++)
        allLinks[i] = malloc(sizeof(Network));


    Topology* topo = malloc(sizeof(Topology));
    assert(topo != NULL);

    topo->nodeCount = numNodes;
    topo->linkCount = numLinks;
    topo->allNodes = allNodes;
    topo->allLinks = allLinks;

    return(topo);
}

static void Topology_parseNetworkNode(Topology* topo, xmlNode* node) {
    /* Networks just have attributes
     * TODO: Fix mem leak
     */
    static int i = 0;
    char* name;
    char* type;
    char* speed;

    xmlAttr* curAttr = node->properties;
    char* curAttrName;

    for (; curAttr; curAttr = curAttr->next) {
        curAttrName = (char*)curAttr->name;
        if (!strcmp(curAttrName, "name"))
            name = (char*)curAttr->children->content;
        else if (!strcmp(curAttrName, "type"))
            type = (char*)curAttr->children->content;
        else if (!strcmp(curAttrName, "speed"))
            speed = (char*)curAttr->children->content;
        else
            fprintf(stderr, "Bad attribute in Network node\n");
    }
    printf("%s %s %d %d\n", name, type, atoi(speed), i);
    topo->allLinks[i]->id = i;
    topo->allLinks[i]->name = name;
    topo->allLinks[i]->type = type;
    topo->allLinks[i]->speed = atoi(speed);
    /** Causes glibc to crash
    free(name);
    free(type);
    free(speed);
    */
    i++;
}

static void Topology_parseNodeNode(Topology* topo, xmlNode* node) {
    /* Nodes have attributes and children denoting links
     * TODO: Implement Links, fix mem leak
     */
    static int i = 0;
    char* name;
    char* cpuCount;
    char* radHard;

    xmlAttr* curAttr = node->properties;
    char* curAttrName;

    for (; curAttr; curAttr = curAttr->next) {
        curAttrName = (char*)curAttr->name;
        if (!strcmp(curAttrName, "name"))
            name = (char*)curAttr->children->content;
        else if (!strcmp(curAttrName, "cpuCount"))
            cpuCount = (char*)curAttr->children->content;
        else if (!strcmp(curAttrName, "radHard"))
            radHard = (char*)curAttr->children->content;
        else
            fprintf(stderr, "Bad attribute in Network node\n");
    }
    printf("%s %s %d %d\n", name, radHard, atoi(cpuCount), i);
    topo->allNodes[i]->id = i;
    topo->allNodes[i]->radHard = (strncmp(radHard,"False",1)) ? 0 : 1;
    topo->allNodes[i]->cpuCount = (unsigned char) atoi(cpuCount);
    /** Causes glibc to crash
    free(name);
    free(cpuCount);
    free(radHard);
    */
    i++;
}

static void Topology_parseElement(Topology* topo, xmlNode* a_node) {
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (!strcmp((char*)cur_node->name,"network"))
                Topology_parseNetworkNode(topo, cur_node);
            else if (!strcmp((char*)cur_node->name,"node"))
                Topology_parseNodeNode(topo, cur_node);
        }
        Topology_parseElement(topo, cur_node->children);
    }
}

int Topology_parseStatic(Topology* topo, const char* topoFileName) {
    xmlDoc* doc = NULL;
    xmlNode* root_element = NULL;

    /* Init the libXML library */
    LIBXML_TEST_VERSION

    /* parse the file to get the DOM */
    doc = xmlReadFile(topoFileName, NULL, 0);
    if (doc == NULL) {
        printf("Error: couldn't parse file %s\n", topoFileName);
        return (-1);
    }

    root_element = xmlDocGetRootElement(doc);

    Topology_parseElement(topo, root_element);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return (0);
}

void Topology_destroy(Topology* topo) {
    int i;
    /* free all nodes */
    for (i=0; i<topo->nodeCount; i++)
        Node_destroy(topo->allNodes[i]);
    free(topo->allNodes);
    /* free all links */
    for (i=0; i<topo->linkCount; i++)
        Network_destroy(topo->allLinks[i]);
    free(topo->allLinks);
    /* free the topology */
    free(topo);
}
