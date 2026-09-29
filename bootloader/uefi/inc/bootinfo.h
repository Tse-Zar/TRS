#ifndef BOOTINFO_H
#define BOOTINFO_H

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
    unsigned long long mmap_addr;
    unsigned long long mmap_size;
    unsigned long long mmap_dsize;
    unsigned long long stack_top;
} boot_info_t;

#endif // BOOTINFO_H