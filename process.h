#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "ram_allocator.h"

#define MAX_VIRTUAL_PAGES 256

/* Page Table Entry */
typedef struct {
    bool    valid;
    uint8_t pfn;   /* physical frame number (0..99 fits in uint8_t) */
} PTE;

/* Process Control Block (simplified) */
typedef struct {
    int  pid;                          /* process identifier              */
    int  num_virtual_pages;            /* V: number of virtual pages       */
    PTE  page_table[MAX_VIRTUAL_PAGES];/* linear page table, V entries used*/
    int  owner_id;                     /* OWNER_PROC1 or OWNER_PROC2       */
} Process;

/*
 * Initialize a Process struct: marks all PTEs invalid.
 * pid      : process id (e.g. 1 or 2)
 * num_pages: V, must be in [1..256]
 * owner_id : RAM owner label (OWNER_PROC1 / OWNER_PROC2)
 */
void process_init(Process *proc, int pid, int num_pages, int owner_id);

/*
 * Load the process into RAM: allocate one frame per virtual page.
 * On failure mid-load, rolls back all allocations and returns false.
 * Prints load summary on success.
 */
bool process_load(Process *proc, RAM *ram);

/* Print the process page table */
void process_print_page_table(const Process *proc);

#endif /* PROCESS_H */
