/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = TRS VGA TEXT MODE - 80x25 -------- =
    ======================================
*/

#include "../inc/vga.h"

static int row = 0;
static int col = 0;
static unsigned char vga_attr = 0x0F;

static const char hex_digits[] = "0123456789ABCDEF";

static inline unsigned char _make_attr(_color fg, _color bg) {
    return (unsigned char)((bg << 4) | (fg & 0x0F));
}

static void dsp_scroll(void) {
    for(int y = 0; y < _HEIGHT - 1; ++y) {
        for(int x = 0; x < _WIDTH; ++x) {
            _MEMORY[(y * _WIDTH + x)] = _MEMORY[(y + 1) * _WIDTH + x];
        }
    }
    unsigned short blank = (unsigned short)(vga_attr << 8) | ' ';
    for(int x = 0; x < _WIDTH; ++x) {
        _MEMORY[(_HEIGHT - 1) * _WIDTH + x] = blank;
    }
    row = _HEIGHT - 1;
}

void dsp_init(void) {
    row = 0;
    col = 0;
    vga_attr = _make_attr(_WHITE, _BLACK);
    dsp_clear();
}

void dsp_clear(void) {
    unsigned short blank = (unsigned short)(vga_attr << 8) | ' ';
    for(int i = 0; i < _WIDTH * _HEIGHT; ++i) {
        _MEMORY[i] = blank;
    } 

    row = 0;
    col = 0;
}

void dsp_set_color(_color fg, _color bg) {
    vga_attr = _make_attr(fg, bg);
}

static void check_col_row(void) {
    if(col >= _WIDTH) {
        col = 0;
        row++;
        if(row >= _HEIGHT) {
            dsp_scroll();
        }
    }
}

void dsp_putchar(char c) {
    if(c == '\n') {
        col = 0;
        row++;
        if(row >= _HEIGHT) {
            dsp_scroll();
        }
        return;
    }
    
    if(c == '\r') {
        col = 0;
        return;
    }

    if(c == '\t') {
        col = (col + 4) & ~3;
        check_col_row();

        return;
    }

    unsigned short entry = (unsigned short)(vga_attr << 8) | (unsigned char)c;
    _MEMORY[row * _WIDTH + col] = entry;
    col++;

    check_col_row();
}

void dsp_print(const char *str) {
    while(*str) {
        dsp_putchar(*str++);
    }
}

void dsp_print_hex(unsigned int value) {
    dsp_print("0x");
    int started = 0;
    for(int i = 28; i >= 0; i -= 4) {
        unsigned char nibble = (unsigned char)((value >> i) & 0xF);
        if(nibble || started || i == 0) {
            dsp_putchar(hex_digits[nibble]);
            started = 1;
        }
    }
}