#include "translator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

bool va_decompose(uint32_t va, uint8_t *vpn, uint8_t *offset) {
    if (va > (uint32_t)VA_MAX) {
        if (vpn)    *vpn    = 0;
        if (offset) *offset = 0;
        return false;
    }
    if (offset) *offset = (uint8_t)(va & 0xFF);
    if (vpn)    *vpn    = (uint8_t)((va >> 8) & 0xFF);
    return true;
}

Translation translate(uint32_t va, const Process *proc) {
    Translation t;
    memset(&t, 0, sizeof(t));
    t.va = va;

    /* Check 1 – VA range */
    if (va > (uint32_t)VA_MAX) {
        t.result = TRANS_VA_OUT_OF_RANGE;
        return t;
    }

    t.offset = (uint8_t)(va & 0xFF);
    t.vpn    = (uint8_t)((va >> 8) & 0xFF);

    /* Check 2 – VPN within process range */
    if (t.vpn >= (uint8_t)proc->num_virtual_pages) {
        t.result = TRANS_VPN_OUT_OF_RANGE;
        return t;
    }

    /* Check 3 – PTE valid */
    if (!proc->page_table[t.vpn].valid) {
        t.result = TRANS_PAGE_NOT_MAPPED;
        return t;
    }

    /* Success */
    t.pfn    = proc->page_table[t.vpn].pfn;
    t.pa     = (uint32_t)t.pfn * PAGE_SIZE + t.offset;
    t.result = TRANS_OK;
    return t;
}

/* Full printer that knows V */
static void print_translation_full(const Translation *t, int V) {
    switch (t->result) {
        case TRANS_OK:
            printf("VA=0x%04X (%-5u)  VPN=0x%02X  OFF=0x%02X  PFN=%-3d  PA=0x%04X (%u)\n",
                   t->va, t->va,
                   t->vpn, t->offset,
                   t->pfn,
                   t->pa, t->pa);
            break;

        case TRANS_VA_OUT_OF_RANGE:
            printf("VA=%-10u  ERROR=VA_OUT_OF_RANGE  (0x%X > 0xFFFF)\n",
                   t->va, t->va);
            break;

        case TRANS_VPN_OUT_OF_RANGE:
            printf("VA=0x%04X (%-5u)  VPN=0x%02X  OFF=0x%02X  ERROR=VPN_OUT_OF_RANGE"
                   "  (vpn=%u, V=%d)\n",
                   t->va, t->va,
                   t->vpn, t->offset,
                   t->vpn, V);
            break;

        case TRANS_PAGE_NOT_MAPPED:
            printf("VA=0x%04X (%-5u)  VPN=0x%02X  OFF=0x%02X  ERROR=PAGE_NOT_MAPPED\n",
                   t->va, t->va, t->vpn, t->offset);
            break;
    }
}

void translation_print(const Translation *t) {
    print_translation_full(t, -1);
}

int translate_file(const char *file_path, const Process *proc) {
    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot open address file '%s': %s\n",
                file_path, strerror(errno));
        return -1;
    }

    printf("\n--- Batch Translation [PID=%d] ---\n", proc->pid);

    char line[64];
    int  count = 0;

    while (fgets(line, sizeof(line), fp)) {
        /* Skip blank lines / comments */
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\n' || *p == '\r' || *p == '#' || *p == '\0') continue;

        /* Parse decimal or 0x hex */
        uint32_t va;
        char *endptr;
        errno = 0;

        if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
            va = (uint32_t)strtoul(p, &endptr, 16);
        else
            va = (uint32_t)strtoul(p, &endptr, 10);

        if (errno != 0 || endptr == p) {
            printf("  [line %d] PARSE ERROR: '%s'\n", count + 1, line);
            continue;
        }

        Translation t = translate(va, proc);
        print_translation_full(&t, proc->num_virtual_pages);
        count++;
    }

    fclose(fp);
    return count;
}
