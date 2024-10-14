#ifndef IR_H
#define IR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcodes.h"

// Structure for a register operand or immediate
typedef struct {
    int sr;   // Source register number
    int imm;  // Immediate value (for LOADI)
} Operand;

typedef struct {
    int opcode;        // Instruction opcode (LOADI, ADD, etc.)
    Operand src1;      // First source operand (register or immediate)
    Operand src2;      // Second source operand (if applicable)
    Operand dst;       // Destination register
} IRLine;

typedef struct {
    IRLine *instructions;
    int count;
    int capacity;
} IR;

typedef enum {
    PRETTY_PRINT,
    TABLE_PRINT
} PrintMode;

void initIR(IR *ir);
void addToIR(IR *ir, IRLine line);
void printIR(IR *ir, PrintMode mode);
void prettyPrintInstruction(IRLine *line);
void printInstructionTable(IRLine *line);
const char* opcodeToString(OpcodeType opcode);

#endif
