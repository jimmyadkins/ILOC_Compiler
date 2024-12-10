#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "IR.h"
#include "node_list.h"

// Graph node structure
typedef struct GraphNode {
    int label;                  // Node label
    IRLine *instruction;        // Associated instruction
    NodeList *dependencies;     // List of dependencies (edges)
    NodeList *parents;          // List of parent nodes (reverse edges)
    int weight;                 // Node weight for scheduling
} GraphNode;

// Dependency graph structure
typedef struct DependencyGraph {
    GraphNode **nodes;          // Array of graph nodes
    int nodeCount;              // Number of nodes
} DependencyGraph;

// Function declarations
DependencyGraph *createDependencyGraph(IR *ir);
void computeLatencies(DependencyGraph *graph);
void printDependencyGraph(DependencyGraph *graph);
void freeDependencyGraph(DependencyGraph *graph);
int nodeExistsInList(NodeList *list, GraphNode *node);

#endif
