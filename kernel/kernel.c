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
#include <colors.h>
#include <gpt.h>
#include <fat32.h>
#include <disk.h>
#include <panic.h>

boot_info_t _boot;

#define YES \
    dsp_set_color(_Peach, _Crust); \
    dsp_print("[+]\n"); \
    dsp_set_color(_Sapphire, _Crust);

void kmain(boot_info_t* bi) {
    memcpy(&_boot, bi, sizeof(boot_info_t));
    
    /* --- inits --- */
    gdt_init();
    pic_remap(IRQ_BASE);
    idt_init();

    dsp_init(_boot.fb);
    dsp_set_color(_Sapphire, _Crust);
    dsp_print("Display initialized. ");
    YES

    dsp_print("GDT initalized. ");
    YES

    dsp_print("IDT initialized. ");
    YES

    dsp_print("PIC remaped. ");
    YES

    kb_init();
    dsp_print("Keyboard intialized. ");
    YES

    pmm_init(&_boot);
    dsp_print("Memory initialized. ");
    YES

    disk_init();
    dsp_print("Disk initialized. ");
    YES
    
    dsp_set_color(_Lavender, _Crust);
    dsp_print("\n\nSystem info:\n----\n");
    int ferr = fat32_init(0);
    dsp_print("FAT32 init: ");
    dsp_print(fat_get_error(ferr));

    if(ferr == FAT_ERR_NONE) {
        DirInfo info;
        if(fat_stat("/sys/space/sys/kern/kernel.bin", &info) == FAT_ERR_NONE) {
            dsp_print("\nkernel.bin size: ");
            dsp_print_dec((int)info.size);
            dsp_print("\n");
        } else {
            kpanic("NO DISK", __FILE__, __LINE__);
        }
    }
    dsp_print("usable pages: ");
    dsp_print_dec(pmm_usable_pages());
    dsp_print("\nused pages: ");
    dsp_print_dec(pmm_used_pages());

    dsp_print("\nTRS OS v" __TRS_VERSION "\n");
    dsp_print("==============\n\n");

    dsp_set_color(_Yantar, _Crust);
    dsp_print("> [SYSTEM] ready.\n\n");

    dsp_print("       .-.-.\n" "  ((  (__I__)  ))\n" "    .'_....._'.\n" "   / / .12 . \\ \\\n" "  | | '  |  ' | |\n" "  | | 9  /  3 | |\n");
    dsp_print("   \\ \\ '.6.' / /\n" "    '.`-...-'.'\n" "     /'-- --'\\\n" "    `\"\"\"\"\"\"\"\"\"` \n");
    
    #define PANIC_TEST 0

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