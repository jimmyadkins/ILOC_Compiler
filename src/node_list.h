#ifndef NODE_LIST_H
#define NODE_LIST_H

// NodeList structure
typedef struct NodeList {
    void *data;               // Generic pointer to data
    struct NodeList *next;    // Pointer to the next node
} NodeList;

// Function declarations
NodeList *createNodeList();
void appendNode(NodeList *list, void *data);
void *removeNextNode(NodeList *list); // Return a generic pointer
int isNodeListEmpty(NodeList *list);
void freeNodeList(NodeList *list);

#endif
