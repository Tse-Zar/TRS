#include <memory_paging.h>

static unsigned long long used_pages   = 0ULL;
static unsigned long long total_pages  = 0ULL;
static unsigned long long  cur = 0ULL;

static unsigned long long* bitmap;

#define BM_GET(p) ((bitmap[(p) >> 6] >> ((p) & 63)) & 1ULL)
#define BM_SET(p) (bitmap[(p) >> 6] |= (1ULL << ((p) & 63)))
#define BM_CLR(p) (bitmap[(p) >> 6] &= ~(1ULL << ((p) & 63)))

void pmm_init(boot_info_t *bi) {
    bitmap = (unsigned long long*)(bi->bitmap_vaddr);
    total_pages = bi->total_pages;

    if((total_pages + 7) >> 3 > bi->bitmap_size) 
        total_pages = (bi->bitmap_size * 8) & ~63ULL;

    for(unsigned long long p = 0; p < total_pages; ++p) {
        if(BM_GET(p)) used_pages++;
    }
}

void* alloc_pages(unsigned long long n) {
    if(!n) return (void*)0;

    for(int pass = 0; pass < 2; ++pass) {
        unsigned long long run = 0;
        for(unsigned long long p = cur; p < total_pages; ++p) {
            if(BM_GET(p)) {
                run = 0;
                continue;
            }
            if(++run == n) {
                unsigned long long start = p - n + 1;
                for(unsigned long long i = 0; i < n; ++i) {
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

void make_page_executable(unsigned long long vaddr) {
    unsigned long long cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    unsigned long long* pml4 = (unsigned long long*)(cr3 & ~0xFFFULL);

    unsigned long long pml4_idx = (vaddr >> 39) & 0x1FF;
    unsigned long long pdpt_idx = (vaddr >> 30) & 0x1FF;
    unsigned long long pd_idx   = (vaddr >> 21) & 0x1FF;
    unsigned long long pt_idx   = (vaddr >> 12) & 0x1FF;

    if (!(pml4[pml4_idx] & 1)) return;
    unsigned long long* pdpt = (unsigned long long*)(pml4[pml4_idx] & 0x000FFFFFFFFFF000ULL);

    if (!(pdpt[pdpt_idx] & 1)) return;
    pdpt[pdpt_idx] &= ~(1ULL << 63); 
    if (pdpt[pdpt_idx] & (1ULL << 7)) return;

    unsigned long long* pd = (unsigned long long*)(pdpt[pdpt_idx] & 0x000FFFFFFFFFF000ULL);

    if (!(pd[pd_idx] & 1)) return;
    pd[pd_idx] &= ~(1ULL << 63);
    if (pd[pd_idx] & (1ULL << 7)) return;

    unsigned long long* pt = (unsigned long long*)(pd[pd_idx] & 0x000FFFFFFFFFF000ULL);

    if (!(pt[pt_idx] & 1)) return;
    pt[pt_idx] &= ~(1ULL << 63);

    __asm__ volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
}


void free_page(void* page) {
    unsigned long long p = (unsigned long long)page / PAGE_SIZE;
    if(p >= total_pages || !BM_GET(p)) return;
    
    BM_CLR(p);
    used_pages--;
    if(p < cur) cur = p;
}

void* vmm_init(void) {
    unsigned long long cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return (void*)(cr3 & ~0xFFFULL);
}
unsigned long long pmm_usable_pages(void) {
    return total_pages - used_pages;
}

unsigned long long pmm_total_pages(void) {
    return total_pages;
}

unsigned long long pmm_used_pages(void) {
    return used_pages;
}