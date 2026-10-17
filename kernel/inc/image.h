#ifndef IMAGE_H
#define IMAGE_H

#include <bootinfo.h>

#define _PanicGraphit 0x121212
#define _PanicDark    0x1a1a1a
#define _PanicRed     0xff4444
#define _PanicOrange  0xff6b00

void dsp_init(framebuf_info_t frame);
void dsp_set_color(unsigned int col_fg, unsigned int col_bg);
void dsp_set_cursor(unsigned int x, unsigned int y);
void dsp_clear(void);

void dsp_print(const char* str);
void dsp_putchar(char c);
void dsp_print_hex(unsigned long long v);
void dsp_print_dec(int num);

#endif