#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>

void printHelp(char* name) {
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", name);
    puts("Options:");
    puts("  -h         Print this help message.");
    puts("  -v         Optional verbose flag.");
    puts("  -s <num>   Number of set index bits.");
    puts("  -E <num>   Number of lines per set.");
    puts("  -b <num>   Number of block offset bits.");
    puts("  -t <file>  Trace file.\n");

    puts("Examples:");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", name);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", name);
}

int main(int argc, char *argv[])
{
    char ch;
    char *trace_file = NULL;
    uint8_t set_len, line_len, block_size = 0;
    while((ch = getopt(argc, argv, "hvs:E:b:t")) != -1) {
        switch(ch) {
            case 'h': {
                printHelp(argv[0]);
                return 0;
            }
            case 'v': {
                break;
            }
            case 's': {
                set_len = (uint8_t)atoi(optarg);
                break;
            }
            case 'E': {
                line_len = (uint8_t)atoi(optarg);
                break;
            }
            case 'b': {
                block_size = (uint8_t)atoi(optarg);
                break;
            }
            case 't': {
                trace_file = optarg;
                break;
            }
            default:
                break;
        }
    }
    if(!(set_len) || !(line_len) || !(block_size) || !(trace_file)) {
        fprintf(stderr, "%s: Missing required command line argument\n", argv[0]);
        printHelp(argv[0]);
        return EXIT_FAILURE;
    }
    printSummary(0, 0, 0);
    return 0;
}
