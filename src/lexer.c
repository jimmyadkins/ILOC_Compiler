#include "lexer.h"
#include "utils.h"
#include <ctype.h>
#include <stdlib.h>

void initLexer(Lexer *lexer, FILE *input) {
    lexer->input = input;
    lexer->currentChar = fgetc(input); 
}

char advance(Lexer *lexer) {
    lexer->currentChar = fgetc(lexer->input);
    return lexer->currentChar;
}

void skipComments(Lexer *lexer) {
    if (lexer->currentChar == '/') {
        skip_comments(lexer->input); 
        lexer->currentChar = fgetc(lexer->input);  
    }
}

void skipWhitespaceAndComments(Lexer *lexer) {
    while (isspace(lexer->currentChar) || lexer->currentChar == '/') {
        if (lexer->currentChar == '/') {
            skipComments(lexer); 
        } else {
            lexer->currentChar = fgetc(lexer->input); 
        }
    }
}

int readNumberFromLexer(Lexer *lexer) {
    int number = 0;
    while (isdigit(lexer->currentChar)) {
        number = number * 10 + (lexer->currentChar - '0');
        lexer->currentChar = fgetc(lexer->input); 
    }
    return number;
}

Token scanS(Lexer *lexer) {
    advance(lexer);
    if (lexer->currentChar == 't') return scanStore(lexer);
    if (lexer->currentChar == 'u') return scanSub(lexer);
    return (Token){INVALID, NOP};  
}

Token scanL(Lexer *lexer) {
    advance(lexer);
    if (lexer->currentChar == 'o') return scanLoad(lexer);
    if (lexer->currentChar == 's') return scanShift(lexer, LSHIFT);
    return (Token){INVALID, NOP};  
}

Token scanR(Lexer *lexer) {
    advance(lexer);
    if (isdigit((unsigned char)lexer->currentChar)) {
        int regNum = lexer->currentChar - '0';
        while (isdigit((unsigned char)advance(lexer))) {
            regNum = regNum * 10 + (lexer->currentChar - '0');
        }
        return (Token){REGISTER, regNum};  
    }
    if (lexer->currentChar == 's') return scanShift(lexer, RSHIFT);
    return (Token){INVALID, NOP};  
}

Token scanStore(Lexer *lexer) {
    if (advance(lexer) == 'o') {
        if (advance(lexer) == 'r') {
            if (advance(lexer) == 'e') {
                advance(lexer); 
                return (Token){INSTRUCTION, STORE};  
            }
        }
    }
    return (Token){INVALID, NOP};  
}

Token scanSub(Lexer *lexer) {
    if (advance(lexer) == 'b') {
        advance(lexer);  
        return (Token){INSTRUCTION, SUB};  
    }
    return (Token){INVALID, NOP};  
}

Token scanLoad(Lexer *lexer) {
    if (advance(lexer) == 'a') {
        if (advance(lexer) == 'd') {
            if (advance(lexer) == 'I') {
                advance(lexer);  
                return (Token){INSTRUCTION, LOADI};  
            }
            advance(lexer);  
            return (Token){INSTRUCTION, LOAD};  
        }
    }
    return (Token){INVALID, NOP};  
}

Token scanShift(Lexer *lexer, OpcodeType shiftType) {
    if (advance(lexer) == 'h' && advance(lexer) == 'i' && advance(lexer) == 'f' && advance(lexer) == 't') {
        advance(lexer);  
        return (Token){INSTRUCTION, shiftType};  
    }
    return (Token){INVALID, NOP};  
}

Token scanOutput(Lexer *lexer) {
    if (advance(lexer) == 'u') {
        if (advance(lexer) == 't') {
            if (advance(lexer) == 'p') {
                if (advance(lexer) == 'u') {
                    if (advance(lexer) == 't') {
                        advance(lexer);  
                        return (Token){INSTRUCTION, OUTPUT};  
                    }
                }
            }
        }
    }
    return (Token){INVALID, NOP};  
}

Token scanNop(Lexer *lexer) {
    if (advance(lexer) == 'o' && advance(lexer) == 'p') {
        advance(lexer); 
        return (Token){INSTRUCTION, NOP};  
    }
    return (Token){INVALID, NOP}; 
}

Token scanMult(Lexer *lexer) {
    if (advance(lexer) == 'u') {
        if (advance(lexer) == 'l') {
            if (advance(lexer) == 't') {
                advance(lexer);  
                return (Token){INSTRUCTION, MULT};  
            }
        }
    }
    return (Token){INVALID, NOP};  
}

Token scanAdd(Lexer *lexer) {
    if (advance(lexer) == 'd') {
        if (advance(lexer) == 'd') {
            advance(lexer);  
            return (Token){INSTRUCTION, ADD}; 
        }
    }
    return (Token){INVALID, NOP};  
}

Token getNextToken(Lexer *lexer) {
    Token token;
    token.cat = INVALID;
    token.val = NOP;  

    skipWhitespaceAndComments(lexer);  

    if (lexer->currentChar == EOF) {
        token.cat = EOF_TOKEN;
        return token;
    }

    switch (lexer->currentChar) {
        case 'l':
            return scanL(lexer);
        case 's':
            return scanS(lexer);
        case 'r':
            return scanR(lexer);
        case 'm':
            return scanMult(lexer);  
        case 'a':
            return scanAdd(lexer);   
        case 'n':
            return scanNop(lexer);   
        case 'o':
            return scanOutput(lexer); 
        case '=':
            advance(lexer);
            if (lexer->currentChar == '>') {
                advance(lexer);  
                return (Token){ARROW, 0};  
            }
            break;
        case ',':
            advance(lexer);
            return (Token){COMMA, 0};  
        default:
            if (isdigit(lexer->currentChar)) {
                token.cat = CONSTANT;
                token.val = readNumberFromLexer(lexer);  
            } else {
                advance(lexer);  
            }
    }

    return token;
}

void printToken(Token token) {
    switch (token.cat) {
        case INSTRUCTION:
            printf("INSTRUCTION: Opcode %d\n", token.val);
            break;
        case REGISTER:
            printf("REGISTER: r%d\n", token.val);
            break;
        case CONSTANT:
            printf("CONSTANT: %d\n", token.val);
            break;
        case COMMA:
            printf("COMMA\n");
            break;
        case ARROW:
            printf("ARROW\n");
            break;
        case EOF_TOKEN:
            printf("EOF\n");
            break;
        default:
            printf("INVALID TOKEN\n");
            break;
    }
}
