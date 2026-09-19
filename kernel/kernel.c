/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = Tsezar Real Secuirity OS - Kernel  =
    = -------------------------- v 0.0.1 =
    ======================================
*/

#include <idt.h>
#include <keyboard.h>
#include <vga.h>
#include <memory_pagging.h>

void kmain(void) {
    /* inits */
    dsp_init();
    dsp_set_color(_LIGHT_GREEN, _BLACK);
    dsp_print("Display initialized. [+]\n");
    idt_init();
    dsp_print("IDT initialized [+]\n");
    kb_init();
    dsp_print("Keyboard intialized. [+]\n");
    pagging_init();
    dsp_print("RAM pagging initialized [+]\n");

    dsp_set_color(_LIGHT_MAGENTA, _BLACK);
    dsp_print("TRS OS v 0.0.2\n");
    dsp_print("==============\n\n");

    dsp_set_color(_CYAN, _BLACK);
    dsp_print("> [SYSTEM] ready.\n");

    dsp_print("START TERMINAL..");
    dsp_set_color(_LIGHT_GREY, _BLACK);
    //term_init();
    
    while(1) {
        __asm__ volatile ("hlt");
    }
}