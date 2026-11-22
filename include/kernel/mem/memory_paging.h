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
void* vmm_init(void);

#endif // MEMORY_PAGING_H
