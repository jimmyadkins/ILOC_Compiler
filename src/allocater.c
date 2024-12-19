#include "allocator.h"
#include "list.h"
#include "ir.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

static int spillMemoryBase = 32768;  // Starting address for spilled memory

void initAllocator(Allocator *allocator, IR *ir, int k) {
    if (ir == NULL) {
        printf("Error: IR is NULL\n");
        exit(EXIT_FAILURE);
    }
    if (ir->instructions == NULL) {
        printf("Error: IR instructions list is NULL\n");
        exit(EXIT_FAILURE);
    }
    allocator->ir = ir;
    allocator->k = k;
    allocator->live = 0;
    allocator->lastStore = 0;
    allocator->currentInstructionIndex = 0;
    allocator->nextSpillLocation = spillMemoryBase;
    allocator->maxRegisters = getMaxSR(ir->instructions);

    if (allocator->ir->count <= 0) {
        printf("Warning: IR count is zero or uninitialized\n");
    }

    allocator->VRtoPR = (int *)malloc(ir->count * sizeof(int));
    allocator->VRtoMemory = (int *)malloc(ir->count * sizeof(int));
    allocator->PRtoVR = (int *)malloc(k * sizeof(int));
    allocator->freePRs = (int *)malloc(k * sizeof(int));
    allocator->PRnext = (int *)malloc(k * sizeof(int));
    allocator->PRsUsed = (int *)malloc(k * sizeof(int));
    allocator->VRrem = (int *)malloc(ir->count * sizeof(int));

    allocator->freePRsCount = k;

    //allocator->finalIR.instructions = (IRLine *)malloc(ir->count * sizeof(IRLine));
    allocator->finalIR.instructions = emptyList();
    allocator->finalIR.count = 0;
    // allocator->finalIR.capacity = ir->count;

    for (int i = 0; i < ir->count; i++) {
        allocator->VRtoPR[i] = -1;
        allocator->VRtoMemory[i] = -1;
        allocator->VRrem[i] = -1;
    }
    for (int i = 1; i < k; i++) { // Start at 1 to skip PR0
        allocator->PRtoVR[i] = -1;       
        allocator->PRnext[i] = -1;       
        allocator->freePRs[i - 1] = i;   
        allocator->PRsUsed[i] = 0;
    }
    allocator->PRtoVR[0] = -1;           // Explicitly initialize PR0 mapping
    allocator->PRnext[0] = -1;           // Explicitly initialize PR0 next-use
    allocator->freePRsCount = k - 1;     // Adjust freePR count to exclude PR0

    //printf("Allocator initialized with %d physical registers.\n", k);
    // printAllocatorState(allocator, ir->count, k);
}

void computeLastUse(Allocator *allocator) {
    //printf("Starting last use computation...\n");
    //printList(allocator->ir->instructions);

    if (!allocator || !allocator->ir || !allocator->ir->instructions) {
        printf("Error: Allocator or IR is NULL\n");
        return;
    }
    List *current = allocator->ir->instructions->tail;
    int irCount = allocator->ir->count;
    int *lastUse = (int *)malloc(allocator->maxRegisters * sizeof(int));
    if (!lastUse) {
        printf("Error: Failed to allocate memory for lastUse array\n");
        exit(EXIT_FAILURE);
    }
        int *SRtoVR = (int *)malloc(allocator->maxRegisters * sizeof(int));
    if (!SRtoVR) {
        printf("Error: Failed to allocate memory for SRtoVR array\n");
        exit(EXIT_FAILURE);
    }
    // int SRtoVR[allocator->maxRegisters]; // Map SR to VR
    int currentVR = 0;         // Current virtual register index
    int lastStore = 0;
    // Initialize last use and SRtoVR
    for (int i = 0; i < allocator->maxRegisters; i++) {
        //lastUse[i] = irCount + 1;
        lastUse[i] = INT_MAX;
        SRtoVR[i] = -1;
    }
    allocator->live = 0;

    for (int i = irCount - 1; i >= 0 && current; i--) {
        IRLine *line = current->head;
        if (!line) {
            current = current->prev;
            continue;
        }
        if (line->opcode == STORE) {
            lastStore = i;
        }
        updateOperand(&line->dst, i, SRtoVR, lastUse, &currentVR, lastStore);
        if (line->dst.sr != -1) {
            SRtoVR[line->dst.sr] = -1;
            //lastUse[line->dst.sr] = irCount + 1;
            lastUse[line->dst.sr] = INT_MAX;
        }
        updateOperand(&line->src1, i, SRtoVR, lastUse, &currentVR, lastStore);
        updateOperand(&line->src2, i, SRtoVR, lastUse, &currentVR, lastStore);

        allocator->live = countAlive(allocator->maxRegisters, SRtoVR);
        current = current->prev;
    }
    // printf("CurrentVR: %d\n", currentVR);
}


int updateOperand(Operand *op, int idx, int *SRtoVR, int *lastUse, int *currentVR, int lastStore) {
    if (op->sr != -1) {
        if (SRtoVR[op->sr] == -1) {
            SRtoVR[op->sr] = (*currentVR)++;
        }
        op->vr = SRtoVR[op->sr];
        op->nu = lastUse[op->sr];  // Set next use
        lastUse[op->sr] = idx;     // Update last use
        op->dirty = (op->nu > lastStore) ? 1 : 0;
        return op->vr;
    }
    return -1;
}

int countAlive(int maxRegisters, int *SRtoVR) {
    int count = 0;
    for (int i = 0; i < maxRegisters; i++) {
        if (SRtoVR[i] != -1) {
            count++;
        }
    }
    return count;
}

int GetPR(Allocator *allocator, int vr) {
    debug(1,"Selecting PR for VR %d, FreePRs count: %d", vr, allocator->freePRsCount);

    // Case 1: Free physical registers available
    if (allocator->freePRsCount > 0) {
        int freePR = allocator->freePRs[--allocator->freePRsCount];
        allocator->PRtoVR[freePR] = vr;
        allocator->VRtoPR[vr] = freePR;
        allocator->PRnext[freePR] = INT_MAX; // Set initial next-use to infinity
        debug(1,"Using free PR: %d for VR: %d", freePR, vr);
        allocator->PRsUsed[freePR-1] = 1;
        return freePR;
    }

    // Case 2: Spill logic
    int bestScore = INT_MAX;
    int bestPR = -1;

    for (int pr = 1; pr < allocator->k; pr++) { // Skip PR0
        if (allocator->PRsUsed[pr-1]) continue;
        int currentVR = allocator->PRtoVR[pr];
        int cost = 0;

        if (allocator->VRrem[currentVR] != -1) {
            cost = 1; // Rematerializable
        } else if (allocator->VRtoMemory[currentVR] != -1) {
            cost = 3; // Respilled / Clean
        } else {
            cost = 6; // Dirty
        }

        int score = cost - (allocator->PRnext[pr] - allocator->currentInstructionIndex); // Weighted heuristic
        debug(1,"PR: %d Score: %d, cost = %d, PRnext: %d", pr, score, cost, allocator->PRnext[pr]);
        if (score < bestScore) {
            bestScore = score;
            bestPR = pr;
        }
    }

    if (bestPR == -1) {
        printf("Error: No PR available to spill\n");
        printAllocatorState(allocator, allocator->ir->count);
        exit(EXIT_FAILURE);
    }

    // Spill the selected register
    int spilledVR = allocator->PRtoVR[bestPR];
    spillRegister(allocator, spilledVR, bestPR);
    debug(1,"Adding PR%d assigned to VR%d", bestPR, vr);
    // Reassign the spilled PR to the new VR
    allocator->PRsUsed[bestPR-1] = 1;
    allocator->PRtoVR[bestPR] = vr;
    allocator->VRtoPR[vr] = bestPR;
    allocator->PRnext[bestPR] = INT_MAX; // Set initial next-use to infinity

    return bestPR;
}


void spillRegister(Allocator *allocator, int vr, int pr) {
    debug(1,"Spilling PR%d assigned to VR%d", pr, vr);
    //rem and spillopt
    if (allocator->VRrem[vr] != -1) {
        debug(1,"VR%d is rematerializable, skipping store. VRrem: %d", vr, allocator->VRrem[vr]);
        allocator->VRtoPR[vr] = -1; // Mark VR as no longer in a physical register
        allocator->PRtoVR[pr] = -1;
        allocator->PRnext[pr] = -1;
        // allocator->freePRs[allocator->freePRsCount++] = pr;
        return; // No need to generate store instructions
    }
    if (allocator->VRtoMemory[vr] != -1) {
        debug(1,"VR%d is a respill or is clean, skipping store", vr);

        allocator->VRtoPR[vr] = -1; // Mark VR as no longer in a physical register  
        allocator->PRtoVR[pr] = -1;
        allocator->PRnext[pr] = -1;
        //allocator->freePRs[allocator->freePRsCount++] = pr;
        return; // No need to generate store instructions
    }

    // if (allocator->lastLoaded[vr] >= allocator->lastStore) {
    //     debug(1,"VR%d is clean, skipping store", vr);
    //     allocator->VRtoPR[vr] = -1;
    //     allocator->PRtoVR[pr] = -1;
    //     allocator->PRnext[pr] = -1;
    //     return;
    // }

    if (allocator->VRtoMemory[vr] == -1) {
        allocator->VRtoMemory[vr] = allocator->nextSpillLocation;
        allocator->nextSpillLocation += 4;
    }
    int memoryLocation = allocator->VRtoMemory[vr];

    IRLine *loadi = (IRLine *)malloc(sizeof(IRLine));
    IRLine *store = (IRLine *)malloc(sizeof(IRLine));

    *loadi = (IRLine){.opcode = LOADI, .src1 = {.imm = memoryLocation}, .dst = {.pr = 0}};
    // *store = (IRLine){.opcode = STORE, .src1 = {.pr = 0}, .src2 = {.pr = pr}};
    *store = (IRLine){.opcode = STORE, .src1 = {.pr = pr}, .src2 = {.pr = 0}};

//    printf("New instructions from spill: \n");
//     prettyPrintInstruction(loadi);
//     prettyPrintInstruction(store);

    addToIR(&allocator->finalIR, *loadi);
    addToIR(&allocator->finalIR, *store);

    free(loadi);
    free(store);

    // allocator->VRspilled[vr] = 1;
    // allocator->lastStore[vr] = allocator->currentInstructionIndex;
    allocator->VRtoPR[vr] = -1;
    allocator->PRtoVR[pr] = -1;
    allocator->PRnext[pr] = -1;
    //allocator->freePRs[allocator->freePRsCount++] = pr;

    allocator->finalIR.count += 2;

    // printf("// Spilling VR%d from PR%d to memory location %d\n", vr, pr, memoryLocation);
    // printAllocatorState(allocator, allocator->ir->count, allocator->k);

}

void restoreRegister(Allocator *allocator, int vr, int pr) {
    if (allocator->VRrem[vr] != -1) {
        debug(1,"VR%d is rematerializable, emitting loadI instruction", vr);
        IRLine *loadi = (IRLine *)malloc(sizeof(IRLine));
        *loadi = (IRLine){.opcode = LOADI, .src1 = {.imm = allocator->VRrem[vr]}, .dst = {.pr = pr, .vr = vr}};
        addToIR(&allocator->finalIR, *loadi);

        // printf("New instructions from restore: \n");
        // prettyPrintInstruction(loadi);

        free(loadi);

        allocator->VRtoPR[vr] = pr; // Update mappings
        allocator->PRtoVR[pr] = vr;
        allocator->PRnext[pr] = INT_MAX-1;
        return;
    }


    // if (allocator->VRspilled[vr]) {
    //     debug(1,"Restoring respilled VR%d", vr);
    //     IRLine *load = (IRLine *)malloc(sizeof(IRLine));
    //     *load = (IRLine){.opcode = LOAD, .src1 = {.imm = allocator->VRtoMemory[vr]}, .dst = {.pr = pr, .vr = vr}};
    //     addToIR(&allocator->finalIR, *load);
    //     free(load);
    //     allocator->VRtoPR[vr] = pr;  // Update mappings
    //     allocator->PRtoVR[pr] = vr;
    //     return;
    // }

    // if (allocator->lastStore[vr] >= allocator->lastModified[vr]) {
    //     debug(1,"Restoring clean VR%d", vr);
    //     IRLine *load = (IRLine *)malloc(sizeof(IRLine));
    //     *load = (IRLine){.opcode = LOAD, .src1 = {.imm = allocator->VRtoMemory[vr]}, .dst = {.pr = pr, .vr = vr}};
    //     addToIR(&allocator->finalIR, *load);
    //     free(load);
    //     allocator->VRtoPR[vr] = pr;  // Update mappings
    //     allocator->PRtoVR[pr] = vr;
    //     return;
    // }

    // Get memory location assigned to VR
    int memoryLocation = allocator->VRtoMemory[vr];
    if (memoryLocation == -1) {
        printf("Error: No memory location assigned for VR%d\n", vr);
        exit(1);
    }
    
    // Create instructions to load the value back into the register
    IRLine *loadi = (IRLine *)malloc(sizeof(IRLine));
    IRLine *load = (IRLine *)malloc(sizeof(IRLine));

    *loadi = (IRLine){.opcode = LOADI, .src1 = {.imm = memoryLocation}, .dst = {.pr = 0}};
    *load = (IRLine){.opcode = LOAD, .src1 = {.pr = 0}, .dst = {.pr = pr, .vr = vr}};
    addToIR(&allocator->finalIR, *loadi);
    addToIR(&allocator->finalIR, *load);

    // printf("New instructions from restore: \n");
    // prettyPrintInstructionPRs(loadi);
    // prettyPrintInstructionPRs(load);

    free(loadi);
    free(load);
    allocator->finalIR.count += 2;
    

    // Update allocator state for the restored VR
    allocator->VRtoPR[vr] = pr;
    allocator->PRtoVR[pr] = vr;
    allocator->PRnext[pr] = INT_MAX-1;
    // allocator->finalIR.count += 2;

    // printf("// Restoring VR%d to PR%d from memory location %d with next use at %d\n",
    //        vr, pr, memoryLocation, nextUse);
}



void allocateRegisters(Allocator *allocator) {
    //printf("Allocating registers\n");
    //int irCount = allocator->ir->count;
    List *current = allocator->ir->instructions->next;
    int index = 0;

    while (current != NULL) {
        allocator->currentInstructionIndex = index;

        IRLine *line = current->head;

        int currentVRs[3] = {-1, -1, -1}; // src1, src2, dst VRs
        currentVRs[0] = line->src1.vr;
        currentVRs[1] = line->src2.vr;
        currentVRs[2] = line->dst.vr;
        for (int i = 0; i < allocator->k; i++) {
            allocator->PRsUsed[i] = 0;
        }
        for (int i = 0; i < 3; i++) {
        if (currentVRs[i] != -1) {
                int pr = allocator->VRtoPR[currentVRs[i]];
                if (pr != -1) {
                    allocator->PRsUsed[pr-1] = 1;
                }
            }
        }

        debug(1,"-----------------------------------------------------------------------------------------");
        debug(1,"Allocating registers for instruction %d: Opcode = %d", index, line->opcode);
        // printf("// ");
        // prettyPrintInstruction(line);
        // printf("// ");
        // prettyPrintInstructionVRs(line);
        // printAllocatorState(allocator, allocator->ir->count, allocator->k);

        processOperand(allocator, &line->src1);
        processOperand(allocator, &line->src2);    

        // Unassign registers for VRs no longer in use
        if (line->src1.vr != -1 && (line->src1.nu == INT_MAX)) {
            //printf("Freeing PR in allocateRegisters. Src1.nu: %d IRcount + 1: %d\n", line->src1.nu, irCount+1);
            freePR(allocator, line->src1.vr);
        }
        if (line->src2.vr != -1 && (line->src2.nu == INT_MAX)) {
            //printf("Freeing PR in allocateRegisters. Src2.nu: %d IRcount + 1: %d\n", line->src2.nu, irCount+1);
            freePR(allocator, line->src2.vr);
        }

        // add loadi to rem list
        if (line->opcode == LOADI) {
            allocator->VRtoMemory[line->dst.vr] = line->src1.imm; 
            allocator->VRrem[line->dst.vr] = line->src1.imm; 
        }
        if (line->opcode == LOAD) {      // Initialize as clean
            if (!line->dst.dirty) {
                // printf("Clean value");
                allocator->VRtoMemory[line->dst.vr] = allocator->VRrem[line->src1.vr];
            }
        }
 
        for (int i = 0; i < allocator->k; i++) {
            allocator->PRsUsed[i] = 0;
        }

        // Handle destination operand (dst)
        if (line->dst.vr != -1) {
            //printf("Processing Dst: VR = %d\n", line->dst.vr);
            int pr = GetPR(allocator, line->dst.vr);
            allocator->VRtoPR[line->dst.vr] = pr;
            allocator->PRtoVR[pr] = line->dst.vr;
            allocator->PRnext[pr] = line->dst.nu;
            //printf("Assigned PR%d to VR%d with Next Use: %d\n", pr, line->dst.vr, allocator->PRnext[pr]);
            line->dst.pr = pr;
        }



        // if (line->dst.vr != -1 && (line->dst.nu == INT_MAX)) {
        //     freePR(allocator, line->dst.vr);
        // }
        //printf("Completed instruction %d\n", index);
        //printf("PRnext after processing instruction %d:\n", index);
        // for (int pr = 0; pr < allocator->k; pr++) {
        //     printf("  PR%d -> Next Use: %d\n", pr, allocator->PRnext[pr]);
        // }
        
        // printf("// ");
        // prettyPrintInstructionPRs(line);
        addToIR(&allocator->finalIR, *line);
        // append(allocator->finalIR.instructions, line);
        // allocator->finalIR.count++;

        current = current->next;
        index++;
        // printf("After allocating registers for instruction %d:\n", index);
        // printAllocatorState(allocator, allocator->ir->count, allocator->k);

    }
    //printf("Finished Allocating registers\n");
}

void processOperand(Allocator *allocator, Operand *op) {
    if (op->vr != -1) {
        if (allocator->VRtoPR[op->vr] == -1) {
            int pr = GetPR(allocator, op->vr);
            if (allocator->VRtoMemory[op->vr] != -1) {
                restoreRegister(allocator, op->vr, pr);
            }
            allocator->VRtoPR[op->vr] = pr;
            allocator->PRtoVR[pr] = op->vr;
        }
        allocator->PRnext[allocator->VRtoPR[op->vr]] = op->nu;
        op->pr = allocator->VRtoPR[op->vr];
    }
}


void freePR(Allocator *allocator, int vr) {
    //printf("Freeing PR for VR%d\n", vr);
    debug(1, "Freeing PR for VR%d", vr);
    int pr = allocator->VRtoPR[vr];
    if (pr != -1) {
        allocator->freePRs[allocator->freePRsCount++] = pr;
        allocator->VRtoPR[vr] = -1;
        allocator->PRtoVR[pr] = -1;
        allocator->PRnext[pr] = -1;
        //printf("Freed PR%d assigned to VR%d\n", pr, vr);
    }
}

void printAllocatedIR(Allocator *allocator) {
    //printf("Printing %d allocated instructions.\n", allocator->finalIR.count);

    List *current = allocator->finalIR.instructions->next;  // Skip sentinel node

    while (current != NULL) {
        IRLine *line = current->head;

        switch (line->opcode) {
            case LOADI:
                printf("loadI %d => r%d\n", line->src1.imm, line->dst.pr);
                break;
            case LOAD:
                printf("load r%d => r%d\n", line->src1.pr, line->dst.pr);
                break;
            case STORE:
                printf("store r%d => r%d\n", line->src1.pr, line->src2.pr);
                break;
            case ADD:
                printf("add r%d, r%d => r%d\n", line->src1.pr, line->src2.pr, line->dst.pr);
                break;
            case SUB:
                printf("sub r%d, r%d => r%d\n", line->src1.pr, line->src2.pr, line->dst.pr);
                break;
            case RSHIFT:
                printf("rshift r%d, r%d => r%d\n", line->src1.pr, line->src2.pr, line->dst.pr);
                break;
            case LSHIFT:
                printf("lshift r%d, r%d => r%d\n", line->src1.pr, line->src2.pr, line->dst.pr);
                break;
            case MULT:
                printf("mult r%d, r%d => r%d\n", line->src1.pr, line->src2.pr, line->dst.pr);
                break;
            case OUTPUT:
                printf("output %d\n", line->src1.imm);
                break;
            case NOP:
                printf("nop");
                break;
            default:
                printf("// Unknown instruction\n");
        }
        current = current->next;
    }
}

void printAllocatorState(Allocator *allocator, int vrCount) {
    printf("VRtoPR| ");
    for (int i = 0; i < vrCount; i++) {
        if (allocator->VRtoPR[i] != -1) {
            printf("%d : %d | ", i, allocator->VRtoPR[i]);
        }
    }
    // printf("\nPRtoVR| ");
    // for (int i = 0; i < k; i++) {
    //     if (allocator->PRtoVR[i] != -1) {
    //         printf("%d : %d | ", i, allocator->PRtoVR[i]);
    //     }
    // }
    // printf("\nPRNext| ");
    // for (int i = 0; i < k; i++) {
    //     if (allocator->PRnext[i] != -1) {
    //         printf("%d : %d | ", i, allocator->PRnext[i]);
    //     }
    // }
    // printf("\nfreePr| ");
    // for (int i = 0; i < allocator->freePRsCount; i++) {
    //     printf("%d : %d | ", i, allocator->freePRs[i]);
    // }
    
    printf("\nVRtoMemory| ");
    for (int i = 0; i < vrCount; i++) {
        if (allocator->VRtoMemory[i] != -1) {
            printf("%d : %d | ", i, allocator->VRtoMemory[i]);
        }
    }

    // printf("\nVRrem| ");
    // for (int i = 0; i < vrCount; i++) {
    //     if (allocator->VRrem[i] != -1) {
    //         printf("%d : %d | ", i, allocator->VRrem[i]);
    //     }
    // }

    // printf("\nPRsUsed| ");
    // for (int i = 0; i < k; i++) {
    //     if (allocator->PRsUsed[i] != -1) {
    //         printf("%d : %d | ", i+1, allocator->PRsUsed[i]);
    //     }
    // }

    // printf("\nlastStore| %d", allocator->lastStore);

    // printf("\nlastLoaded| ");
    // for (int i = 0; i < vrCount; i++) {
    //     if (allocator->lastLoaded[i] != -1) {
    //         printf("%d : %d | ", i, allocator->lastLoaded[i]);
    //     }
    // }
    printf("\n");
}


