#include "os.h"

// virtual address: [sign ext|virtual page number|offset]
// bits:            [63    57|56               12|11   0]
// (1) From the structure of the virtual address, we get a total of 2**45 virtual pages
// (2) Size of each page = 4KB = 2**12 B
// (3) Size of each PTE is 64 bit = 8B = 2**3 B
// (4) From (2) + (3), we get that each Page Table trie node contains 2**12 / 2**3 = 2**9 PTEs
// (5) If each Page Table trie node contains 2**9 records, we can define a trie with D = 2**9
// (6) For a VPN of 45 bits, we will get a trie with 5 levels

#define OFFSET_BITS 12
#define VPN_BITS (57 - 12)
#define LEVEL_BITS 9
#define D 512
#define LEVELS 5


uint8_t check_valid(uint64_t ppn) {
    // Return the value of the least significant bit (valid bit)
    return (uint8_t)(ppn & 0x1);
}

uint64_t get_level_bits(uint64_t vpn, uint8_t level) {
    // Dividing the VPN bits to 5 levels, we drop the least significant VPN bits
    vpn = vpn >> (((LEVELS - 1) - level) * LEVEL_BITS);
    // Now we only need the 9 least significant bits
    vpn = vpn & 0x1ff;

    return vpn;
}


void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    uint64_t pte_addr, pte_offset, pte;
    uint64_t * rw_addr;
    int i;
    pt = pt << 12;
    for (i = 0; i < LEVELS; i++) {
        // First we get the specific trie level addr bits
        pte_offset = get_level_bits(vpn, i);
        pte_addr = (pt & ~0x1ff) + pte_offset;
        rw_addr = (uint64_t *)phys_to_virt(pte_addr);
        pte = *rw_addr;
        if (!check_valid(pte)) {
            if (ppn == NO_MAPPING) {
                // If the entry shows NO_MAPPING, and we want to remove the current PTE,
                // no action needs to be taken - just return
                return;
            }
            else {
            	if (i < LEVELS - 1) {
                	// If we want to save the current PN, we allocate a new trie node
                	*rw_addr = (alloc_page_frame() << 12) | 0x1;
                }
            }
        }
        pt = *rw_addr;
    }
    
    if (ppn == NO_MAPPING) {
        // If we wish to delete the page, we set the valid bit to 0
        *rw_addr = *rw_addr & (~1);
    }
    else {
        // Otherwise, we set the entry to be:
        //  PTE:  [   PPN   |  0  | 1 ]
        // bits:  [63     12|11  1| 0 ]
        *rw_addr = (ppn << 12) | 1;
    }
}


uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    uint64_t pte_addr, pte_offset, pte;
    uint64_t * rw_addr;
    int i;
    pt = pt << 12;
    for (i = 0; i < LEVELS; i++) {
        // First we get the specific trie level addr bits
        pte_offset = get_level_bits(vpn, i);
        pte_addr = (pt & ~0x1ff) + pte_offset;
        rw_addr = (uint64_t *)phys_to_virt(pte_addr);
        pte = *rw_addr;
        if (!check_valid(pte)) {
            // If the pte is not valid, we return no mapping
            return NO_MAPPING;
        }
        pt = pte;
    }
    return pt >> 12;
}
