#include "node_list.h"
#include <stdlib.h>
#include <stdio.h>

// Create a new NodeList
NodeList *createNodeList() {
    NodeList *list = (NodeList *)malloc(sizeof(NodeList));
    if (!list) {
        fprintf(stderr, "Error: Failed to allocate memory for NodeList\n");
        exit(EXIT_FAILURE);
    }
    list->data = NULL; // Sentinel node
    list->next = NULL;
    return list;
}

// Append a node to the list
void appendNode(NodeList *list, void *data) {
    while (list->next) {
        list = list->next;
    }
    NodeList *newNode = (NodeList *)malloc(sizeof(NodeList));
    if (!newNode) {
        fprintf(stderr, "Error: Failed to allocate memory for NodeList node\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;
    list->next = newNode;
}

// Remove the next node and return its data
void *removeNextNode(NodeList *list) {
    if (!list || !list->next) {
        return NULL;
    }
    NodeList *toRemove = list->next;
    void *data = toRemove->data;
    list->next = toRemove->next;
    free(toRemove);
    return data;
}

// Check if the list is empty
int isNodeListEmpty(NodeList *list) {
    return (list->next == NULL);
}

// Free the NodeList
void freeNodeList(NodeList *list) {
    while (list) {
        NodeList *next = list->next;
        free(list);
        list = next;
    }
}
