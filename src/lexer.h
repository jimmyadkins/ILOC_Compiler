#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "opcodes.h"  

typedef enum {
    INSTRUCTION, REGISTER, CONSTANT, COMMA, ARROW, INVALID, EOF_TOKEN
} TokenType;

typedef struct {
    TokenType cat;   
    OpcodeType val;  
} Token;

typedef struct {
    FILE *input;    
    int currentChar; 
} Lexer;


void initLexer(Lexer *lexer, FILE *input);
Token getNextToken(Lexer *lexer);
void printToken(Token token);

Token scanS(Lexer *lexer);
Token scanStore(Lexer *lexer);
Token scanSub(Lexer *lexer);
Token scanL(Lexer *lexer);
Token scanLoad(Lexer *lexer);
Token scanShift(Lexer *lexer, OpcodeType shiftType);
Token scanR(Lexer *lexer);
Token scanMult(Lexer *lexer);
Token scanAdd(Lexer *lexer);
Token scanNop(Lexer *lexer);
Token scanOutput(Lexer *lexer);

#endif
