#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 

int debugLevel = 0;

void error(char* msg) {
    const char *prefix = "Error: ";
    fwrite(prefix, sizeof(char), strlen(prefix), stderr);
    fwrite(msg, sizeof(char), strlen(msg), stderr);
    fwrite("\n", sizeof(char), 1, stderr);  // Add newline
    exit(EXIT_FAILURE);
}

void assertCondition(bool condition, char* errmsg) {
    if (!condition) {
        const char *prefix = "Assertion failed: ";
        fwrite(prefix, sizeof(char), strlen(prefix), stderr);
        fwrite(errmsg, sizeof(char), strlen(errmsg), stderr);
        fwrite("\n", sizeof(char), 1, stderr);  // Add newline
        exit(EXIT_FAILURE);
    }
}

void debug(char* str) {
    debug_l(str, 1);
}

void debug_l(char* str, int level) {
    if (debugLevel >= level)
        printf("//%s\n", str);
}

// Skip past all whitespace characters 
void skipWhitespace(FILE* input) {
    int ch;
    while ((ch = getc(input)) != EOF) {
        if (!isspace(ch)) {
            ungetc(ch, input);
            break;
        }
    }
}

// Skip past only spaces and tabs
void skipBlankspace(FILE* input) {
    int ch;
    while ((ch = getc(input)) != EOF) {
        if (ch != ' ' && ch != '\t') {
            ungetc(ch, input);
            break;
        }
    }
}

// Return the next non-whitespace character, skipping whitespace
char passWhitespace(FILE* input) {
    skipWhitespace(input);
    return getc(input);
}

// Return the next non-blankspace character, skipping only spaces and tabs
char passBlankspace(FILE* input) {
    skipBlankspace(input);
    return getc(input);
}

// Reads a number from the input one character at a time
bool readNumber(FILE* input, int* result) {
    skipWhitespace(input);
    
    int num = 0;
    int sign = 1;
    int ch = getc(input);
    
    // Check for negative sign
    if (ch == '-') {
        sign = -1;
        ch = getc(input);
    }

    if (!isdigit(ch)) {
        ungetc(ch, input);  // If not a digit, return failure
        return false;
    }

    while (isdigit(ch)) {
        num = num * 10 + (ch - '0');
        ch = getc(input);
    }

    ungetc(ch, input);  // Put back the last non-digit character
    *result = num * sign;
    return true;
}

void skip_comments(FILE *input) {
    int c;
    while ((c = getc(input)) != EOF) {
        if (c == '\n') {
            break;  // End of the comment
        }
    }
}

