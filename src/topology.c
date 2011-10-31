#include "topology.h"

Network* Network_init(unsigned int id, char* name, char* type, unsigned int speed) {
    Network* thisNetwork = malloc(sizeof(Network));
    assert(thisNetwork != NULL);
    
    thisNetwork->id = id;
    thisNetwork->name = strdup(name);
    thisNetwork->type = strdup(type);
    thisNetwork->speed = speed;
    
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
    
    unsigned int i;
    for(i=0; i<numLinks; i++)
        replica->links[i] = allLinks[linkIDs[i]];
}

Topology* Topology_init(void) {
    Topology* topo = malloc(sizeof(Topology));
    assert(topo != NULL);

    topo->nodeCount = 0;
    topo->linkCount = 0;
    topo->allNodes = NULL;
    topo->allLinks = NULL;

    return(topo);
}

static void Topology_parseNetworkNode(Topology* topo, xmlNode* node) {
    // This could return static i, and on last call it could be set to
    // numLinks
    /* Networks just have attributes */
    static int i = 0;
    char* name = NULL;
    char* type = NULL;
    char* speed = NULL;

    xmlAttr* curAttr = node->properties;
    char* curAttrName = NULL;

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

    /* Add link to link list */
    topo->allLinks = (Network**)realloc(topo->allLinks, (i+1)*sizeof(Network*));
    // CHECK MEM
    topo->allLinks[i] = Network_init(i, name, type, (unsigned int)atoi(speed));

    /* Update link count */
    topo->linkCount = ++i;
}

static int Topology_matchName(Topology* topo, char* linkName) {
    /* Using linkName, return link ID */
    unsigned int i;
    for (i=0; i<topo->linkCount; i++)
        if(!strcmp(topo->allLinks[i]->name, linkName))
            break;
    return (i);
}

static void Topology_parseNodeNode(Topology* topo, xmlNode* node) {
    /* Nodes have attributes and children denoting links */
    static int i = 0;
    char* name = NULL;
    char* cpuCount = NULL;
    char* radHard = NULL;
    unsigned int* linkIDs = malloc(topo->linkCount*sizeof(int));
    unsigned int associatedLinkCount = 0;

    xmlAttr* curAttr = node->properties;
    char* curAttrName;

    /* Attributes of the node */
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
    bool rh = (strncmp(radHard,"False",1)) ? 0 : 1;

    /* Children (networks associated with the node) */
    xmlNode* child = node->children;
    for (; child; child = child->next) {
        curAttr = child->properties;
        for (; curAttr; curAttr = curAttr->next) {
            linkIDs[associatedLinkCount] = Topology_matchName(topo, (char*)curAttr->children->content);
            associatedLinkCount++;
        }
    }

    /* Add the node to node list */
    topo->allNodes = (Node**)realloc(topo->allNodes, (i+1)*sizeof(Node*));
    // CHECK MEM
    topo->allNodes[i] = Node_init(i, name, (unsigned char)atoi(cpuCount), rh);
    Node_addLinks(topo->allNodes[i], associatedLinkCount, linkIDs, topo->allLinks);

    /* Update node count */
    topo->nodeCount = ++i;

    free(linkIDs);
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

    /* Parse the file to get the DOM and root */
    doc = xmlReadFile(topoFileName, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Error: couldn't parse file %s\n", topoFileName);
        return (-1);
    }
    root_element = xmlDocGetRootElement(doc);

    /* Parse each element starting with the root */
    Topology_parseElement(topo, root_element);

    /* Clean up */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return (0);
}

void Topology_destroy(Topology* topo) {
    unsigned int i;
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

