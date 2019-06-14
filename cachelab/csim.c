#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>

#define BUFFER_SIZE 64

/* Simple Doubly Linked List from Linux Kernel */

/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized list entries.
 */
#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)

/**
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */
struct list_head {
	struct list_head *next, *prev;
};

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/**
 * Casts a member of a structure out to the containing structure
 * @param ptr        the pointer to the member.
 * @param type       the type of the container struct this is embedded in.
 * @param member     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                      \
        const struct list_head *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
/*@}*/

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new_,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new_;
	new_->next = next;
	new_->prev = prev;
	prev->next = new_;
}

/**
 * list_add - add a new entry
 * @new_: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new_ entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add(struct list_head *new_, struct list_head *head)
{
	__list_add(new_, head, head->next);
}

/**
 * list_add_tail - add a new entry
 * @new_: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new_, struct list_head *head)
{
	__list_add(new_, head->prev, head);
}


/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}



/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void list_move(struct list_head *list, struct list_head *head)
{
        __list_del(list->prev, list->next);
        list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
        __list_del(list->prev, list->next);
        list_add_tail(list, head);
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */

#define list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head);	\
       pos = pos->next)

/**
 * __list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 *
 * This variant differs from list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or 1 entry) most of the time.
 */
#define __list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; prefetch(pos->prev), pos != (head); \
        	pos = pos->prev)

/**
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop counter.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, line_t, member);	\
	     &pos->member != (head);					\
	     pos = list_entry(pos->member.next, line_t, member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_entry((head)->prev, line_t, member);	\
	     &pos->member != (head); 	\
	     pos = list_entry(pos->member.prev, line_t, member))


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
                trace_file = calloc(strlen(optarg)+1, sizeof(char));
                strcpy(trace_file, optarg);
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
