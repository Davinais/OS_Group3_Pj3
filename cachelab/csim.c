#include "cachelab.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>

#define BUFFER_SIZE 64

uint8_t set_len = 0;
uint8_t line_size = 0;
uint8_t block_len = 0;
uint32_t hit_count = 0, miss_count = 0, eviction_count = 0;

typedef struct line{
    uint64_t tag;
    struct list_head list;
}line_t;

typedef struct set{
    size_t size;
    struct list_head line_head;
}set_t;

typedef struct result{
    bool miss;
    bool hit;
    bool eviction;
}result_t;

static inline void list_pop(struct list_head *head) {
	if(!list_empty(head)) {
        line_t *popped = list_entry(head->next, line_t, list);
        list_del(head->next);
        free(popped);
    }
}

void freeLineList(struct list_head *head) {
    struct list_head *listptr, *tmp;
    line_t *entry;
    list_for_each_safe(listptr, tmp, head) {
        entry = list_entry(listptr, line_t, list);
        list_del(listptr);
        free(entry);
    }
}

line_t* searchCache(set_t *set, uint64_t tag){
    line_t *entry;
    list_for_each_entry(entry, &(set->line_head), list) {
        if(entry->tag == tag) {
            return entry;
        }
    }
    return NULL;
}

bool addLine(set_t *set, uint64_t tag){
    line_t* new_cache = (line_t*)calloc(1, sizeof(line_t));
    new_cache->tag = tag;

    //add to back of the list
    list_add_tail(&(new_cache->list), &(set->line_head));
    (set->size)++;

    //check valid cache
    if(set->size <= line_size){
        return false; //no eviction
    }
    else {
        // Delete first entry, since our list is in time order
        // First Entry would be the longest not used line
        list_pop(&(set->line_head));
        (set->size)--;
        return true;
    }
}

static inline uint64_t getTag(uint64_t addr){
    return addr >> block_len;
}

result_t access(set_t *set, uint64_t tag){
    result_t ret = {false, false, false};
    line_t *ret_cache;
    // Search if the line is in cache
    if((ret_cache = searchCache(set, tag)) != NULL){
        // Line Linklist is ordered in used time 
        // Recently used line would be put to the end
        list_move_tail(&(ret_cache->list), &(set->line_head));
        ret.hit = true;
        hit_count++;
    }
    else {
        ret.miss = true;
        miss_count++;
        // addLine() return true if there's eviction happened
        if(addLine(set, tag)){
            ret.eviction = true;
            eviction_count++;
        }
    }
    return ret;
}

result_t load(set_t *set, uint64_t tag){
    return access(set, tag);
}

//modify need to call load and store
result_t store(set_t *set, uint64_t tag){
    return access(set, tag);
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
    bool verbose = false;
    while((ch = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(ch) {
            case 'h': {
                printHelp(argv[0]);
                return 0;
            }
            case 'v': {
                verbose = true;
                break;
            }
            case 's': {
                set_len = (uint8_t)atoi(optarg);
                break;
            }
            case 'E': {
                line_size = (uint8_t)atoi(optarg);
                break;
            }
            case 'b': {
                block_len = (uint8_t)atoi(optarg);
                break;
            }
            case 't': {
                trace_file = strdup(optarg);
                break;
            }
            default:
                break;
        }
    }
    // If anything of the following be 0, the simulator won't work
    if(!(set_len) || !(line_size) || !(block_len) || !(trace_file)) {
        fprintf(stderr, "%s: Missing required command line argument\n", argv[0]);
        printHelp(argv[0]);
        return EXIT_FAILURE;
    }

    FILE *fptr;
    if(!(fptr = fopen(trace_file, "r"))) {
        perror("Error: ");
        return EXIT_FAILURE;
    }

    uint8_t set_size = 1 << set_len;
    set_t *cache = calloc(set_size, sizeof(set_t));
    // Initialize set array
    for(uint8_t i = 0; i < set_size; i++) {
        cache[i].size = 0;
        INIT_LIST_HEAD(&(cache[i].line_head));
    }

    char buf[BUFFER_SIZE] = { 0 };
    char cmd;
    uint64_t addr, offset, set, tag;
    uint64_t set_mask = ((~0UL) >> ((sizeof(uint64_t) << 3) - set_len)) << (block_len);
    result_t ret;
    while((fgets(buf, sizeof(buf), fptr) != NULL)) {
        // Avoid 'only new line ending' on the last line
        // Also avoid instruction load command
        if(buf[0] != '\r' && buf[0] != '\n' && buf[0] == ' ') {
            sscanf(buf, " %c %lxu,%lxu", &cmd, &addr, &offset);
            if(verbose) {
                // Remove new line char at the end of line
                if(buf[strlen(buf)-1] == '\n') {
                    buf[strlen(buf)-1] = 0;
                    if(buf[strlen(buf)-1] == '\r') {
                        buf[strlen(buf)-1] = 0;
                    }
                }
                printf("%s", buf+1);
            }
            set = (addr & set_mask) >> (block_len);
            tag = getTag(addr);
            switch (cmd) {
                case 'L': {
                    ret = load(cache+set, tag);
                    break;
                }
                case 'S': {
                    ret = store(cache+set, tag);
                    break;
                }
                case 'M': {
                    ret = load(cache+set, tag);
                    if(verbose) {
                        if(ret.miss) {
                            printf(" miss");
                        }
                        else if(ret.hit) {
                            printf(" hit");
                        }
                        if(ret.eviction) {
                            printf(" eviction");
                        }
                    }
                    ret = store(cache+set, tag);
                    break;
                }
                default:
                    break;
            }
            if(verbose) {
                if(ret.miss) {
                    printf(" miss");
                }
                else if(ret.hit) {
                    printf(" hit");
                }
                if(ret.eviction) {
                    printf(" eviction");
                }
                puts("");
            }
        }
    }
    fclose(fptr);

    printSummary(hit_count, miss_count, eviction_count);
    for(uint8_t i = 0; i < set_size; i++) {
        freeLineList(&(cache[i].line_head));
    }
    free(cache);
    free(trace_file);
    return 0;
}
