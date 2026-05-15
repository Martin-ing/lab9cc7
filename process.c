#include "process.h"
#include <stdio.h>
#include <string.h>

void process_init(Process *proc, int pid, int num_pages, int owner_id) {
    proc->pid              = pid;
    proc->num_virtual_pages = num_pages;
    proc->owner_id         = owner_id;
    for (int i = 0; i < MAX_VIRTUAL_PAGES; i++) {
        proc->page_table[i].valid = false;
        proc->page_table[i].pfn   = 0;
    }
}

bool process_load(Process *proc, RAM *ram) {
    int V = proc->num_virtual_pages;

    /* Sanity: assert enough free frames exist */
    if (ram->free_count < V) {
        fprintf(stderr,
            "LOAD ERROR [PID %d]: Not enough free frames (%d free, need %d).\n",
            proc->pid, ram->free_count, V);
        return false;
    }

    int allocated[MAX_VIRTUAL_PAGES];
    int count = 0;

    for (int vpn = 0; vpn < V; vpn++) {
        int frame = ram_allocate_frame(ram, proc->owner_id);
        if (frame == -1) {
            /* Mid-load failure – rollback */
            fprintf(stderr,
                "LOAD ERROR [PID %d]: Allocation failed at VPN %d. Rolling back.\n",
                proc->pid, vpn);
            for (int k = 0; k < count; k++) {
                ram_free_frame(ram, allocated[k]);
                proc->page_table[k].valid = false;
            }
            return false;
        }
        proc->page_table[vpn].valid = true;
        proc->page_table[vpn].pfn   = (uint8_t)frame;
        allocated[count++]          = frame;
    }

    /* Print load summary */
    printf("\nLoad process [PID=%d]: V=%d  ->  VPN 0..%d mapped to PFNs [ ",
           proc->pid, V, V - 1);
    for (int i = 0; i < V; i++) {
        printf("%d", proc->page_table[i].pfn);
        if (i < V - 1) printf(", ");
    }
    printf(" ]\n");

    return true;
}

void process_print_page_table(const Process *proc) {
    printf("\nPage Table [PID=%d] (%d entries):\n", proc->pid, proc->num_virtual_pages);
    printf("  %-6s  %-6s  %s\n", "VPN", "Valid", "PFN");
    printf("  ------  ------  ---\n");
    for (int i = 0; i < proc->num_virtual_pages; i++) {
        if (proc->page_table[i].valid)
            printf("  0x%02X    YES     %d\n", i, proc->page_table[i].pfn);
        else
            printf("  0x%02X    NO      -\n", i);
    }
    printf("\n");
}
