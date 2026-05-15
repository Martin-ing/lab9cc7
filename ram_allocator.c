#include "ram_allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void ram_init(RAM *ram) {
    for (int i = 0; i < NUM_FRAMES; i++) {
        ram->frames[i].state = FRAME_FREE;
        ram->frames[i].owner = OWNER_NONE;
    }
    ram->free_count     = NUM_FRAMES;
    ram->occupied_count = 0;
    ram->seed           = 0;
}

unsigned int ram_randomize(RAM *ram, unsigned int seed, int need) {
    if (seed == 0)
        seed = (unsigned int)time(NULL);

    int threshold = (need > 10) ? need : 10;  // max(10, V)

    for (int iter = 0; iter < 50; iter++) {
        ram_init(ram);
        srand(seed);

        int occ = 10 + rand() % 51;  // entre 10 y 60

        int indices[NUM_FRAMES];
        for (int i = 0; i < NUM_FRAMES; i++) indices[i] = i;
        for (int i = NUM_FRAMES - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int tmp = indices[i]; indices[i] = indices[j]; indices[j] = tmp;
        }
        for (int i = 0; i < occ; i++) {
            ram->frames[indices[i]].state = FRAME_OCCUPIED;
            ram->frames[indices[i]].owner = OWNER_NONE;
        }
        ram->occupied_count = occ;
        ram->free_count     = NUM_FRAMES - occ;

        if (ram->free_count >= threshold) {
            ram->seed = seed;
            return seed;
        }
        seed++;
    }

    fprintf(stderr, "ERROR: no se pudo satisfacer free >= %d en 50 intentos.\n", threshold);
    exit(EXIT_FAILURE);
}

int ram_allocate_frame(RAM *ram, int owner) {
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (ram->frames[i].state == FRAME_FREE) {
            ram->frames[i].state = FRAME_OCCUPIED;
            ram->frames[i].owner = owner;
            ram->free_count--;
            ram->occupied_count++;
            return i;
        }
    }
    return -1;
}

void ram_free_frame(RAM *ram, int frame_idx) {
    if (frame_idx < 0 || frame_idx >= NUM_FRAMES) return;
    if (ram->frames[frame_idx].state == FRAME_OCCUPIED) {
        ram->frames[frame_idx].state = FRAME_FREE;
        ram->frames[frame_idx].owner = OWNER_NONE;
        ram->free_count++;
        ram->occupied_count--;
    }
}

void ram_print_map(const RAM *ram) {
    printf("\nPHYSICAL RAM (%d frames) after random init (seed=%u):\n",
           NUM_FRAMES, ram->seed);
    printf("FREE=%d  OCCUPIED=%d\n\n", ram->free_count, ram->occupied_count);

    for (int i = 0; i < NUM_FRAMES; i++) {
        char state;
        if (ram->frames[i].state == FRAME_FREE) {
            state = 'F';
        } else {
            int ow = ram->frames[i].owner;
            if      (ow == OWNER_PROC1) state = '1';
            else if (ow == OWNER_PROC2) state = '2';
            else                        state = 'X';
        }
        printf("%2d:%c", i, state);
        if ((i + 1) % 10 == 0) printf("\n");
        else                    printf("  ");
    }
    printf("\n");
}