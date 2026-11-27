/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = Tsezar Reliable System   - Kernel  =
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
#include <sys.h>
#include <app.h>
#include <security.h>
#include <timer.h>
#include <thread.h>
#include <types.h>

boot_info_t _boot;

#define YES \
    dsp_set_color(_Peach, _Crust); \
    dsp_print("[+]\n");

static const char* statue[24] = {"    %%%", "   =====","  &%&%%%&", "  %& < <% ",
"   &\\__/","    \\ |____","   .', ,  ()","  / -.  _)| "," |_(_.    |",
" '-'\\  )  |"," trs )    |","    /  .  ).","   /    _. |"," /'---':.-'|",
"(__.' /    /"," \\   ( /  /","  \\ /  _  | ","   \\  |  '|","   | . \\  |",
"   |(     | ","   |  \\ \\ |","    \\  )\\ |","   __)/ / \\","--\"--(_.Ooo'----" };

#define PRINT_STATUE \
    if(part_of_statue < 24) { \
        dsp_set_color(_Red, _Crust); \
        dsp_print(statue[part_of_statue++]); \
        dsp_set_cursor(30, 0); \
        dsp_set_color(_Sapphire, _Crust); \
    }

#define DBG_PORT 0x3F8U
static void dbg(char c) {
    __asm__ volatile("outb %0, %1" :: "a"(c), "d"((ushort)DBG_PORT));
}

static void kernel_idle(void) {
    while(1) __asm__ volatile("sti; hlt");
}

void kmain(boot_info_t* bi) {
    dbg('K');
    memcpy(&_boot, bi, sizeof(boot_info_t));

    /* --- pre-work inits --- */
    gdt_init();
    pic_remap(IRQ_BASE);
    idt_init();
    timer_init(TIMER_HZ);
    pmm_init(&_boot);
    int part_of_statue = 0;

    /* --- display init ---
     * use GOP(UEFI) framebuffer & backbuffer
     * flip after every dsp_print & dsp_print_dec
     * now flip all buffer maybe can flip only the modified ones
    */
    dsp_init(_boot.fb);
    PRINT_STATUE
    dsp_set_color(_Sapphire, _Crust); // main kernel-cli color
    dsp_print("Display initialized. ");
    YES

    PRINT_STATUE
    dsp_print("GDT initialized. ");
    YES

    PRINT_STATUE
    dsp_print("IDT initialized. ");
    YES

    PRINT_STATUE
    dsp_print("TIMER initialized. ");
    YES

    PRINT_STATUE
    dsp_print("PIC remapped. ");
    YES

    PRINT_STATUE
    dsp_print("Memory initialized. ");
    YES

    PRINT_STATUE
    kb_init();
    dsp_print("Keyboard initialized. ");
    YES

    PRINT_STATUE
    disk_init();
    dsp_print("Disk initialized. ");
    YES

    PRINT_STATUE
    syscall_init();
    dsp_print("Syscall initialized. ");
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
        if(fat_stat("/dsk/space/sys/kern/kernel.bin", &info) == FAT_ERR_NONE) {
            PRINT_STATUE
            dsp_set_color(_Lavender, _Crust);
            dsp_print("kernel.bin size: ");
            dsp_print_dec(info.size);
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
    dsp_print("START TERMINAL..\n");
    dsp_print("Press any key for work!");

    stream_t* kb = kbd_get_stream();
    char c = 0;
    while(1) {
        if(stream_try_read(kb, &c, 1)) break;
        __asm__ volatile("hlt");
    }

    dsp_set_color(_Yantar, _TermBG);
    dsp_clear();

    //thread_create((ullong)kernel_idle, 0, 0, 0);

    start_proc("/dsk/space/sys/term.pro");
   // sched_start();
}
