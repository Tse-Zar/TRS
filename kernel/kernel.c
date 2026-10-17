/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = Tsezar Relaible System   - Kernel  =
    = -------------------------- v 0.0.4 =
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

static const char* statue[24] = {"    %%%", "   =====","  &%&%%%&", "  %& < <% ",
"   &\\__/","    \\ |____","   .', ,  ()","  / -.  _)| "," |_(_.    |",
" '-'\\  )  |"," trs )    |","    /  .  ).","   /    _. |"," /'---':.-'|",
"(__.' /    /"," \\   ( /  /","  \\ /  _  | ","   \\  |  '|","  | . \\  |",
"   |(     | ","   |  \\ \\ |","    \\  )\\ |","   __)/ / \\","--\"--(_.Ooo'----" };

#define PRINT_STATUE \
    dsp_set_color(_Red, _Crust); \
    dsp_print(statue[i++]); \
    dsp_set_cursor(30, 0); \
    dsp_set_color(_Sapphire, _Crust);

void kmain(boot_info_t* bi) {
    memcpy(&_boot, bi, sizeof(boot_info_t));
    
    /* --- inits --- */
    gdt_init();
    pic_remap(IRQ_BASE);
    idt_init();

    int i = 0;
    dsp_init(_boot.fb);
    PRINT_STATUE
    dsp_set_color(_Sapphire, _Crust);
    dsp_print("Display initialized. ");
    YES

    PRINT_STATUE
    dsp_print("GDT initalized. ");
    YES

    PRINT_STATUE
    dsp_print("IDT initialized. ");
    YES
    
    PRINT_STATUE
    dsp_print("PIC remaped. ");
    YES

    PRINT_STATUE
    kb_init();
    dsp_print("Keyboard intialized. ");
    YES

    PRINT_STATUE
    pmm_init(&_boot);
    dsp_print("Memory initialized. ");
    YES

    PRINT_STATUE
    disk_init();
    dsp_print("Disk initialized. ");
    YES

    PRINT_STATUE
    dsp_print("\n");
    PRINT_STATUE
    dsp_set_color(_Lavender, _Crust);
    dsp_print("System info:\n");
    PRINT_STATUE
    dsp_set_color(_Lavender, _Crust);
    dsp_print("----");
    int ferr = fat32_init(0);
    dsp_print("\n");
    PRINT_STATUE
    dsp_set_color(_Lavender, _Crust);
    dsp_print("FAT32 init: ");
    dsp_print(ferr ? fat_get_error(ferr) : "");
    YES

    if(ferr == FAT_ERR_NONE) {
        DirInfo info;
        if(fat_stat("/sys/space/sys/kern/kernel.bin", &info) == FAT_ERR_NONE) {
            PRINT_STATUE
            dsp_set_color(_Lavender, _Crust);
            dsp_print("kernel.bin size: ");
            dsp_print_dec((int)info.size);
            dsp_print("\n");
            PRINT_STATUE
        } else {
            kpanic("NO DISK", __FILE__, __LINE__);
        }
    }
    dsp_print("\n");
    PRINT_STATUE
    dsp_set_color(_Lavender, _Crust);
    dsp_print("usable pages: ");
    dsp_print_dec(pmm_usable_pages());
    dsp_print("\n");
    PRINT_STATUE
    dsp_set_color(_Lavender, _Crust);
    dsp_print("used pages: ");
    dsp_print_dec(pmm_used_pages());

    dsp_print("\n");
    PRINT_STATUE
    dsp_set_color(_Lavender, _Crust);
    dsp_print("\n");
    PRINT_STATUE
    dsp_set_color(_Lavender, _Crust);
    dsp_print("TRS OS v" __TRS_VERSION "\n");
    PRINT_STATUE
    dsp_set_color(_Lavender, _Crust);
    dsp_print("==============\n\n");

    dsp_set_color(_Yantar, _Crust);
    dsp_print("> [SYSTEM] ready.\n\n");

    __asm__ volatile("sti");


    dsp_set_color(_Yantar, _Crust);
    dsp_print("START TERMINAL..");
    //start_process("space/sys/term.pro")
    while(1) {
        __asm__ volatile ("sti; hlt");
    }
}