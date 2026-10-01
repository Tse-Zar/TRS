#include <image.h>
#include <stdbool.h>

#define PSF2_MAGIC0 0x72
#define PSF2_MAGIC1 0xb5
#define PSF2_MAGIC2 0x4a
#define PSF2_MAGIC3 0x86

typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int headersize;
    unsigned int flags;
    unsigned int length;
    unsigned int charsize;
    unsigned int heigth;
    unsigned int width;
} __attribute__((packed)) psf_header;

long y = 0, x = 0;
psf_header* font;
unsigned int bg = _Crust, fg = _PureWhite;

void dsp_init(framebuf_info_t fb) {
    extern char _binary_ter_v32b_psf_start[];
    font = (psf_header*)_binary_ter_v32b_psf_start;

    unsigned int* dst = (unsigned int*)fb.base;
    unsigned int total = (fb.pitch / 4) * fb.height;
    for(int i = 0; i < total; ++i) {
        dst[i] = _Crust;
    }
}

void dsp_putchar(framebuf_info_t fb, char c) {
    unsigned int* dst = (unsigned int*)fb.base;
    bool is_b = false;

    if(c == '\n') {
        y += font->heigth;
        x = 0;
        return;
    }
    if(c == '\t') {
        x += font->width * 4;
        return;
    }
    if(c == '\b') {
        if(x >= font->width) x -= font->width;
        c = ' ';
        is_b = true;
    }
    unsigned char glyph = (unsigned char)c;

    if(glyph >= font->length) {
        glyph = '?';
    }

    unsigned char* font_glyphs = (unsigned char*)font + font->headersize;
    unsigned int bytes_per_line = font->charsize / font->heigth;
    unsigned char* glyph_data = font_glyphs + (glyph * font->charsize);

    for(int i = 0; i < font->heigth; ++i) {
        unsigned short line = 0;
        if (bytes_per_line == 1) {
            line = glyph_data[i];
        } else if (bytes_per_line == 2) {
            line = (glyph_data[i * 2] << 8) | glyph_data[i * 2 + 1];
        }

        for(int j = 0; j < font->width; ++j) {
            unsigned int color = (line & (0x8000 >> j)) ? fg : bg;

            unsigned int screen_x = x + j;
            unsigned int screen_y = y + i;

            if(screen_x < fb.width && screen_y < fb.height) {
                dst[screen_y * (fb.pitch / 4) + screen_x] = color;
            }
        }
    }
    if(!is_b) x += font->width;
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