#include "scheduler.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

int getLatency(int opcode) {
    switch (opcode) {
        case LOAD:
        case STORE:
            return 3;
        case MULT:
            return 2;
        default:
            return 1;
    }
}

DependencyGraph *createDependencyGraph(IR *ir) {
    printIR(ir, PRETTY_PRINT);
    debug(1, "Creating dependency graph");

    int maxSR = ir->count+1;
    debug(1, "Maximum VR index: %d", maxSR);

    int *VRtoNode = (int *)malloc((maxSR + 1) * sizeof(int));
    int *lastStoreMap = (int *)malloc((maxSR + 1) * sizeof(int));
    for (int i = 0; i <= maxSR; i++) {
        VRtoNode[i] = -1; 
        lastStoreMap[i] = -1;
    }

    // Allocate the dependency graph
    DependencyGraph *graph = (DependencyGraph *)malloc(sizeof(DependencyGraph));
    graph->nodes = (GraphNode **)malloc(ir->count * sizeof(GraphNode *));
    graph->nodeCount = ir->count;

    // Track last STORE and OUTPUT nodes
    int lastStore = -1;
    int lastOutput = -1;

    List *current = ir->instructions->next;
    NodeList *trackedLoads = createNodeList();
    int nodeIndex = 0;

    while (current) {
        IRLine *line = current->head;
        GraphNode *node = (GraphNode *)malloc(sizeof(GraphNode));
        

        node->label = nodeIndex + 1;
        node->instruction = line;
        node->dependencies = createNodeList();
        node->parents = createNodeList();

        switch (line->opcode) {
            case LOADI:
                VRtoNode[line->dst.vr] = node->label;
                break;

            case LOAD:
                if (line->src1.vr != -1 && VRtoNode[line->src1.vr] != -1) {
                    appendNode(node->dependencies, graph->nodes[VRtoNode[line->src1.vr] - 1]);
                }
                if (lastStore != -1) {
                    appendNode(node->dependencies, graph->nodes[lastStore - 1]);
                }
                VRtoNode[line->dst.vr] = node->label;

                appendNode(trackedLoads, node); // Track current load
                break;

            case STORE:
                if (line->src1.vr != -1 && VRtoNode[line->src1.vr] != -1) {
                    appendNode(node->dependencies, graph->nodes[VRtoNode[line->src1.vr] - 1]);
                }
                if (line->src2.vr != -1 && VRtoNode[line->src2.vr] != -1) {
                    appendNode(node->dependencies, graph->nodes[VRtoNode[line->src2.vr] - 1]);
                }
                if (lastStore != -1) {
                    if (!nodeExistsInList(node->dependencies, graph->nodes[lastStore - 1])) {
                        appendNode(node->dependencies, graph->nodes[lastStore - 1]);
                    }
                }
                if (lastOutput != -1) {
                    if (!nodeExistsInList(node->dependencies, graph->nodes[lastOutput - 1])) {
                        appendNode(node->dependencies, graph->nodes[lastOutput - 1]);
                    }
                }

                // Add dependencies on tracked loads
                NodeList *loadList = trackedLoads->next;
                while (loadList) {
                    GraphNode *loadNode = (GraphNode *)loadList->data;
                    if (!nodeExistsInList(node->dependencies, loadNode)) {
                        appendNode(node->dependencies, loadNode);
                    }
                    loadList = loadList->next;
                }
                freeNodeList(trackedLoads);
                trackedLoads = createNodeList();

                VRtoNode[line->src2.vr] = node->label;
                lastStore = node->label;
                break;


            case OUTPUT:
                if (lastOutput != -1) {
                    appendNode(node->dependencies, graph->nodes[lastOutput - 1]);
                }
                if (lastStore != -1) {
                    appendNode(node->dependencies, graph->nodes[lastStore - 1]);
                }
                lastOutput = node->label;
                break;

            case NOP:
                break;

            default:
                if (line->src1.vr != -1 && VRtoNode[line->src1.vr] != -1) {
                    appendNode(node->dependencies, graph->nodes[VRtoNode[line->src1.vr] - 1]);
                }
                if (line->src2.vr != -1 && VRtoNode[line->src2.vr] != -1) {
                    appendNode(node->dependencies, graph->nodes[VRtoNode[line->src2.vr] - 1]);
                }
                VRtoNode[line->dst.vr] = node->label;
                break;
        }

        graph->nodes[nodeIndex++] = node;
        current = current->next;
    }

    freeNodeList(trackedLoads);
    free(VRtoNode);
    return graph;
}

// probably not ideal but it works
int nodeExistsInList(NodeList *list, GraphNode *node) {
    NodeList *current = list->next;
    while (current) {
        if (current->data == node) {
            return 1; 
        }
        current = current->next;
    }
    return 0; 
}

int calcWeights(GraphNode *node) {
    if (node->weight > 0) {
        return node->weight;
    }

    int maxWeight = 0;
    NodeList *parents = node->parents->next;

    while (parents) {
        GraphNode *parent = (GraphNode *)parents->data;
        int parentWeight = calcWeights(parent);
        if (parentWeight > maxWeight) {
            maxWeight = parentWeight;
        }
        parents = parents->next;
    }

    node->weight = getLatency(node->instruction->opcode) + maxWeight;
    return node->weight;
}

void computeLatencies(DependencyGraph *graph) {
    for (int i = 0; i < graph->nodeCount; i++) {
        GraphNode *node = graph->nodes[i];
        NodeList *deps = node->dependencies->next;

        while (deps) {
            GraphNode *dep = (GraphNode *)deps->data;
            appendNode(dep->parents, (void *)node);
            deps = deps->next;
        }
    }

    for (int i = 0; i < graph->nodeCount; i++) {
        calcWeights(graph->nodes[i]);
    }
}

void printDependencyGraph(DependencyGraph *graph) {
    printf("nodes:\n");
    for (int i = 0; i < graph->nodeCount; i++) {
        GraphNode *node = graph->nodes[i];
        printf("    n%d : ", node->label);
        prettyPrintInstructionVRs(node->instruction);
    }

    printf("\nedges:\n");
    for (int i = 0; i < graph->nodeCount; i++) {
        GraphNode *node = graph->nodes[i];
        printf("    n%d : { ", node->label);
        NodeList *deps = node->dependencies->next;
        while (deps) {
            GraphNode *dep = (GraphNode *)deps->data;
            printf("n%d", dep->label);
            if (deps->next) {
                printf(", ");
            }
            deps = deps->next;
        }
        printf(" }\n");
    }

    printf("\nweights:\n");
    for (int i = 0; i < graph->nodeCount; i++) {
        GraphNode *node = graph->nodes[i];
        printf("    n%d : %d\n", node->label, node->weight);
    }
}

void freeDependencyGraph(DependencyGraph *graph) {
    for (int i = 0; i < graph->nodeCount; i++) {
        GraphNode *node = graph->nodes[i];
        freeNodeList(node->dependencies);
        freeNodeList(node->parents);
        free(node);
    }
    free(graph->nodes);
    free(graph);
}
