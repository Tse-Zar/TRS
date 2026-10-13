#include <image.h>
#include <stdbool.h>
#include <string.h>

#define PSF2_MAGIC 0x72B54A86

typedef struct {
    unsigned int magic;
    unsigned int version;
    unsigned int headersize;
    unsigned int flags;
    unsigned int length;
    unsigned int charsize;
    unsigned int height;
    unsigned int width;
} __attribute__((packed)) psf_header;

static long dsp_y = 0, dsp_x = 0;
static psf_header* font;
static unsigned int bg = _Crust, fg = _PureWhite;
static framebuf_info_t fb;

static void check_scroll_or_new_line(void) {
    if(dsp_x + font->width > fb.width) {
        dsp_y += font->height;
        dsp_x = 0;
    }

    if(dsp_y + font->height > fb.height) {
        unsigned int* dst = (unsigned int*)fb.base;
        unsigned long long total_fb = fb.height * fb.pitch / 4;
        unsigned int line = font->height * fb.pitch / 4;
        memmove(dst, ((unsigned int*)fb.base + line), (total_fb - line) * sizeof(unsigned int));
        
        for(unsigned int i = 1; i <= line; ++i) {
            dst[total_fb - i] = bg;
        }
        dsp_y -= font->height;
        dsp_x = 0;
    }
}
void dsp_clear(void) {
    unsigned int* dst = (unsigned int*)fb.base;
    unsigned int total = (fb.pitch / 4) * fb.height;
    for(unsigned int i = 0; i < total; ++i) {
        dst[i] = bg;
    }

    dsp_x = 0;
    dsp_y = 0;
}

void dsp_init(framebuf_info_t framebuffer) {
    extern char _binary_ter_v32b_psf_start[];
    font = (psf_header*)_binary_ter_v32b_psf_start;
    //if(font->magic != PSF2_MAGIC) return; 
    fb = framebuffer;

    dsp_clear();    
}

void dsp_putchar(char c) {
    unsigned int* dst = (unsigned int*)fb.base;
    bool is_b = false;

    check_scroll_or_new_line();

    if(c == '\n') {
        dsp_y += font->height;
        dsp_x = 0;
        return;
    }
    if(c == '\t') {
        dsp_x += font->width * 4;
        return;
    }
    if(c == '\b') {
        if(dsp_x >= font->width) dsp_x -= font->width;
        c = ' ';
        is_b = true;
    }
    if(c == '\r') {
        dsp_x = 0;
        return;
    }

    unsigned char glyph = (unsigned char)c;

    if(glyph >= font->length) {
        glyph = '?';
    }

    unsigned char* font_glyphs = (unsigned char*)font + font->headersize;
    unsigned int bytes_per_line = font->charsize / font->height;
    unsigned char* glyph_data = font_glyphs + (glyph * font->charsize);

    for(unsigned int i = 0; i < font->height; ++i) {
        unsigned short line = 0;
        if (bytes_per_line == 1) {
            line = glyph_data[i];
        } else if (bytes_per_line == 2) {
            line = (glyph_data[i * 2] << 8) | glyph_data[i * 2 + 1];
        }

        for(unsigned int j = 0; j < font->width; ++j) {
            unsigned int color = (line & (0x8000 >> j)) ? fg : bg;

            unsigned int screen_x = dsp_x + j;
            unsigned int screen_y = dsp_y + i;

            if(screen_x < fb.width && screen_y < fb.height) {
                dst[screen_y * (fb.pitch / 4) + screen_x] = color;
            }
        }
    }
    if(!is_b) dsp_x += font->width;
}

void dsp_print(const char *str) {
    while(*str) {
        dsp_putchar(*str);
        str++;
    }
}

void dsp_set_color(unsigned int col_fg, unsigned int col_bg) {
    unsigned int fg_r = (col_fg & fb.rmask);
    unsigned int fg_g = (col_fg & fb.gmask);
    unsigned int fg_b = (col_fg & fb.bmask);

    unsigned int bg_r = (col_bg & fb.rmask);
    unsigned int bg_g = (col_bg & fb.gmask);
    unsigned int bg_b = (col_bg & fb.bmask);

    fg = fg_r | fg_g | fg_b;
    bg = bg_r | bg_g | bg_b;
}

void dsp_print_hex(unsigned long long v) {
    char buf[19];
    const char* hex = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for(int i = 0; i < 16; ++i) {
        buf[2 + i] = hex[(v >> (60 - i * 4)) & 0xF];
    }
    buf[18] = '\0';
    dsp_print(buf);
}