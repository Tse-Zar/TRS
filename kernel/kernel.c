/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = Tsezar Real Secuirity OS - Kernel  =
    = -------------------------- v 0.0.1 =
    ======================================
*/

#include "idt.h"
#include <keyboard.h>
#include <vga.h>
#include <memory_pagging.h>

void kmain(void) {
    /* inits */
    dsp_init();
    dsp_set_color(_LIGHT_RED, _BLACK);
    dsp_print("Display initialized. [+]");
    idt_init();
    kb_init();
    dsp_print("Keyboard intialized. [+]");

    dsp_print("TRS OS v 0.0.1\n");
    dsp_print("==============\n\n");

    dsp_set_color(_LIGHT_GREEN, _BLACK);
    dsp_print("> [SYSTEM] ready.\n");
    dsp_set_color(_LIGHT_GREY, _BLACK);

    dsp_print("START TERMINAL..");
    //term_init();
    
    while(1) {
        __asm__ volatile ("hlt");
    }
}