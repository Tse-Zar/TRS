#include <memory_paging.h>
#include <types.h>
#include <string.h>
#include <app.h>

static ullong used_pages   = 0ULL;
static ullong total_pages  = 0ULL;
static ullong  cur = 0ULL;

static ullong* bitmap;

#define BM_GET(p) ((bitmap[(p) >> 6] >> ((p) & 63)) & 1ULL)
#define BM_SET(p) (bitmap[(p) >> 6] |= (1ULL << ((p) & 63)))
#define BM_CLR(p) (bitmap[(p) >> 6] &= ~(1ULL << ((p) & 63)))

void pmm_init(boot_info_t *bi) {
    bitmap = (ullong*)(bi->bitmap_vaddr);
    total_pages = bi->total_pages;

    if((total_pages + 7) >> 3 > bi->bitmap_size)
        total_pages = (bi->bitmap_size * 8) & ~63ULL;

    for(ullong p = 0; p < total_pages; ++p) {
        if(BM_GET(p)) used_pages++;
    }
}

void* alloc_pages(ullong n) {
    if(!n) return (void*)0;

    for(int pass = 0; pass < 2; ++pass) {
        ullong run = 0;
        for(ullong p = cur; p < total_pages; ++p) {
            if(BM_GET(p)) {
                run = 0;
                continue;
            }
            if(++run == n) {
                ullong start = p - n + 1;
                for(ullong i = 0; i < n; ++i) {
                    BM_SET(start + i);
                }

                used_pages += n;
                cur = p + 1;
                return (void*)(start * PAGE_SIZE);
            }
        }

        cur = 0;
    }
    return (void*)0;
}

void make_page_executable(ullong vaddr) {
    ullong cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    ullong* pml4 = (ullong*)(cr3 & ~0xFFFULL);

    ullong pml4_idx = (vaddr >> 39) & 0x1FF;
    ullong pdpt_idx = (vaddr >> 30) & 0x1FF;
    ullong pd_idx   = (vaddr >> 21) & 0x1FF;
    ullong pt_idx   = (vaddr >> 12) & 0x1FF;

    if (!(pml4[pml4_idx] & 1)) return;
    ullong* pdpt = (ullong*)(pml4[pml4_idx] & 0x000FFFFFFFFFF000ULL);

    if (!(pdpt[pdpt_idx] & 1)) return;
    pdpt[pdpt_idx] &= ~(1ULL << 63);
    if (pdpt[pdpt_idx] & (1ULL << 7)) return;

    ullong* pd = (ullong*)(pdpt[pdpt_idx] & 0x000FFFFFFFFFF000ULL);

    if (!(pd[pd_idx] & 1)) return;
    pd[pd_idx] &= ~(1ULL << 63);
    if (pd[pd_idx] & (1ULL << 7)) return;

    ullong* pt = (ullong*)(pd[pd_idx] & 0x000FFFFFFFFFF000ULL);

    if (!(pt[pt_idx] & 1)) return;
    pt[pt_idx] &= ~(1ULL << 63);

    __asm__ volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
}


void free_page(void* page) {
    ullong p = (ullong)page / PAGE_SIZE;
    if(p >= total_pages || !BM_GET(p)) return;

    BM_CLR(p);
    used_pages--;
    if(p < cur) cur = p;
}

ullong pmm_usable_pages(void) {
    return total_pages - used_pages;
}

ullong pmm_total_pages(void) {
    return total_pages;
}

ullong pmm_used_pages(void) {
    return used_pages;
}

/* --- VIRTUAL MEMORY --- */

ullong vmm_get_cr3(void) {
    ullong cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static void map_page(ullong* pml4, ullong vaddr, ullong paddr, ullong flags) {
    ullong pml4_idx = (vaddr >> 39) & 0x1FF;
    ullong pdpt_idx = (vaddr >> 30) & 0x1FF;
    ullong pd_idx   = (vaddr >> 21) & 0x1FF;
    ullong pt_idx   = (vaddr >> 12) & 0x1FF;

    /* --- pdpt --- */
    ullong* pdpt;
    if(!(pml4[pml4_idx] & 1)) {
        ullong pdpt_phys = (ullong)alloc_pages(1);
        memset((void*)pdpt_phys, 0, PAGE_SIZE);

        pml4[pml4_idx] = pdpt_phys | PTE_P | PTE_WR | PTE_US;
        pdpt = (ullong*)pdpt_phys;
    } else {
        pdpt = (ullong*)(pml4[pml4_idx] & PTE_ADDR);
    }

    /* --- pd --- */
    ullong* pd;
    if(!(pdpt[pdpt_idx] & 1)) {
        ullong pd_phys = (ullong)alloc_pages(1);
        memset((void*)pd_phys, 0, PAGE_SIZE);

        pdpt[pdpt_idx] = pd_phys | PTE_P | PTE_WR | PTE_US;
        pd = (ullong*)pd_phys;
    } else {
        pd = (ullong*)(pdpt[pdpt_idx] & PTE_ADDR);
    }

    /* --- pt --- */
    ullong* pt;
    if(!(pd[pd_idx] & PTE_P)) {
        ullong pt_phys = (ullong)alloc_pages(1);
        memset((void*)pt_phys, 0, PAGE_SIZE);
        pd[pd_idx] = pt_phys | PTE_P | PTE_WR | flags;
        pt = (ullong*)pt_phys;
    } else {
        pt = (ullong*)(pd[pd_idx] & PTE_ADDR);
    }

    pt[pt_idx] = (paddr & PTE_ADDR) | flags;
    __asm__ volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
}

ullong vmm_create_cr3(void) {
    ullong table_phys = (ullong)alloc_pages(1);
    memset((void*)table_phys, 0, PAGE_SIZE);
    ullong* new_pml4 = (ullong*)table_phys;

    ullong cr3 = vmm_get_cr3();

    /* --- copy high half kernel --- */
    ullong* kern_pml4 = (ullong*)(cr3 & PTE_ADDR);
    for(int i = 256; i < 512; ++i) {
        new_pml4[i] = kern_pml4[i];
    }

    return table_phys | PTE_P;
}

void vmm_map_user(ullong cr3, ullong img_phys, ullong img_size, ullong stack_phys, ullong stack_size) {
    ullong* pml4 = (ullong*)(cr3 & PTE_ADDR);

    /* --- code RX --- */
    ullong img_pages = (img_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for(ullong p = 0; p < img_pages; ++p) {
        map_page(pml4, USER_BASE + p * PAGE_SIZE, img_phys + p * PAGE_SIZE, USER_CODE_FLAGS);
    }

    /* --- stack RW --- */
    ullong stack_pages = (stack_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for(ullong p = 0; p < stack_pages; ++p) {
        map_page(pml4, USER_TOP - (p + 1 ) * PAGE_SIZE, stack_phys, USER_DATA_FLAGS);
    }

    /* --- guard --- */
    map_page(pml4, USER_TOP - stack_pages - img_pages, 0, 0);
}

void vmm_switch(ullong cr3) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(cr3) : "memory");
}
