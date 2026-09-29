#ifndef IMAGE_H
#define IMAGE_H

#include <bootinfo.h>

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    float a;
} _rgba_color;

void dsp_init(framebuf_info_t fb);
void dsp_set_color(_rgba_color);
void dsp_print(framebuf_info_t fb, const char* str);
void dsp_putchar(framebuf_info_t fb, char c);
#endif