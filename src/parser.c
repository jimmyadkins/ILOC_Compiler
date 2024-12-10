#include "parser.h"
#include "utils.h"
#include <stdio.h>

void initParser(Parser *parser, Lexer *lexer, IR *ir) {
    parser->lexer = lexer;
    parser->ir = ir;
}

void parseProgram(Parser *parser) {
    Token token;
    while ((token = getNextToken(parser->lexer)).cat != EOF_TOKEN) {
        if (token.cat == INSTRUCTION) {
            IRLine line = parseInstruction(parser, token.val);
            addToIR(parser->ir, line);
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
            line.opcode = opcode;
            return line;
    }
}

IRLine parseLoadI(Parser *parser) {
    IRLine line;
    initIRLine(&line);
    line.opcode = LOADI;
    Token tok = getNextToken(parser->lexer);

    if (tok.cat == CONSTANT) {
        line.src1.imm = tok.val;
        // printf("Loaded constant: %d into src1\n", line.src1.imm);
    } else {
        printf("Expected CONSTANT, but got token category: %d\n", tok.cat);
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == ARROW) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            line.dst.sr = tok.val;
            // printf("Loaded destination register: r%d\n", line.dst.sr);
        } else {
            printf("Expected REGISTER, but got token category: %d\n", tok.cat);
        }
    }
    // printf("Parsed LOADI: imm=%d, dst=%d\n", line.src1.imm, line.dst.sr);
    return line;
}

IRLine parseArithOp(Parser *parser, OpcodeType opcode) {
    IRLine line;
    initIRLine(&line);
    line.opcode = opcode;

    Token tok = getNextToken(parser->lexer);
    if (tok.cat == REGISTER) {
        line.src1.sr = tok.val;
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == COMMA) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            line.src2.sr = tok.val;
        }
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == ARROW) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            line.dst.sr = tok.val;
        }
    }
    // printf("Parsed arithop: src1=%d, src2=%d, dst=%d\n", line.src1.sr, line.src2.sr, line.dst.sr);
    return line;
}

IRLine parseMemOp(Parser *parser, OpcodeType opcode) {
    IRLine line;
    initIRLine(&line);
    line.opcode = opcode;

    Token tok = getNextToken(parser->lexer);
    assertCondition(tok.cat == REGISTER, "Non register type found for src1 in memop");
    if (opcode == STORE) {
        line.src1.sr = tok.val;
    } else {
        line.src1.sr = tok.val;
    }

    // tok = getNextToken(parser->lexer);
    assertCondition(getNextToken(parser->lexer).cat == ARROW, "Non arrow type found after src1 in memop");
    tok = getNextToken(parser->lexer);
    assertCondition(tok.cat == REGISTER, "Non register type found for src2 in memop");

    if (opcode == STORE) {
        line.src2.sr = tok.val;  
    } else {
        line.dst.sr = tok.val;
    }
    // printf("Parsed memop: src1=%d, src2=%d\n", line.src1.sr, line.src2.sr);
    return line;
}

IRLine parseShiftOp(Parser *parser, OpcodeType opcode) {
    IRLine line;
    initIRLine(&line);
    line.opcode = opcode;

    Token tok = getNextToken(parser->lexer);
    if (tok.cat == REGISTER) {
        line.src1.sr = tok.val;
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == COMMA) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            line.src2.sr = tok.val;
        }
    }

    tok = getNextToken(parser->lexer);
    if (tok.cat == ARROW) {
        tok = getNextToken(parser->lexer);
        if (tok.cat == REGISTER) {
            line.dst.sr = tok.val;
        }
    }
    return line;
}

IRLine parseOutput(Parser *parser) {
    IRLine line;
    initIRLine(&line);
    line.opcode = OUTPUT;

    Token tok = getNextToken(parser->lexer);
    if (tok.cat == CONSTANT) {
        line.src1.imm = tok.val;
    }
    // printf("Parsed OUTPUT: imm=%d\n", line.src1.imm);
    return line;
}

IRLine parseNop() {
    IRLine line;
    line.opcode = NOP;
    return line;
}
