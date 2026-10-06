#ifndef MEMORY_PAGGING_H
#define MEMORY_PAGGING_H

#include <std.h>
#include <bootinfo.h>

#define PAGE_SIZE 4096
#define HIGHT_OFFSET 0xFFFFFFFF80000000ULL

/* --- Physical Memory Mapping --- */
void pmm_init(boot_info_t info);
void* pmm_alloc_block(void);
void pmm_free_block(void* addr);

/* --- Virtual Memory Mapping --- */
void vmm_init(void);
void vmm_map_page(unsigned long long* pml4, unsigned long long vaddr, unsigned long long paddr, unsigned long long flags);
static inline void* phys_to_virt(void* phys) {
    return (void*)((unsigned long long)phys + HIGHT_OFFSET);
}

/* --- Kernel Malloc --- */
void kmalloc_init(unsigned long long kernel_pml4);
void* kmalloc(unsigned long long size);
void kfree(void* ptr);

#endif // MEMORY_PAGGING_