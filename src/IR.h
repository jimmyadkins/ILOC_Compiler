#ifndef IR_H
#define IR_H
#include <limits.h>
#include "list.h"
#include "opcodes.h"

// Operand structure
typedef struct Operand {
    int sr;         // Source register number
    int vr;         // Virtual register number
    int pr;         // Physical register number
    int imm;        // Immediate value (if any)
    int nu;         // Next use
    int spilled;    // Flag indicating if this operand is spilled to memory
    int dirty;      // indicates if this operand's next use is dirty (has a store between loads)
} Operand;

// IRLine structure
typedef struct IRLine {
    int opcode;        // Instruction opcode (LOADI, ADD, etc.)
    Operand src1;      // First source operand (register or immediate)
    Operand src2;      // Second source operand (if applicable)
    Operand dst;       // Destination register
    int index;         // Original instruction index (for debugging)
} IRLine;

// IR structure
typedef struct IR {
    List *instructions;  // List of IRLine instructions
    int count;           // Number of instructions
} IR;

// Function declarations
typedef enum {
    PRETTY_PRINT,
    TABLE_PRINT
} PrintMode;

void initIR(IR *ir);
void initIRLine(IRLine *line);
void addToIR(IR *ir, IRLine line);
void printIR(IR *ir, PrintMode mode);
void freeIR(IR *ir);
int getMaxSR(List *instructions);

void prettyPrintInstruction(IRLine *line);
void prettyPrintInstructionPRs(IRLine *line);
void prettyPrintInstructionVRs(IRLine *line);
void printInstructionTable(IRLine *line);
const char* opcodeToString(OpcodeType opcode);

#endif
