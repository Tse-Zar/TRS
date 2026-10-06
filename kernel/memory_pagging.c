#include <memory_pagging.h>
#include <std.h>
#include <string.h>

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL

static unsigned long long* get_next_table(unsigned long long* cur, unsigned int idx, unsigned long long flg) {
    unsigned long long entry = cur[idx];

    if(!(entry & PAGE_PRESENT)) {
        void* new_tab_phys = pmm_alloc_block();
        if(!new_tab_phys) {
            return NULL;
        }

        unsigned long long* new_tab_virt = (unsigned long long*)phys_to_virt(new_tab_phys);

        // PAGE_SIZE(4096 byte) / 8 byte = 512 tabs 
        memset(new_tab_phys, 0, 512);

        cur[idx] = (unsigned long long)new_tab_virt | PAGE_PRESENT | PAGE_WRITABLE | (flg & 0xFFF);
        entry = cur[idx];
    }

    unsigned long long next_table_phys = entry & PAGE_ADDR_MASK;

    return (unsigned long long*)phys_to_virt((void*)next_table_phys);
}

void vmm_map_page(unsigned long long *pml4, unsigned long long vaddr, unsigned long long paddr, unsigned long long flags) {
    unsigned int pml4_idx = (vaddr >> 39) & 0x1FF;
    unsigned int pdpt_idx = (vaddr >> 30) & 0x1FF;
    unsigned int pd_idx   = (vaddr >> 21) & 0x1FF;
    unsigned int pt_idx   = (vaddr >> 12) & 0x1FF;

    unsigned long long* pdpt = get_next_table(pml4, pml4_idx, flags);
    if(!pdpt) return;

    unsigned long long* pd = get_next_table(pdpt, pdpt_idx, flags);
    if(!pd) return;

    unsigned long long* pt = get_next_table(pd, pd_idx, flags);
    if(!pt) return;

    pt[pt_idx] = (paddr & PAGE_ADDR_MASK) | (flags & 0xFFF) | PAGE_PRESENT;

    __asm__ volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
}

void pmm_init(boot_info_t info) {
    unsigned long long total_size = info.mmap_size;
    
}