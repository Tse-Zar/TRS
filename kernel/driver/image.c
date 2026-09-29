#include <image.h>

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
    unsigned char magic[2];
    unsigned char mode;
    unsigned char charsize;
} psf_header;

long y = 0, x = 0;
psf_header* font;
unsigned int bg = _Crust, fg = _PureWhite;

void dsp_init(framebuf_info_t fb) {
    extern char _binary_ter_v16n_psf_start[];
    font = (psf_header*)_binary_ter_v16n_psf_start;

    unsigned int* dst = (unsigned int*)fb.base;
    unsigned int total = (fb.pitch / 4) * fb.height;
    for(int i = 0; i < total; ++i) {
        dst[i] = _Crust;
    }
}

void dsp_putchar(framebuf_info_t fb, char c) {
    unsigned int* dst = (unsigned int*)fb.base;

    if(c == '\n') {
        y += font->charsize;
        x = 0;
        return;
    }
    if(c == '\t') {
        x += 4;
        return;
    }

    unsigned char* font_glyphs = (unsigned char*)font + sizeof(psf_header);
    unsigned char* glyph = font_glyphs + ((unsigned char)c * font->charsize);
    int height = font->charsize;

    for(int i = 0; i < height; ++i) {
        unsigned char line = glyph[i];

        for(int j = 0; j < 8; ++j) {
            unsigned int color = (line & (0x80 >> j)) ? fg : bg;

            unsigned int screen_x = x + j;
            unsigned int screen_y = y + i;

            if(screen_x < fb.width && screen_y < fb.height) {
                dst[screen_y * (fb.pitch / 4) + screen_x] = color;
            }
        }
    }
    x += 8;
}

void dsp_print(framebuf_info_t fb, const char *str) {
    while(*str) {
        dsp_putchar(fb, *str);
        str++;
    }
}

void dsp_set_color(unsigned int col_fg, unsigned int col_bg) {
    fg = col_fg;
    bg = col_bg;
}