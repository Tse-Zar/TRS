/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = Tsezar Relaible System   - Kernel  =
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

boot_info_t _boot;

void kmain(boot_info_t* bi) {
    memcpy(&_boot, bi, sizeof(boot_info_t));
    
    /* --- inits --- */
    dsp_init(_boot.fb);
    dsp_set_color(_Sapphire, _Crust);
    dsp_print("Display initialized. [+]\n");

    pic_remap(IRQ_BASE);

    idt_init();
    dsp_print("IDT initialized [+]\n");
    kb_init();
    dsp_print("Keyboard intialized. [+]\n");
    //pagging_init();
    //dsp_print("RAM pagging initialized [+]\n");

    dsp_set_color(_Flamingo, _Crust);
    dsp_print("TRS OS v" __TRS_VERSION "\n");
    dsp_print("==============\n\n");

    dsp_set_color(_Green, _Crust);
    dsp_print("> [SYSTEM] ready.\n");

    dsp_set_color(_Yantar, _Crust);
    //dsp_print("START TERMINAL..");
    
    stream_t* kb = kbd_get_stream();
    char c;
    while(1) {
        __asm__ volatile ("cli");
        if(stream_try_read(kb, &c, 1) == 1) { 
            __asm__ volatile("sti");
            dsp_putchar(c);
            continue;
        }
        __asm__ volatile ("sti; hlt");
    }
}