#ifndef RAM_ALLOCATOR_H
#define RAM_ALLOCATOR_H

#include <stdint.h>

#define NUM_FRAMES       100
#define FRAME_FREE       0
#define FRAME_OCCUPIED   1

/* Owner IDs for extra-credit multi-process support */
#define OWNER_NONE       -1
#define OWNER_PROC1       1
#define OWNER_PROC2       2

typedef struct {
    int  state;   /* FRAME_FREE or FRAME_OCCUPIED */
    int  owner;   /* OWNER_NONE, OWNER_PROC1, OWNER_PROC2 */
} Frame;

typedef struct {
    Frame frames[NUM_FRAMES];
    int   free_count;
    int   occupied_count;
    unsigned int seed;
} RAM;

/* Initialize RAM struct (all frames FREE, counts set) */
void ram_init(RAM *ram);

/*
 * Randomly mark some frames OCCUPIED to simulate pre-used memory.
 * occupied_count ∈ [10, 60], retried until free_count >= max(10, need).
 * seed: 0 → time-based; else use provided seed.
 * owner: which process "pre-occupies" (use OWNER_NONE for random holes).
 * Returns the seed actually used.
 */
unsigned int ram_randomize(RAM *ram, unsigned int seed, int need);

/*
 * Allocate one FREE frame for the given owner.
 * Returns frame index on success, -1 if none available.
 */
int ram_allocate_frame(RAM *ram, int owner);

/* Free a frame (used if rollback is needed) */
void ram_free_frame(RAM *ram, int frame_idx);

/* Print the compact 10-per-row RAM map */
void ram_print_map(const RAM *ram);

#endif /* RAM_ALLOCATOR_H */
