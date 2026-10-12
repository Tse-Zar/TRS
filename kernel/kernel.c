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
#include <memory_paging.h>
#include <bootinfo.h>
#include <string.h>
#include <gdt.h>
boot_info_t _boot;

void kmain(boot_info_t* bi) {
    memcpy(&_boot, bi, sizeof(boot_info_t));
    
    /* --- inits --- */
    gdt_init();
    pic_remap(IRQ_BASE);
    idt_init();

    dsp_init(_boot.fb);
    dsp_set_color(_Sapphire, _Crust);
    dsp_print("Display initialized. ");
    dsp_set_color(_Peach, _Crust);
    dsp_print("[+]\n");

    dsp_set_color(_Sapphire, _Crust);
    dsp_print("GDT initalized. ");
    dsp_set_color(_Peach, _Crust);
    dsp_print("[+]\n");

    dsp_set_color(_Sapphire, _Crust);
    dsp_print("IDT initialized. ");
    dsp_set_color(_Peach, _Crust);
    dsp_print("[+]\n");

    dsp_set_color(_Sapphire, _Crust);
    dsp_print("PIC remaped. ");
    dsp_set_color(_Peach, _Crust);
    dsp_print("[+]\n");

    dsp_set_color(_Sapphire, _Crust);
    kb_init();
    dsp_print("Keyboard intialized. ");
    dsp_set_color(_Peach, _Crust);
    dsp_print("[+]\n");


    dsp_set_color(_Flamingo, _Crust);
    dsp_print("TRS OS v" __TRS_VERSION "\n");
    dsp_print("==============\n\n");

    dsp_set_color(_Green, _Crust);
    dsp_print("> [SYSTEM] ready.\n");
    
    #define PANIC_TEST 0

#if PANIC_TEST == 1
    __asm__ volatile("int3");                        /* Breakpoint, vector 3 */
#elif PANIC_TEST == 2
    __asm__ volatile(
    "xorl %%ecx, %%ecx\n\t"
    "movl $1, %%eax\n\t"
    "xorl %%edx, %%edx\n\t"
    "divl %%ecx\n\t"      
    ::: "eax", "ecx", "edx", "cc");
#elif PANIC_TEST == 3
    *(volatile unsigned long long*)0x100000012010ULL = 1;   /* 4 ГБ → #PF, err=2, cr2=0x100000000 */
#endif
    __asm__ volatile("sti");

    dsp_set_color(_Yantar, _Crust);
    //dsp_print("START TERMINAL..");
    //start_process("space/sys/term.pro")
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