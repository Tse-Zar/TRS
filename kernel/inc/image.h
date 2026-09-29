#ifndef IMAGE_H
#define IMAGE_H

#include <bootinfo.h>

/*typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    float a;
} _rgba_color;
*/

#define _Rosewater 0xf5e0dc
#define _Flamingo 0xf2cdcd
#define _Pink 0xf5c2e7
#define _Mauve 0xcba6f7
#define _Red 0xf38ba8
#define _Maroon 0xeba0ac
#define _Peach 0xfab387
#define _Yellow 0xf9e2af
#define _Green 0xa6e3a1
#define _Teal 0x94e2d5
#define _Sky 0x89dceb
#define _Sapphire 0x74c7ec
#define _Blue 0x89b4fa
//Lavender 0xb4befe
//Text 0xcdd6f4
//Subtext1 0xbac2de
//Subtext0 0xa6adc8
//Overlay2 0x6c7086
//Overlay1 0x585b70
//verlay0 0x45475a
//Surface2 0x313244
//Surface1 0x43465e
//Surface0 0x313244
#define _Base 0x1e1e2e
#define _Mantle 0x181825
#define _Crust 0x11111b
#define _DarkerCrust 0x0b0b10
// BrightText 0xf5f5f5
#define _PureWhite 0xffffff
/*  NordPolar1 0x2e3440
    NordPolar2 0x3b4252
    NordPolar3 0x4c566a
*/
void dsp_init(framebuf_info_t fb);
void dsp_set_color(unsigned int col_fg, unsigned int col_bg);
void dsp_print(framebuf_info_t fb, const char* str);
void dsp_putchar(framebuf_info_t fb, char c);

#endif