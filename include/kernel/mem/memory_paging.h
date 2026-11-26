/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = Physical memory manager ---------- =
    = bitmap built from the UEFI map --- =
    ======================================
*/

#ifndef MEMORY_PAGING_H
#define MEMORY_PAGING_H

#include <bootinfo.h>
#include <types.h>

#define PAGE_SIZE 4096ULL
#define MAX_PAGES_FOR_APP 4096

/* --- physical memory map --- */
void pmm_init(boot_info_t* bi);
void* alloc_pages(ullong p);
void make_page_executable(ullong vaddr);
void free_page(void* page);
ullong pmm_usable_pages(void);
ullong pmm_used_pages(void);
ullong pmm_total_pages(void);

/* --- virtual memory map --- */
ullong* vmm_init(void);
// void vmm_map_page(ullong cr3, ullong vaddr, ullong paddr, ullong flags);
void vmm_map_user(ullong cr3, ullong img_phys, ullong img_size, ullong stack_phys, ullong stack_size);
void vmm_switch(ullong cr3);
ullong vmm_get_cr3(void);

#endif // MEMORY_PAGING_H
