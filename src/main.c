#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "lexer.h"
#include "parser.h"
#include "IR.h"

// Function declarations
void print_help();
void process_file(char *filename, int flag_lexer, int flag_pretty, int flag_table);

int main(int argc, char **argv) {
    int opt;
    int flag_lexer = 0, flag_pretty = 0, flag_table = 1;  // Default flag_table is set to 1
    
    struct option long_options[] = {
        {"lexer", no_argument, NULL, 'l'},
        {"pretty-print", no_argument, NULL, 'p'},
        {"table-print", no_argument, NULL, 't'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "lpth", long_options, NULL)) != -1) {
        switch (opt) {
            case 'l':
                flag_lexer = 1;
                flag_table = 0;  // Disable table-print when other flag is set
                break;
            case 'p':
                flag_pretty = 1;
                flag_table = 0;  // Disable table-print when other flag is set
                break;
            case 't':
                flag_table = 1;  // Explicitly enable table-print
                break;
            case 'h':
                print_help();
                exit(0);
            default:
                print_help();
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected filename after options\n");
        print_help();
        exit(EXIT_FAILURE);
    }

    char *filename = argv[optind];
    process_file(filename, flag_lexer, flag_pretty, flag_table);

    return 0;
}


void print_help() {
    printf("Usage: ./thc [options] filename\n");
    printf("Options:\n");
    printf("  -l, --lexer          Print list of tokens\n");
    printf("  -p, --pretty-print   Pretty print ILOC code\n");
    printf("  -t, --table-print    Print IR in tabular form\n");
    printf("  -h, --help           Print this help message\n");
}

void process_file(char *filename, int flag_lexer, int flag_pretty, int flag_table) {
    // Open the file
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    Lexer lexer;
    initLexer(&lexer, file);

    IR ir;
    initIR(&ir);
    
    if (flag_lexer) {
        Token token;
        // Print all tokens one by one
        while ((token = getNextToken(&lexer)).cat != EOF_TOKEN) {
            printToken(token);
        }
    } else {
        Parser parser;
        initParser(&parser, &lexer, &ir);
        parseProgram(&parser);

        if (flag_pretty) {
            printIR(&ir, PRETTY_PRINT);
        }
        
        if (flag_table) {
            printIR(&ir, TABLE_PRINT);
        }
    }

    fclose(file);
}
