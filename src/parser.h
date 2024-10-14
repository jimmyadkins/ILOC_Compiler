#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ir.h"

typedef struct {
    Lexer *lexer;
    IR *ir;
} Parser;


void initParser(Parser *parser, Lexer *lexer, IR *ir);
void parseProgram(Parser *parser);

IRLine parseInstruction(Parser *parser, OpcodeType opcode);
IRLine parseLoadI(Parser *parser);
IRLine parseMemOp(Parser *parser, OpcodeType opcode);
IRLine parseArithOp(Parser *parser, OpcodeType opcode);
IRLine parseShiftOp(Parser *parser, OpcodeType opcode);
IRLine parseOutput(Parser *parser);
IRLine parseNop();

#endif
