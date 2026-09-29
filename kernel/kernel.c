/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = Tsezar Real Secuirity OS - Kernel  =
    = -------------------------- v 0.0.3 =
    ======================================
*/

#include <pic.h>
#include <stream.h>
#include <version.h>
#include <idt.h>
#include <keyboard.h>
#include <image.h>
#include <memory_pagging.h>
#include <bootinfo.h>
#include <string.h>

void kmain(boot_info_t* bi) {
    boot_info_t _boot; 
    memcpy(&_boot, bi, sizeof(boot_info_t));
    
    /* --- inits --- */
    dsp_init(_boot.fb);
    dsp_set_color(_Sapphire, _Crust);
    dsp_print(_boot.fb, "Display initialized. [+]\n");

    pic_remap(IRQ_BASE);

    idt_init();
    dsp_print(_boot.fb, "IDT initialized [+]\n");
    //kb_init();
    //dsp_print("Keyboard intialized. [+]\n");
    //pagging_init();
    //dsp_print("RAM pagging initialized [+]\n");

    dsp_set_color(_Flamingo, _Crust);
    dsp_print(_boot.fb, "TRS OS v" __TRS_VERSION "\n");
    dsp_print(_boot.fb, "==============\n\n");

    dsp_set_color(_Green, _Crust);
    dsp_print(_boot.fb, "> [SYSTEM] ready.\n");

    //dsp_print("START TERMINAL..");
    //dsp_set_color();
    //term_init();
    
    //stream_t* kb = kbd_get_stream();
    char c;
    while(1) {
        //if(stream_try_read(kb, &c, 1) == 1) dsp_putchar(c);
        __asm__ volatile ("hlt");
    }
}