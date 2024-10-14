#include "ir.h"
#include <stdlib.h>
#include <stdio.h>

void initIR(IR *ir) {
    ir->count = 0;
    ir->capacity = 10; 
    ir->instructions = (IRLine *)malloc(sizeof(IRLine) * ir->capacity);
    if (!ir->instructions) {
        fprintf(stderr, "Error: Memory allocation failed for IR instructions.\n");
        exit(EXIT_FAILURE);
    }
}

void addToIR(IR *ir, IRLine line) {
    if (ir->count >= ir->capacity) {
        ir->capacity *= 2;  // Double the capacity
        ir->instructions = (IRLine *)realloc(ir->instructions, sizeof(IRLine) * ir->capacity);
        if (!ir->instructions) {
            exit(EXIT_FAILURE);
        }
    }
    ir->instructions[ir->count++] = line;
}

void printIR(IR *ir, PrintMode mode) {
    for (int i = 0; i < ir->count; i++) {
        IRLine *line = &ir->instructions[i];
        switch (mode) {
            case PRETTY_PRINT:
                prettyPrintInstruction(line);
                break;
            case TABLE_PRINT:
                printInstructionTable(line);
                break;
        }
    }
}

void prettyPrintInstruction(IRLine *line) {
    switch (line->opcode) {
        case LOADI:
            printf("loadI %d => r%d\n", line->src1.imm, line->dst.sr);
            break;
        case LOAD:
            printf("load r%d => r%d\n", line->src1.sr, line->dst.sr);
            break;
        case STORE:
            printf("store r%d => r%d\n", line->src1.sr, line->src2.sr);
            break;
        case ADD:
            printf("add r%d, r%d => r%d\n", line->src1.sr, line->src2.sr, line->dst.sr);
            break;
        case SUB:
            printf("sub r%d, r%d => r%d\n", line->src1.sr, line->src2.sr, line->dst.sr);
            break;
        case MULT:
            printf("mult r%d, r%d => r%d\n", line->src1.sr, line->src2.sr, line->dst.sr);
            break;
        case OUTPUT:
            printf("output %d\n", line->src1.imm);
            break;
        case NOP:
            printf("nop\n");
            break;
        default:
            printf("Unknown instruction\n");
    }
}

void printInstructionTable(IRLine *line) {
    switch (line->opcode) {
        case LOADI:
            printf("| %-6s | %-3d | -    | => | r%-3d |\n",  // Use immediate value for LOADI
                   opcodeToString(line->opcode),
                   line->src1.imm,
                   line->dst.sr);
            break;
        case STORE:
            printf("| %-6s | r%-3d | -    | => | r%-3d |\n",  // Store shows two sources
                   opcodeToString(line->opcode),
                   line->src1.sr,
                   line->src2.sr);
            break;
        case OUTPUT:
            printf("| %-6s | %-5d | -    | => | -    |\n",
               opcodeToString(line->opcode),
               line->src1.imm);  // Output only prints the immediate value
            break;
        case NOP:
            printf("| %-6s | -    | -    | => | -    |\n", opcodeToString(line->opcode));
            break;
        default:
            printf("| %-6s | r%-3d | r%-3d | => | r%-3d |\n",  // Default register-based instructions
                   opcodeToString(line->opcode),
                   line->src1.sr,
                   line->src2.sr,
                   line->dst.sr);
            break;
    }
}


const char* opcodeToString(OpcodeType opcode) {
    switch (opcode) {
        case LOADI: return "loadI";
        case LOAD:  return "load";
        case STORE: return "store";
        case ADD:   return "add";
        case SUB:   return "sub";
        case MULT:  return "mult";
        case LSHIFT: return "lshift"; 
        case RSHIFT: return "rshift";   
        case OUTPUT: return "output";
        case NOP:   return "nop";
        default:    return "unknown";
    }
}
