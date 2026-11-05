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

#define PAGE_SIZE 4096
#define MAX_PAGES_FOR_APP 524288

/* --- phisical memory map --- */
void pmm_init(boot_info_t* bi);
void* alloc_pages(unsigned long long p);
void make_page_executable(unsigned long long vaddr);
void free_page(void* page);
unsigned long long pmm_usable_pages(void);
unsigned long long pmm_used_pages(void);
unsigned long long pmm_total_pages(void);

/* --- virtual memory map --- */
void* vmm_init(void);

#endif // MEMORY_PAGING_H