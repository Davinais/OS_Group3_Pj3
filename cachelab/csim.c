#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>

int cache_size;
int set_size;
int line_len;
int block_size;

typedef struct cache{
    uint64_t tag;
    struct cache* next;
}cache_t;

typedef struct result{
    bool miss;
    bool hit;
    bool eviction;
}result_t;

int size_of_cache(int set_size, int line_len, int block_size){
    return pow(2, (set_size + line_len + block_size));
}

cache_t* searchCache(cache_t* head, int tag){
    if(head == NULL) return NULL;
    if(head->tag == tag) return head;
    return search(head->next, tag);
}

bool addCache(cache_t* head, int tag){
    cache_t* new_cache = (cache_t*)malloc(sizeof(cache_t));
    new_cache->tag = tag;
    new_cache->next = NULL;

    //add to back of the list
    cache_t* temp = head;
    int size_of_current_set = 0;
    while(temp->next != NULL){
        size_of_current_set ++;
        temp = temp->next;
    }

    //add new node
    temp = new_cache;
    
    //check valid cache
    if(size_of_cache(size_of_current_set, line_len, block_size) < size_of_cache(set_size, line_len, block_size)){
        return false; //no eviction
    }
    else {
        //delete head
        head = head->next;
        return true;
    }
} 

uint64_t addr2tag(int addr){
    uint64_t tag = 0;
    return tag;
}

result_t load(cache_t* head, int addr){
    result_t ret = {false, false, false};
    uint64_t tag = addr2tag(addr);
    if(searchCache(head, tag) != NULL) ret.hit = true;
    else {
        ret.miss = true;
        if(addCache(head, tag)) ret.eviction = true;
    }
}

//same wtf? modify need to call load and store
result_t store(cache_t* head, int addr){
    result_t ret = {false, false, false};
    uint64_t tag = addr2tag(addr);
    if(searchCache(head, tag) != NULL) ret.hit = true;
    else {
        ret.miss = true;
        if(addCache(head, tag)) ret.eviction = true;
    }
}

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
