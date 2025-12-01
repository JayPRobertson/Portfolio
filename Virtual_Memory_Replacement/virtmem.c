#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
 
/*
 * Some compile-time constants.
 */
 
#define REPLACE_NONE 0
#define REPLACE_FIFO 1
#define REPLACE_LRU  2
#define REPLACE_CLOCK 3
#define REPLACE_OPTIMAL 4
 
#define TRUE 1
#define FALSE 0
#define PROGRESS_BAR_WIDTH 60
#define MAX_LINE_LEN 100
 
/*
 * Some function prototypes to keep the compiler happy.
 */
int setup(void);
int teardown(void);
int output_report(void);
long resolve_address(long, int);
void error_resolve_address(long, int);
 
int lru_replace();
int fifo_replace();
int clock_replace();
 
/*
 * Variables used to keep track of the number of memory-system events
 * that are simulated.
 */
int page_faults = 0;
int mem_refs    = 0;
int swap_outs   = 0;
int swap_ins    = 0;
 
 
/*
 * Page-table information. You are permitted to modify this in order to
 * implement schemes such as CLOCK. However, you are not required
 * to do so.
 */
struct page_table_entry* page_table = NULL;
struct page_table_entry {
    long page_num;
    int dirty;
    int free;
    int arrival_time;       //FIFO
    int last_access_time;   //LRU
    int reference_bit;      //CLOCK
};
 
 
/*
 * These global variables will be set in the main() function. The default
 * values here are non-sensical, but it is safer to zero out a variable
 * rather than trust to random data that might be stored in it -- this
 * helps with debugging (i.e., eliminates a possible source of randomness
 * in misbehaving programs).
 */
 
int size_of_frame = 0;  /* power of 2 */
int size_of_memory = 0; /* number of frames */
int page_replacement_scheme = REPLACE_NONE;
int clock_hand = 0;
 
/*
 * Function to convert a logical address into its corresponding 
 * physical address. The value returned by this function is the
 * physical address (or -1 if no physical address can exist for
 * the logical address given the current page-allocation state.
 */
 
long resolve_address(long logical, int memwrite) {
    long page, frame;
    long offset;
    long mask = 0;
    long effective;
    int i;
    
    /* Get the page and offset */
    page = (logical >> size_of_frame);

    for (i = 0; i < size_of_frame; i++) {
        mask = mask << 1;
        mask |= 1;
    }
    offset = logical & mask;

    /* Find page in the inverted page table. */
    frame = -1;
    for (i = 0; i < size_of_memory; i++) {
        if (!page_table[i].free && page_table[i].page_num == page) {
            frame = i;
            break;
        }
    }
        
    /* If frame is not -1, then we can successfully resolve the
     * address and return the result. */
    if (frame != -1) {
        if (memwrite){ //if write operation performed, make dirty
            page_table[frame].dirty = memwrite;
        }
        page_table[frame].reference_bit = 1;
        page_table[frame].last_access_time = mem_refs;
        effective = (frame << size_of_frame) | offset;
        return effective;
    }
        
    /* If we reach this point, there was a page fault. Find
     * a free frame. */
     page_faults++;
     
     for (i = 0; i < size_of_memory; i++) {
        if (page_table[i].free) {
            frame = i;
            break;
        }
    }
        
    /* If we found a free frame, then patch up the
     * page table entry and compute the effective
     * address. Otherwise, swap.
     */
    if (frame != -1) {
        page_table[frame].page_num = page;
        page_table[frame].free = FALSE;
        page_table[frame].arrival_time = mem_refs;
        page_table[frame].last_access_time = mem_refs;
        swap_ins++;
        effective = (frame << size_of_frame) | offset;
        return effective;
    }
    
    if (page_replacement_scheme == REPLACE_FIFO){
        frame = fifo_replace();
    }else if (page_replacement_scheme == REPLACE_LRU){
        frame = lru_replace();
    }else if (page_replacement_scheme == REPLACE_CLOCK){
        frame = clock_replace();
    }else {
        return -1;
    }

    if (page_table[frame].dirty){
        swap_outs++;
    }
    swap_ins++;
    page_table[frame].page_num = page;
    effective = (frame << size_of_frame) | offset;
    return effective;
}

/* Finds the index of the page table entry with the lowest arrival time.
   Note: does not update last_access_time or reference_bit since these are
         not needed for FIFO */
int fifo_replace(){
    int minarrive = 0;
    for (int i = 1; i < size_of_memory; i++){
        if (page_table[i].arrival_time < page_table[minarrive].arrival_time){
            minarrive = i;
        }
    }
    page_table[minarrive].arrival_time = mem_refs; 
    return minarrive;
}

/* Finds the index of the page table entry with the oldest access time.
   Note: does not update arrival_time or reference_bit since these are
         not needed for LRU */
int lru_replace(){
    int longest_acc = 0;

    for (int i = 1; i < size_of_memory; i++){
        if (page_table[i].last_access_time < page_table[longest_acc].last_access_time){
            longest_acc = i;
        }
    }
    page_table[longest_acc].last_access_time = mem_refs;
    return longest_acc;
}

/* Finds the index of the first page table entry with reference_bit=0.
   Note: does not update arrival_time or last_access_time since these are
         not needed for CLOCK */
int clock_replace(){
    int to_replace = -1;

    while (to_replace == -1){
        if (page_table[clock_hand].reference_bit == 0){
            to_replace = clock_hand;
        }else{
            page_table[clock_hand].reference_bit = 0; //give second chance
        }

        clock_hand = (clock_hand + 1) % size_of_memory;
    }
    return to_replace;
}
 
/*
 * Super-simple progress bar.
 */
void display_progress(int percent){
     int to_date = PROGRESS_BAR_WIDTH * percent / 100;
     static int last_to_date = 0;
     int i;
 
     if (last_to_date < to_date){
         last_to_date = to_date;
     } else {
         return;
     }
 
     printf("Progress [");
     for (i=0; i<to_date; i++){
         printf(".");
     }
     for (; i<PROGRESS_BAR_WIDTH; i++){
         printf(" ");
     }
     printf("] %3d%%", percent);
     printf("\r");
     fflush(stdout);
 }
 
 
int setup(){
     int i;
 
     page_table = (struct page_table_entry *)malloc(
         sizeof(struct page_table_entry) * size_of_memory
     );
 
     if (page_table == NULL){
         fprintf(stderr,
             "Simulator error: cannot allocate memory for page table.\n");
         exit(1);
     }
 
     for (i=0; i<size_of_memory; i++){
         page_table[i].free = TRUE;
         page_table[i].reference_bit = 1;
         page_table[i].arrival_time = -1;
         page_table[i].last_access_time = -1;
     }
 
     return -1;
}
 
 
int teardown(){
    if (page_table != NULL) {
        free(page_table);
        page_table = NULL;
    }
     return -1;
}
 
 
void error_resolve_address(long a, int l){
     fprintf(stderr, "\n");
     fprintf(stderr, 
         "Simulator error: cannot resolve address 0x%lx at line %d\n",
         a, l
     );
     exit(1);
}
 
 
int output_report(){
     printf("\n");
     printf("Memory references: %d\n", mem_refs);
     printf("Page faults: %d\n", page_faults);
     printf("Swap ins: %d\n", swap_ins);
     printf("Swap outs: %d\n", swap_outs);
 
     return -1;
}
 
 
int main(int argc, char **argv){
     /* For working with command-line arguments. */
     int i;
     char *s;
 
     /* For working with input file. */
     FILE *infile = NULL;
     char *infile_name = NULL;
     struct stat infile_stat;
     int  line_num = 0;
     int infile_size = 0;
 
     /* For processing each individual line in the input file. */
     char buffer[MAX_LINE_LEN];
     long addr;
     char addr_type;
     int  is_write;
 
     /* For making visible the work being done by the simulator. */
     int show_progress = FALSE;
 
     /* Process the command-line parameters. Note that the
      * REPLACE_OPTIMAL scheme is not required.
      */
     for (i=1; i < argc; i++){
         if (strncmp(argv[i], "--replace=", 9) == 0){
             s = strstr(argv[i], "=") + 1;
             if (strcmp(s, "fifo") == 0){
                 page_replacement_scheme = REPLACE_FIFO;
             } else if (strcmp(s, "lru") == 0){
                 page_replacement_scheme = REPLACE_LRU;
             } else if (strcmp(s, "clock") == 0){
                 page_replacement_scheme = REPLACE_CLOCK;
             } else if (strcmp(s, "optimal") == 0){
                 page_replacement_scheme = REPLACE_OPTIMAL;
             } else {
                 page_replacement_scheme = REPLACE_NONE;
             }
         } else if (strncmp(argv[i], "--file=", 7) == 0){
             infile_name = strstr(argv[i], "=") + 1;
         } else if (strncmp(argv[i], "--framesize=", 12) == 0){
             s = strstr(argv[i], "=") + 1;
             size_of_frame = atoi(s);
         } else if (strncmp(argv[i], "--numframes=", 12) == 0){
             s = strstr(argv[i], "=") + 1;
             size_of_memory = atoi(s);
         } else if (strcmp(argv[i], "--progress") == 0){
             show_progress = TRUE;
         }
     }
 
     if (infile_name == NULL){
         infile = stdin;
     } else if (stat(infile_name, &infile_stat) == 0){
         infile_size = (int)(infile_stat.st_size);
         /* If this fails, infile will be null */
         infile = fopen(infile_name, "r");  
     }
 
 
     if (page_replacement_scheme == REPLACE_NONE ||
         size_of_frame <= 0 ||
         size_of_memory <= 0 ||
         infile == NULL)
     {
         fprintf(stderr, 
             "usage: %s --framesize=<m> --numframes=<n>", argv[0]);
         fprintf(stderr, 
             " --replace={fifo|lru|optimal} [--file=<filename>]\n");
         exit(1);
     }
 
 
     setup();
 
     while (fgets(buffer, MAX_LINE_LEN-1, infile)){
         line_num++;
         if (strstr(buffer, ":")){
             sscanf(buffer, "%c: %lx", &addr_type, &addr);
             if (addr_type == 'W'){
                 is_write = TRUE;
             } else {
                 is_write = FALSE;
             }
 
             if (resolve_address(addr, is_write) == -1){
                 error_resolve_address(addr, line_num);
             }
             mem_refs++;
         } 
 
         if (show_progress){
             display_progress(ftell(infile) * 100 / infile_size);
         }
     }
     
 
     teardown();
     output_report();
 
     fclose(infile);
 
     exit(0);
}
 
 