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

void pmm_init(boot_info_t* bi);
void* pmm_alloc_page(void);
void pmm_free_page(void* page);

unsigned long long pmm_usable_pages(void);
unsigned long long pmm_used_pages(void);

#endif // MEMORY_PAGING_H