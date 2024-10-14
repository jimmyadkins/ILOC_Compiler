#pragma once
#include <stdbool.h>
#include <stdio.h>

extern int debugLevel;

// Error handling functions
void error(char* msg);
void assertCondition(bool condition, char* errmsg);

// Debugging functions
void debug(char* str);
void debug_l(char* str, int level);

// Utility functions for parsing whitespace
void skipWhitespace(FILE* input);
void skipBlankspace(FILE* input);
char passWhitespace(FILE* input);
char passBlankspace(FILE* input);

// Utility function for reading numbers
bool readNumber(FILE* input, int* result);

void skip_comments(FILE *input);
