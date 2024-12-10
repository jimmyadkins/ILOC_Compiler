#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "ir.h"  // Assuming this file defines the IR and IRLine structures

// #define MAX_REGISTERS 1999999  // Adjust as needed

// Allocator structure
typedef struct Allocator {
    IR *ir;
    IR finalIR; // New list for the final IR
    int maxRegisters;
    int *VRtoPR;
    int *VRtoMemory;
    int *PRtoVR;
    int *PRnext;
    int *freePRs;
    int *PRsUsed;
    int *VRrem;
    int *VRspilled;
    int *lastStore;
    int *lastModified;
    int freePRsCount;
    int nextSpillLocation;
    int k;
    int live;
    int currentInstructionIndex;
} Allocator;


// Allocator function declarations

/**
 * Initializes the allocator with the IR and the number of physical registers.
 */
void initAllocator(Allocator *allocator, IR *ir, int k);

/**
 * Computes the last use of each operand and populates the next use table.
 */
void computeLastUse(Allocator *allocator);

/**
 * Updates the last use and assigns virtual registers for an operand.
 * Used during the last-use computation phase.
 */
int updateOperand(Operand *op, int idx, int *SRtoVR, int *lastUse, int *currentVR);

/**
 * Allocates physical registers for each instruction in the IR.
 * Includes spilling and restoring of virtual registers as needed.
 */
void allocateRegisters(Allocator *allocator);

/**
 * Retrieves a physical register for a virtual register, performing spilling if necessary.
 */
int GetPR(Allocator *allocator, int vr);

/**
 * Spills a virtual register from a physical register to memory.
 */
void spillRegister(Allocator *allocator, int vr, int pr);

/**
 * Restores a virtual register from memory to a physical register.
 */
void restoreRegister(Allocator *allocator, int vr, int pr);

/**
 * Frees a physical register assigned to a virtual register.
 */
void freePR(Allocator *allocator, int vr);

/**
 * Prints the IR with allocated registers for debugging or verification.
 */
void printAllocatedIR(Allocator *allocator);

void processOperand(Allocator *allocator, Operand *op);
int countAlive(int maxRegisters, int *SRtoVR);
// int isCleanValue(Allocator *allocator, int vr, int currentInstruction);

void printAllocatorState(Allocator *allocator, int vrCount, int k);

#endif // ALLOCATOR_H
