#ifndef TOKEN_H
#define TOKEN_H

// token.h
typedef enum {
    TOKEN_LOAD,    
    TOKEN_LOADI,  
    TOKEN_STORE,   
    TOKEN_ADD,     
    TOKEN_SUB,     
    TOKEN_MULT,    
    TOKEN_LSHIFT,  
    TOKEN_RSHIFT,  
    TOKEN_OUTPUT,  
    TOKEN_NOP,    
    TOKEN_REGISTER, 
    TOKEN_CONSTANT, 
    TOKEN_COMMA,    
    TOKEN_ARROW,    
    TOKEN_EOF,      
    TOKEN_UNKNOWN   
} TokenType;

typedef struct {
    TokenType type;
    int value;     
    char* lexeme;  
} Token;

#endif
