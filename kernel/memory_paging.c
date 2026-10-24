/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = Physical memory manager ---------- =
    = bitmap built from the UEFI map --- =
    ======================================
*/

#include <memory_paging.h>
#include <panic.h>
#include <string.h>

extern char __kernel_start[];
extern char __kernel_end[];

static unsigned char*     bitmap = 0;
static unsigned long long bitmap_bytes = 0;
static unsigned long long total_pages = 0;
static unsigned long long used_pages = 0;
static unsigned long long usable_pages = 0;
static unsigned long long last_alloc = 0;

static inline void bitmap_set(unsigned long long p) { bitmap[p >> 3] |= (unsigned char)(1u << (p & 7)); }
static inline void bitmap_clear(unsigned long long p) { bitmap[p >> 3] &= (unsigned char)~(1u << (p & 7)); }
static inline int  bitmap_test(unsigned long long p) { return bitmap[p >> 3] >> (p & 7); }

static unsigned long long align_up(unsigned long long v, unsigned long long a) {
    return (v + a - 1) & ~(a - 1);
}

static mem_desc_t* desc_at(boot_info_t* bi, unsigned long long i) {
    return (mem_desc_t*)((unsigned char*)(unsigned long long)bi->mmap_addr + i * bi->mmap_dsize);
}

static void reserv_range(unsigned long long start, unsigned long long size) {
    unsigned long long first = start / PAGE_SIZE;
    unsigned long long last = align_up(start + size, PAGE_SIZE) / PAGE_SIZE;

    for(unsigned long long p = first; p < last && p < total_pages; ++p) {
        if(!bitmap_test(p)) { bitmap_set(p); used_pages++; }
    } 
}

void pmm_init(boot_info_t *bi) {
    if(bi->mmap_dsize == 0 || bi->mmap_size == 0) {
        kpanic("Memory map is missing", __FILE__, __LINE__);
    }

    unsigned long long count = bi->mmap_size / bi->mmap_dsize;

    unsigned long long max_addr = 0;
    for(unsigned long long i = 0; i < count; ++i) {
        mem_desc_t* desc = desc_at(bi, i);
        if(desc->type != ConventionalMemory) continue;

        unsigned long long end = desc->phys + desc->pages * PAGE_SIZE;
        if(end > max_addr) max_addr = end;
    }

    if(max_addr == 0) kpanic("No convetnional memory found", __FILE__, __LINE__);

    total_pages = max_addr / PAGE_SIZE;
    bitmap_bytes = align_up(total_pages / 8 + 1, PAGE_SIZE);

    unsigned long long best = 0, best_pages = 0;
    for(unsigned long long i = 0; i < count; ++i) {
        mem_desc_t* d = desc_at(bi, i);
        if(d->type != ConventionalMemory) continue;
        if(d->pages > best_pages) {
            best_pages = d->pages;
            best = d->phys;
        }
    }

    bitmap = (unsigned char*)align_up(best, PAGE_SIZE);
    if((unsigned long long)bitmap + bitmap_bytes > best + best_pages * PAGE_SIZE) {
        kpanic("Bitmap does not fit into memory", __FILE__, __LINE__);
    }

    memset(bitmap, 0xFF, bitmap_bytes);

    for(unsigned long long i = 0; i < count; ++i) {
        mem_desc_t* d = desc_at(bi, i);
        if(d->type != ConventionalMemory) continue;

        unsigned long long first = d->phys / PAGE_SIZE;
        for(unsigned long long p = first; p < first + d->pages && p < total_pages; ++p) {
            bitmap_clear(p);
        }

        usable_pages += d->pages;
    }

    reserv_range(0, 0x100000);
    reserv_range(0x400000, 0x100000);
    reserv_range((unsigned long long)__kernel_start, (unsigned long long)(__kernel_end - __kernel_start));
    reserv_range((unsigned long long)bitmap, bitmap_bytes);
    reserv_range(bi->fb.base, bi->fb.size);
    reserv_range(bi->stack_top - 128ULL * 1024, 128ULL * 1024);
    reserv_range(bi->mmap_addr, bi->mmap_size);
    reserv_range(0x400000, 0x100000);
}

void* pmm_alloc_pages(unsigned long long  pages) {
    if (!bitmap || pages == 0) return NULL;

    unsigned long long consecutive_pages = 0;
    unsigned long long start_page = 0;

    for (unsigned long long p = 0; p < total_pages; ++p) {
        if (!bitmap_test(p)) {
            if (consecutive_pages == 0) {
                start_page = p; 
            }
            consecutive_pages++;

            if (consecutive_pages == pages) {
                for (unsigned long long i = start_page; i <= p; ++i) {
                    bitmap_set(i);
                }
                
                used_pages += pages;
                last_alloc = p + 1;
                
                return (void*)(start_page * PAGE_SIZE);
            }
        } else {
            consecutive_pages = 0;
        }
    }

    return NULL; 
}


void pmm_free_page(void *page) {
    if(!bitmap) return;

    unsigned long long p = (unsigned long long)page / PAGE_SIZE;
    if(p >= total_pages || !bitmap_test(p)) return;

    bitmap_clear(p);
    used_pages--;
    if(p < last_alloc) last_alloc = p;
}

unsigned long long pmm_usable_pages(void) { return usable_pages; }
unsigned long long pmm_used_pages(void) { return used_pages; }