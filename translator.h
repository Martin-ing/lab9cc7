#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "process.h"

/* Page / frame size in bytes */
#define PAGE_SIZE        256
#define VA_MAX           0xFFFF
#define VA_BITS          16

/* Translation result codes */
typedef enum {
    TRANS_OK              = 0,
    TRANS_VA_OUT_OF_RANGE = 1,
    TRANS_VPN_OUT_OF_RANGE= 2,
    TRANS_PAGE_NOT_MAPPED = 3
} TransResult;

/* Full result of one translation */
typedef struct {
    TransResult result;
    uint32_t    va;       /* original virtual address (might be > 0xFFFF on error) */
    uint8_t     vpn;
    uint8_t     offset;
    uint8_t     pfn;
    uint32_t    pa;
} Translation;

/*
 * Decompose va into vpn + offset.
 * Returns false and sets vpn and offset to 0 if va > VA_MAX.
 */
bool va_decompose(uint32_t va, uint8_t *vpn, uint8_t *offset);

/*
 * Translate a single virtual address for the given process.
 * Fills in *out completely.
 */
Translation translate(uint32_t va, const Process *proc);

/*
 * Print one translation result line in the required format.
 */
void translation_print(const Translation *t);

/*
 * Read virtual addresses (decimal or 0x hex) from file_path,
 * translate each, and print results.
 * Returns number of addresses processed, -1 on file error.
 */
int translate_file(const char *file_path, const Process *proc);

#endif /* TRANSLATOR_H */
