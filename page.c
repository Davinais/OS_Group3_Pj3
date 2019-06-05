#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <getopt.h>

#define INPUT_DATA  "test.txt"
#define OUTPUT_FILE "group3_ans.txt"
#define ADDR_LEN 32
#define BUF_SIZE 32

int main(int argc, char *argv[]) {
    char ch;
    uint8_t page_len = 0;
    while((ch = getopt(argc, argv, "n:")) != -1) {
        switch(ch) {
            case 'n': {
                int tmp = atoi(optarg);
                if((tmp > ADDR_LEN) || (tmp <= 0)) {
                    fprintf(stderr, "Error: Invalid page length!(Expected 1 to %d)\n", ADDR_LEN);
                    return EXIT_FAILURE;
                }
                page_len = (uint8_t)tmp;
                break;
            }   
            default:
                break;
        }
    }
    if(page_len == 0) {
        fprintf(stderr, "Error: Page Length Zero, Exited.\n");
        return EXIT_FAILURE;
    }

    FILE *input_fp, *output_fp;
    if(!(input_fp = fopen(INPUT_DATA, "r"))) {
        perror("Error in fopen");
        return EXIT_FAILURE;   
    }
    if(!(output_fp = fopen(OUTPUT_FILE, "w"))) {
        perror("Error in fopen");
        return EXIT_FAILURE;   
    }

    char buf[BUF_SIZE] = { 0 };
    uint32_t addr, page_num, page_offset;
    uint8_t offset_len = ADDR_LEN - page_len;
    while((fgets(buf, sizeof(buf), input_fp) != NULL)) {
        //Avoid 'only new line ending' on the last line
        if(buf[0] != '\r' && buf[0] != '\n') {
            sscanf(buf, "%x", &addr);
            page_num = addr >> offset_len;
            page_offset = addr ^ (page_num << offset_len);
            fprintf(output_fp, "%u %u\n", page_num, page_offset);
        }
    }
    fclose(input_fp);
    fclose(output_fp);
    return 0;
}
