/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = TRS VGA TEXT MODE - 80x25 -------- =
    ======================================
*/

#ifndef VGA_H
#define VGA_H

#define _WIDTH   80
#define _HEIGHT  25
#define _MEMORY ((volatile unsigned short*)0xB8000)

typedef enum {
    _BLACK           = 0x0,
    _BLUE            = 0x1,
    _GREEN           = 0x2,
    _CYAN            = 0x3,
    _RED             = 0x4,
    _MAGENTA         = 0x5,
    _BROWN           = 0x6,
    _LIGHT_GREY      = 0x7,
    _DARK_GREY       = 0x8,
    _LIGHT_BLUE      = 0x9,
    _LIGHT_GREEN     = 0xA,
    _LIGHT_CYAN      = 0xB,
    _LIGHT_RED       = 0xC,
    _LIGHT_MAGENTA   = 0xD,
    _YELLOW          = 0xE,
    _WHITE           = 0xF
} _color;

void dsp_init(void);
void dsp_clear(void);
void dsp_set_color(_color fg, _color bg);
void dsp_putchar(char c);
void dsp_print(const char* str);

#endif // VGA_H