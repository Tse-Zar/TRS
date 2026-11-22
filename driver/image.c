#include <image.h>
#include <stdbool.h>
#include <string.h>
#include <colors.h>
#include <memory_paging.h>

#define PSF2_MAGIC 0x72B54A86

typedef struct {
    uint magic;
    uint version;
    uint headersize;
    uint flags;
    uint length;
    uint charsize;
    uint height;
    uint width;
} __attribute__((packed)) psf_header;

static long dsp_y = 0, dsp_x = 0;
static psf_header* font;
static uint bg = _Crust, fg = _PureWhite;
static framebuf_info_t fb;
static framebuf_info_t back;

void dsp_flip(void) {
    if(!back.base || !fb.base || back.base == fb.base) return;

    ullong bytes = fb.size;
    if(!bytes)
        bytes = (ullong)fb.pitch * fb.height;

    memcpy((void*)fb.base,(void*)back.base, bytes);
}

static void check_scroll_or_new_line(void) {
    if(dsp_x + font->width > back.width) {
        dsp_y += font->height;
        dsp_x = 0;
    }

    if(dsp_y + font->height > back.height) {
        uint* dst = (uint*)back.base;
        ullong total_fb = back.height * back.pitch / 4;
        uint line = font->height * back.pitch / 4;
        memmove(dst, ((uint*)back.base + line), (total_fb - line) * sizeof(uint));

        for(uint i = 1; i <= line; ++i) {
            dst[total_fb - i] = bg;
        }
        dsp_y -= font->height;
        dsp_x = 0;
    }
}

void dsp_clear(void) {
    uint* dst = (uint*)back.base;
    uint total = (back.pitch / 4) * back.height;
    for(uint i = 0; i < total; ++i) {
        dst[i] = bg;
    }

    dsp_x = 0;
    dsp_y = 0;

    dsp_flip();
}

void dsp_init(framebuf_info_t framebuffer) {
    extern char _binary_ter_v32b_psf_start[];
    font = (psf_header*)_binary_ter_v32b_psf_start;

    fb = framebuffer;
    back = framebuffer;

    ullong bytes = fb.size;
    if(!bytes)
        bytes = (ullong)fb.pitch * fb.height;

    back.base = (ullong)alloc_pages(bytes / PAGE_SIZE);
    if(!back.base) {
        back = fb;
    }

    dsp_clear();
}

/* display print decimal number */
void dsp_print_dec(int num) {
    uint k;
    if(num < 0) {
        dsp_putchar('-');
        k = (uint)-num;
    } else {
        k = (uint)num;
    }

    char buffer[64];

    int i = 0;
    do {
        char n = '0' + k % 10;
        buffer[i++] = n;
        k /= 10;
    } while(k > 0);

    while(i > 0) dsp_putchar(buffer[--i]);

    dsp_flip();

}

void dsp_putchar(char c) {
    uint* dst = (uint*)back.base;
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

    uchar glyph = (uchar)c;

    if(glyph >= font->length) {
        glyph = '?';
    }

    uchar* font_glyphs = (uchar*)font + font->headersize;
    uint bytes_per_line = font->charsize / font->height;
    uchar* glyph_data = font_glyphs + (glyph * font->charsize);

    for(uint i = 0; i < font->height; ++i) {
        ushort line = 0;
        if (bytes_per_line == 1) {
            line = glyph_data[i];
        } else if (bytes_per_line == 2) {
            line = (glyph_data[i * 2] << 8) | glyph_data[i * 2 + 1];
        }

        for(uint j = 0; j < font->width; ++j) {
            uint color = (line & (0x8000 >> j)) ? fg : bg;

            uint screen_x = dsp_x + j;
            uint screen_y = dsp_y + i;

            if(screen_x < back.width && screen_y < back.height) {
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

    dsp_flip();
}

void dsp_set_color(uint col_fg, uint col_bg) {
    uint fg_r = (col_fg & fb.rmask);
    uint fg_g = (col_fg & fb.gmask);
    uint fg_b = (col_fg & fb.bmask);

    uint bg_r = (col_bg & fb.rmask);
    uint bg_g = (col_bg & fb.gmask);
    uint bg_b = (col_bg & fb.bmask);

    fg = fg_r | fg_g | fg_b;
    bg = bg_r | bg_g | bg_b;
}

void dsp_print_hex(ullong v) {
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

void dsp_set_cursor(uint x, uint y) {
    dsp_x = x ? x * font->width : dsp_x;
    dsp_y = y ? y * font->height : dsp_y;
}
