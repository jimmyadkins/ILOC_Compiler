#include "parser.h"
#include <stdio.h>

// Initialize the parser
void initParser(Parser *parser, Lexer *lexer, IR *ir) {
    parser->lexer = lexer;
    parser->ir = ir;
}

// processes the entire input file and populates the IR
void parseProgram(Parser *parser) {
    Token token;
    while ((token = getNextToken(parser->lexer)).cat != EOF_TOKEN) {
        // printToken(token);  // Print every token as it is retrieved
        if (token.cat == INSTRUCTION) {
            IRLine line = parseInstruction(parser, token.val);
            addToIR(parser->ir, line);  // Add the parsed instruction to the IR
        }
    }
}

IRLine parseInstruction(Parser *parser, OpcodeType opcode) {
    switch (opcode) {
        case LOADI:
            return parseLoadI(parser);
        case LOAD:
        case STORE:
            return parseMemOp(parser, opcode);
        case ADD:
        case SUB:
        case MULT:
            return parseArithOp(parser, opcode);
        case RSHIFT:
        case LSHIFT:
            return parseShiftOp(parser, opcode);
        case OUTPUT:
            return parseOutput(parser);
        case NOP:
            return parseNop(parser); 
        default:
            printf("Unknown instruction\n");
            IRLine line;
            line.opcode = opcode;  // Store the opcode even if unknown
            return line;
    }
}

IRLine parseNop() {
    IRLine line;
    line.opcode = NOP;
    return line;
}


// Parse shift instructions like RSHIFT and LSHIFT
IRLine parseShiftOp(Parser *parser, OpcodeType opcode) {
    IRLine line;
    line.opcode = opcode;

    Token tok = getNextToken(parser->lexer);
    if (tok.cat == REGISTER) {
        line.src1.sr = tok.val;  // First source register
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == COMMA) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            line.src2.sr = tok.val;  // Second source register
        }
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == ARROW) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            line.dst.sr = tok.val;  // Destination register
        }
    }

    return line;
}


// Parse LOADI instruction
IRLine parseLoadI(Parser *parser) {
    IRLine line;
    line.opcode = LOADI;

    Token tok = getNextToken(parser->lexer);
    if (tok.cat == CONSTANT) {
        line.src1.imm = tok.val;  // Immediate value
    } else {
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == ARROW) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            line.dst.sr = tok.val;  // Destination register
        } else {
            printf("Expected REGISTER, but got token category: %d\n", tok.cat);
        }
    } else {
        printf("Expected ARROW, but got token category: %d\n", tok.cat);
    }
    return line;
}



IRLine parseMemOp(Parser *parser, OpcodeType opcode) {
    IRLine line;
    line.opcode = opcode;

    Token tok = getNextToken(parser->lexer);
    if (tok.cat == REGISTER) {
        line.src1.sr = tok.val;  // First register (source or address)
    } else {
        printf("Expected REGISTER, but got token category: %d\n", tok.cat);
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == ARROW) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            if (opcode == STORE) {
                line.src2.sr = tok.val;  // For store, second operand is also a source
            } else {
                line.dst.sr = tok.val;   // For load, it's the destination
            }
        } else {
            printf("Expected REGISTER, but got token category: %d\n", tok.cat);
        }
    } else {
        printf("Expected ARROW, but got token category: %d\n", tok.cat);
    }
    return line;
}



// Parse arithmetic operations like ADD, SUB, MULT
IRLine parseArithOp(Parser *parser, OpcodeType opcode) {
    IRLine line;
    line.opcode = opcode;

    Token tok = getNextToken(parser->lexer);
    if (tok.cat == REGISTER) {
        line.src1.sr = tok.val;  // First source register
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == COMMA) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            line.src2.sr = tok.val;  // Second source register
        }
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == ARROW) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            line.dst.sr = tok.val;   // Destination register
        }
    }
    return line;
}

// Parse OUTPUT instruction
IRLine parseOutput(Parser *parser) {
    IRLine line;
    line.opcode = OUTPUT;

    Token tok = getNextToken(parser->lexer);
    if (tok.cat == CONSTANT) {
        line.src1.imm = tok.val;  // Immediate value for output
    }
    return line;
}
