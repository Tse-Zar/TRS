#ifndef BOOTINFO_H
#define BOOTINFO_H

#define KERNEL_VBASE      0xFFFFFFFF80000000ULL
#define KERNEL_FB_VBASE   0xFFFFFFFF60000000ULL
#define KSTACK_VBASE      0xFFFFFFFFF0000000ULL
#define BOOTINFO_VBASE    0xFFFFFFFFF8000000ULL
#define PMMBMP_VBASE      0xFFFFFFFFF8100000ULL
#define MMAP_VBASE        0xFFFFFFFFF9000000ULL
#define KERNEL_MAX_PHYS   0x80000000ULL
#define KERNEL_STACK_PG   32

/* --- page table ehtry --- */
#define PTE_P    (1ULL << 0)
#define PTE_WR   (1ULL << 1)
#define PTE_US   (1ULL << 2)
#define PTE_PCD  (1ULL << 4)
#define PTE_PS   (1ULL << 7)
#define PTE_NX   (1ULL << 63)
#define PTE_ADDR 0x000FFFFFFFFFF000ULL

#define MM_UNUSED (PTE_P | PTE_WR | PTE_NX)
#define MM_CODE   (PTE_P | PTE_WR)
#define MM_DATA   (PTE_P | PTE_WR | PTE_NX)
#define MM_DEV    (PTE_P | PTE_WR | PTE_NX | PTE_PCD)
#define MM_UCODE  (PTE_P | PTE_WR | PTE_US)
#define MM_UDATA  (PTE_P | PTE_WR | PTE_NX | PTE_US)

#define KROUND_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

typedef struct {
    unsigned int       type;
    unsigned int       pad;
    unsigned long long phys_start;
    unsigned long long virt_start;
    unsigned long long num_pages;
    unsigned long long attr;
} mmap_desc_t;

typedef enum {
    ReservedMemoryType,
    LoaderCode,
    LoaderData,
    BootServicesCode,
    BootServicesData,
    RuntimeServicesCode,
    RuntimeServicesData,
    ConventionalMemory,
    UnusableMemory,
    ACPIReclaimMemory,
    ACPIMemoryNVS,
    MemoryMappedIO,
    MemoryMappedIOPortSpace,
    PalCode,
    PersistentMemory,
    UnacceptedMemoryType,
    MaxMemoryType
} kern_mem_type;

typedef struct {
    unsigned long long base;
    unsigned long long size;
    
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    unsigned int bpp;
    unsigned int rmask;
    unsigned int gmask;
    unsigned int bmask;
    unsigned int reserved_mask;
} framebuf_info_t;

typedef struct {
    framebuf_info_t fb;

    unsigned long long mmap_vaddr;
    unsigned long long mmap_paddr;
    unsigned long long mmap_size;
    unsigned long long mmap_dsize;

    unsigned long long stack_top;
    unsigned long long identity_base;
    
    unsigned long long kern_size;
    unsigned long long kern_base;
    unsigned long long kern_paddr;
    unsigned long long kern_code_pg;
    unsigned long long hight_half;

    unsigned long long bitmap_vaddr;
    unsigned long long bitmap_paddr;
    unsigned long long total_pages;
    unsigned long long bitmap_size;
} boot_info_t;

#define KERN_MAGIC 0x5452534BU
typedef struct {
    unsigned int magic;
    unsigned int image_size;
    unsigned int mem_pages;
    unsigned int text_pages;
    unsigned int entry_off;
    unsigned int hdr_size;
} kern_hdr_t;


#endif // BOOTINFO_H