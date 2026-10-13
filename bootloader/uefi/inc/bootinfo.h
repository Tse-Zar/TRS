#ifndef BOOTINFO_H
#define BOOTINFO_H

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
    unsigned long long mmap_addr;
    unsigned long long mmap_size;
    unsigned long long mmap_dsize;
    unsigned long long stack_top;
} memmap_t;

typedef struct {
    framebuf_info_t fb;
    //memmap_t        mm;

    unsigned long long mmap_addr;
    unsigned long long mmap_size;
    unsigned long long mmap_dsize;
    unsigned long long stack_top;
} boot_info_t;

#endif // BOOTINFO_H