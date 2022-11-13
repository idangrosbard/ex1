#include "os.h"

// [sign ext|virtual page number|offset]
// [63    57|56               12|11   0]
// Total of 57 - 12 = 45 bits. Using 9 bits per level.
// 5 levels, 2**9 = 512 entries node
#define OFFSET_BITS 12
#define VPN_BITS (57 - 12)
#define LEVEL_BITS 9
#define D 512
#define LEVELS 5


void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    uint64_t curr_level_pt;
    uint64_t curr_level_pte;
    uint64_t entry;
    int curr_level;

    entry = pt;
    for (curr_level = 0; curr_level < LEVELS; curr_level++) {
        curr_level_pt = entry;
        curr_level_pte = vpn;
        // First we drop the offset bits (not part of the VPN)
        curr_level_pte = curr_level_pte >> OFFSET_BITS;
        // Then, based on the trie level, we drop the least significant bits (in multiplications of 9)
        curr_level_pte = curr_level_pte >> (((LEVELS - 1) - curr_level) * LEVEL_BITS);
        // Finaly, we take only the least significant 9 bits
        curr_level_pte = curr_level_pte & 0x1ff;
        entry = *(uint64_t *)(curr_level_pt + curr_level_pte);
        if (entry == NO_MAPPING) {
            if (ppn == NO_MAPPING) {
                // If the entry shows NO_MAPPING, and we want to remove the current PTE, 
                // no action needs to be taken - just return NO_MAPPING
                return;
            }
            else {
                // If we want to save the current PN, we allocate a new trie node
                *(uint64_t *)(curr_level_pt + curr_level_pte) = alloc_page_frame();
                entry = *(uint64_t *)(curr_level_pt + curr_level_pte);
            }
        }
    }
    *(uint64_t *)(curr_level_pt + curr_level_pte) = ppn;
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    uint64_t curr_level_pt;
    uint64_t curr_level_pte;
    uint64_t entry;
    int curr_level;

    entry = pt;
    for (curr_level = 0; curr_level < LEVELS; curr_level++) {
        curr_level_pt = entry;
        curr_level_pte = vpn;
        // First we drop the offset bits (not part of the VPN)
        curr_level_pte = curr_level_pte >> OFFSET_BITS;
        // Then, based on the trie level, we drop the least significant bits (in multiplications of 9)
        curr_level_pte = curr_level_pte >> (((LEVELS - 1) - curr_level) * LEVEL_BITS);
        // Finaly, we take only the least significant 9 bits
        curr_level_pte = curr_level_pte & 0x1ff;
        entry = *(uint64_t *)(curr_level_pt + curr_level_pte);
        if (entry == NO_MAPPING) {
            return NO_MAPPING;
        }
    }
    entry = *(uint64_t *)(curr_level_pt + curr_level_pte);
    return entry;
}
