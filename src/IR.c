#include "ir.h"
#include "list.h"
#include "utils.h"


#include <stdlib.h>
#include <stdio.h>

// void initIR(IR *ir) {
//     ir->count = 0;
//     ir->capacity = 10;
//     ir->instructions = (IRLine *)malloc(sizeof(IRLine) * ir->capacity);
//     if (!ir->instructions) {
//         fprintf(stderr, "Error: Memory allocation failed for IR instructions.\n");
//         exit(EXIT_FAILURE);
//     }
// }

void initIR(IR *ir) {
    ir->instructions = emptyList();  // Initialize as an empty list
    ir->count = 0;
    //printf("IR initialized.\n");
}

void initIRLine(IRLine *line) {
    line->opcode = -1; // Invalid opcode as a default
    line->src1.sr = -1;
    line->src1.vr = -1;
    line->src1.nu = INT_MAX; // Represents "no next use"
    line->src1.imm = 0;

    line->src2.sr = -1;
    line->src2.vr = -1;
    line->src2.nu = INT_MAX;
    line->src2.imm = 0;

    line->dst.sr = -1;
    line->dst.vr = -1;
    line->dst.nu = INT_MAX;
}


// void addToIR(IR *ir, IRLine line) {
//     // Ensure IRLine fields are initialized to avoid carryover issues
//     line.src1.sr = line.src1.sr != -1 ? line.src1.sr : -1;
//     line.src2.sr = line.src2.sr != -1 ? line.src2.sr : -1;
//     line.dst.sr = line.dst.sr != -1 ? line.dst.sr : -1;

//     if (ir->count >= ir->capacity) {
//         printf("Increasing IR capacity...\n");
//         ir->capacity *= 2;
//         ir->instructions = (IRLine *)realloc(ir->instructions, sizeof(IRLine) * ir->capacity);
//         if (!ir->instructions) {
//             printf("Error: Failed to reallocate memory for IR instructions\n");
//             exit(EXIT_FAILURE);
//         }
//     }

//     // Debug statement for opcode and operands being added
//     printf("Adding to IR: Opcode = %s, Src1 = r%d, Src2 = r%d, Dst = r%d\n", 
//            opcodeToString(line.opcode), 
//            line.src1.sr, 
//            line.src2.sr, 
//            line.dst.sr);

//     ir->instructions[ir->count++] = line;
// }

void addToIR(IR *ir, IRLine line) {
    if (!ir || !ir->instructions) {
        printf("Error: IR or instructions list is NULL.\n");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the new instruction
    IRLine *newLine = (IRLine *)malloc(sizeof(IRLine));
    if (!newLine) {
        printf("Error: Memory allocation failed for new instruction.\n");
        exit(EXIT_FAILURE);
    }
    *newLine = line;

    // Use the append function to add the new instruction to the list
    append(ir->instructions, newLine);

    // Increment the IR count
    ir->count++;

    // Debug: Print the list size after addition
    //printf("List size after addToIR: %d\n", size(ir->instructions));
}


int getMaxSR(List *instructions) {
    if (!instructions) {
        printf("Error: Instructions list is NULL\n");
        return -1;
    }

    int maxSR = 0;  
    List *current = instructions->next;  

    while (current) {
        IRLine *line = current->head;
        if (line) {
            if (line->src1.sr != -1) {
                maxSR = (line->src1.sr > maxSR) ? line->src1.sr : maxSR;
            }
            if (line->src2.sr != -1) {
                maxSR = (line->src2.sr > maxSR) ? line->src2.sr : maxSR;
            }
            if (line->dst.sr != -1) {
                maxSR = (line->dst.sr > maxSR) ? line->dst.sr : maxSR;
            }
        }
        current = current->next;
    }

    return maxSR+1;
}

// void printIR(IR *ir, PrintMode mode) {
//     for (int i = 0; i < ir->count; i++) {
//         IRLine *line = &ir->instructions[i];
//         switch (mode) {
//             case PRETTY_PRINT:
//                 prettyPrintInstruction(line);
//                 break;
//             case TABLE_PRINT:
//                 printInstructionTable(line);
//                 break;
//         }
//     }
// }

void printIR(IR *ir, PrintMode mode) {
    List *current = ir->instructions->next;  // Use List consistently

    while (current != NULL) {
        IRLine *line = current->head;
        if (line) {
            switch (mode) {
                case PRETTY_PRINT:
                    prettyPrintInstruction(line);
                    break;
                case TABLE_PRINT:
                    printInstructionTable(line);
                    break;
            }
        }
        current = current->next;
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
        case LSHIFT:
            printf("lshift r%d, r%d => r%d\n", line->src1.sr, line->src2.sr, line->dst.sr);
            break;
        case RSHIFT:
            printf("lshift r%d, r%d => r%d\n", line->src1.sr, line->src2.sr, line->dst.sr);
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

void prettyPrintInstructionPRs(IRLine *line) {
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
        case MULT:
            printf("mult r%d, r%d => r%d\n", line->src1.pr, line->src2.pr, line->dst.pr);
            break;
        case LSHIFT:
            printf("lshift r%d, r%d => r%d\n", line->src1.pr, line->src2.pr, line->dst.pr);
            break;
        case RSHIFT:
            printf("lshift r%d, r%d => r%d\n", line->src1.pr, line->src2.pr, line->dst.pr);
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

void prettyPrintInstructionVRs(IRLine *line) {
    switch (line->opcode) {
        case LOADI:
            printf("loadI %d => r%d\n", line->src1.imm, line->dst.vr);
            break;
        case LOAD:
            printf("load r%d => r%d\n", line->src1.vr, line->dst.vr);
            break;
        case STORE:
            printf("store r%d => r%d\n", line->src1.vr, line->src2.vr);
            break;
        case ADD:
            printf("add r%d, r%d => r%d\n", line->src1.vr, line->src2.vr, line->dst.vr);
            break;
        case SUB:
            printf("sub r%d, r%d => r%d\n", line->src1.vr, line->src2.vr, line->dst.vr);
            break;
        case MULT:
            printf("mult r%d, r%d => r%d\n", line->src1.vr, line->src2.vr, line->dst.vr);
            break;
        case LSHIFT:
            printf("lshift r%d, r%d => r%d\n", line->src1.vr, line->src2.vr, line->dst.vr);
            break;
        case RSHIFT:
            printf("lshift r%d, r%d => r%d\n", line->src1.vr, line->src2.vr, line->dst.vr);
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
            printf("| %-6s | %-4d | -    | => | r%-3d |\n",
                   opcodeToString(line->opcode),
                   line->src1.imm,
                   line->dst.sr);  
            break;
        case STORE:
            printf("| %-6s | r%-3d | -    | => | r%-3d |\n",
                   opcodeToString(line->opcode),
                   line->src1.sr,
                   line->src2.sr);
            break;
        case OUTPUT:
            printf("| %-6s | %-4d | -    | => | -    |\n",
                   opcodeToString(line->opcode),
                   line->src1.imm);
            break;
        case NOP:
            printf("| %-6s | -    | -    | => | -    |\n", opcodeToString(line->opcode));
            break;
        default:
            printf("| %-6s | r%-3d | r%-3d | => | r%-3d |\n",
                   opcodeToString(line->opcode),
                   line->src1.sr,
                   line->src2.sr,
                   line->dst.sr);
            break;
    }
}

void freeIR(IR *ir) {
    freeList(ir->instructions);  // Free the linked list of instructions
    ir->count = 0;               // Reset the count
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
