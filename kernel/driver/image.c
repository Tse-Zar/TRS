#include <image.h>

long y = 0, x = 0;

void dsp_init(framebuf_info_t fb) {
    unsigned int* dst = (unsigned int*)fb.base;
    unsigned int total = (fb.pitch / 4) * fb.height;
    for(int i = 0; i < total; ++i) {
        dst[i] = 0x000000;
    }
}

void dsp_putchar(framebuf_info_t fb, char c) {
    unsigned int* dst = (unsigned int*)fb.base;

    if(c == '\b') return;
    if(c == '\n') return;
    if(c == '\t') return;

    dst[y * fb.width + ++x] = c;
    
}

void dsp_print(framebuf_info_t fb, const char *str) {

}