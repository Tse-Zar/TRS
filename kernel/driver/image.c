#include <image.h>
#include <stdbool.h>
#include <string.h>

#define PSF2_MAGIC0 0x72
#define PSF2_MAGIC1 0xb5
#define PSF2_MAGIC2 0x4a
#define PSF2_MAGIC3 0x86

static unsigned char back_buffer[3317760];

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
framebuf_info_t fb;

static void check_scroll_or_new_line(void) {
    if(x + font->width > fb.width) {
        y += font->heigth;
        x = 0;
    }

    if(y + font->heigth > fb.height) {
        unsigned int* dst = (unsigned int*)fb.base;
        unsigned long long total_fb = fb.height * fb.pitch / 4;
        unsigned int line = font->heigth * fb.pitch / 4;
        memmove(dst, ((unsigned int*)fb.base + line), (total_fb - line) * sizeof(unsigned int));
        
        for(int i = 1; i < line; ++i) {
            dst[total_fb - i] = bg;
        }
        y -= font->heigth;
        x = 0;
    }
}
void dsp_clear(void) {
    unsigned int* dst = (unsigned int*)fb.base;
    unsigned int total = (fb.pitch / 4) * fb.height;
    for(int i = 0; i < total; ++i) {
        back_buffer[i] = (unsigned char)_Crust;
    }
}

void dsp_init(framebuf_info_t framebuffer) {
    extern char _binary_ter_v32b_psf_start[];
    font = (psf_header*)_binary_ter_v32b_psf_start;
    fb = framebuffer;
    

    dsp_clear();    
}

void dsp_putchar(char c) {
    unsigned int* dst = (unsigned int*)fb.base;
    bool is_b = false;

    check_scroll_or_new_line();

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
    if(c == '\r') {
        x = 0;
        return;
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

void dsp_print(const char *str) {
    while(*str) {
        dsp_putchar(*str);
        str++;
    }
}

void dsp_set_color(unsigned int col_fg, unsigned int col_bg) {
    fg = col_fg;
    bg = col_bg;
}