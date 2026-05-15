#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ram_allocator.h"
#include "process.h"
#include "translator.h"

int main(int argc, char *argv[]) {

    if (argc < 3) { fprintf(stderr, "ERROR: faltan argumentos.\n"); return EXIT_FAILURE; }

    int V1 = atoi(argv[1]);
    if (V1 < 1 || V1 > 256) {
        fprintf(stderr, "ERROR: num_virtual_pages must be in [1..256], got %d\n", V1);
        return EXIT_FAILURE;
    }
    const char *addr_file1 = argv[2];

    unsigned int seed = 0;
    int next_arg = 3;
    if (next_arg < argc && argv[next_arg][0] != '-') {
        seed = (unsigned int)strtoul(argv[next_arg], NULL, 10);
        next_arg++;
    }

    int V2 = 0;
    const char *addr_file2 = NULL;
    if (next_arg < argc && strcmp(argv[next_arg], "--proc2") == 0) {
        next_arg++;
        if (next_arg + 1 >= argc) {
            fprintf(stderr, "ERROR: --proc2 requires <V2> and <addr_file2>\n");
            return EXIT_FAILURE;
        }
        V2         = atoi(argv[next_arg++]);
        addr_file2 = argv[next_arg++];
        if (V2 < 1 || V2 > 256) {
            fprintf(stderr, "ERROR: V2 must be in [1..256], got %d\n", V2);
            return EXIT_FAILURE;
        }
    }

    int total_pages_needed = V1 + V2;

    RAM ram;
    ram_init(&ram);
    seed = ram_randomize(&ram, seed, total_pages_needed);
    ram_print_map(&ram);

    Process proc1;
    process_init(&proc1, 1, V1, OWNER_PROC1);
    if (!process_load(&proc1, &ram)) {
        fprintf(stderr, "FATAL: Failed to load Process 1.\n");
        return EXIT_FAILURE;
    }
    process_print_page_table(&proc1);

    int n1 = translate_file(addr_file1, &proc1);
    if (n1 < 0) return EXIT_FAILURE;
    printf("  [Total translated for PID=1: %d addresses]\n", n1);

    if (V2 > 0 && addr_file2 != NULL) {
        printf("\n=== Extra Credit: Second Process ===\n");
        printf("Remaining free frames after loading PID=1: %d\n", ram.free_count);

        Process proc2;
        process_init(&proc2, 2, V2, OWNER_PROC2);
        if (!process_load(&proc2, &ram)) {
            fprintf(stderr, "FATAL: Failed to load Process 2.\n");
            return EXIT_FAILURE;
        }
        process_print_page_table(&proc2);

        printf("\nUpdated RAM after loading both processes:\n");
        ram_print_map(&ram);

        int n2 = translate_file(addr_file2, &proc2);
        if (n2 < 0) return EXIT_FAILURE;
        printf("  [Total translated for PID=2: %d addresses]\n", n2);
    }

    return EXIT_SUCCESS;
}