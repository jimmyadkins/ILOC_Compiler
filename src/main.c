#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "utils.h"
#include "lexer.h"
#include "parser.h"
#include "IR.h"
#include "allocator.h"
#include "scheduler.h"

// Function declarations
void print_help();
void process_file(char *filename, int flag_lexer, int flag_pretty, int flag_table, int flag_alloc, int num_registers, int flag_debug, int flag_sched);

// Main function
int main(int argc, char **argv) {
    int opt;
    int flag_debug = 0, flag_lexer = 0, flag_pretty = 0, flag_table = 0, flag_alloc = 1, flag_sched = 0;  // Default is allocator (-a)
    int num_registers = 4;  // Default register count
    
    struct option long_options[] = {
        {"lexer", no_argument, NULL, 'l'},
        {"pretty-print", optional_argument, NULL, 'p'},
        {"table-print", no_argument, NULL, 't'},
        {"alloc", no_argument, NULL, 'a'},
        {"sched", no_argument, NULL, 's'}, // Add the scheduling flag
        {"registers", required_argument, NULL, 'k'},
        {"help", no_argument, NULL, 'h'},
        {"debug", no_argument, NULL, 'd'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "lptask:hd", long_options, NULL)) != -1) {
        switch (opt) {
            case 'l':
                flag_lexer = 1;
                flag_alloc = 0;  // Disable allocator when lexer flag is set
                break;
            case 'p':
                flag_pretty = 1;
                flag_alloc = 0;  // Disable allocator when pretty-print flag is set
                if (optarg) {
                    // Optional parameter for pretty-print to specify register type
                    // (e.g., source, virtual, physical), handle it if required
                    // Example: parse and set a variable here if necessary
                }
                break;
            case 't':
                flag_table = 1;  // Enable table-print explicitly
                flag_alloc = 0;  // Disable allocator when table-print flag is set
                break;
            case 'a':
                flag_alloc = 1;   // Enable register allocation explicitly
                break;
            case 'k':
                num_registers = atoi(optarg);  // Set number of registers for allocation
                if (num_registers <= 0) {
                    fprintf(stderr, "Error: Number of registers must be positive.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                print_help();
                exit(0);
            case 'd':
                flag_debug = 1;  // Enable debugging mode
                
                break;
            case 's':
                flag_sched = 1;
                flag_alloc = 0;  // Disable allocator when scheduling is active
                break;
            default:
                print_help();
                exit(EXIT_FAILURE);
        }
    }

    // Check for required filename argument
    if (optind >= argc) {
        fprintf(stderr, "Expected filename after options\n");
        print_help();
        exit(EXIT_FAILURE);
    }

    char *filename = argv[optind];

    // Default to allocator if no print flag is set
    if (!flag_lexer && !flag_pretty && !flag_table && !flag_sched) {
        flag_alloc = 1;
    }

    // Process the file with the specified flags
    process_file(filename, flag_lexer, flag_pretty, flag_table, flag_alloc, num_registers, flag_debug, flag_sched);

    return 0;
}

// Function to print the help message
void print_help() {
    printf("Usage: ./thc [options] filename\n");
    printf("Options:\n");
    printf("  -l, --lexer                Print list of tokens\n");
    printf("  -p, --pretty-print [reg]   Pretty print ILOC code (optional: reg type)\n");
    printf("  -t, --table-print          Print IR in tabular form\n");
    printf("  -a, --alloc                Perform register allocation on the block (default if no print flags)\n");
    printf("  -s, --sched                Perform scheduler operations on the block\n");
    printf("  -k, --registers num        Number of registers to use for allocation (default 4)\n");
    printf("  -d, --debug                Print debugging information\n");
    printf("  -h, --help                 Print this help message\n");
    printf("\nIf -a is enabled (default), outputs an equivalent block of ILOC code with registers 0 to k-1.\n");
}

// Function to process the file based on the specified flags
void process_file(char *filename, int flag_lexer, int flag_pretty, int flag_table, int flag_alloc, int num_registers, int flag_debug, int flag_sched) {
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

    if (flag_debug) {
        debugLevel = 1;
    }
    
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

        if (flag_sched) {
            debug(1, "Initializing scheduling...");
            Allocator allocator;
            initAllocator(&allocator, &ir, num_registers);
            computeLastUse(&allocator);
            DependencyGraph *graph = createDependencyGraph(&ir);
            computeLatencies(graph);
            printDependencyGraph(graph);
            freeDependencyGraph(graph);
        }

        if (flag_alloc) {
            // Run the allocator if -a flag is provided or defaulted
            Allocator allocator;
            debug(1, "Initializing allocator with %d registers...", num_registers);
            initAllocator(&allocator, &ir, num_registers);
            debug(1, "Computing last use...");
            computeLastUse(&allocator);
            debug(1, "Allocating registers...");
            //printf("Allocating registers...\n");
            allocateRegisters(&allocator);
            // debug(1, "Printing allocated IR.");
            printAllocatedIR(&allocator);  // Print the IR after register allocation
        } else {
            if (flag_pretty) {
                printIR(&ir, PRETTY_PRINT);
            }
            
            if (flag_table) {
                printIR(&ir, TABLE_PRINT);
            }
        }
    }

    fclose(file);
}
