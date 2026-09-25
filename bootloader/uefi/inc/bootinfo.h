#ifndef BOOTINFO_H
#define BOOTINFO_H

typedef struct {
    unsigned long long framebuf_addr;
    unsigned long long framebuf_size;
    unsigned int framebuf_width;
    unsigned int framebuf_height;
    unsigned int framebuf_pitch;
    unsigned int framebuf_bpp;
    unsigned long long mmap_addr;
    unsigned long long mmap_size;
    unsigned long long mmap_dsize;
    unsigned long long stack_top;
} boot_info_t;

#endif // BOOTINFO_H