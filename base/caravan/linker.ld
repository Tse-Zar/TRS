OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

PRO_BASE = 0x500000;

SECTIONS
{
    . = PRO_BASE;
    __app_start = .;

    .text : {
        *(.text.entry)
        *(.text)
        *(.text.*)
    }

    . = ALIGN(4K);
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    . = ALIGN(4K);
    .data : {
        *(.data)
        *(.data.*)
        *(.data.rel.ro*)
    }

    __app_file_end = .;

    . = ALIGN(4K);
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }

    . = ALIGN(4K);
    __app_end = .;

    /DISCARD/ : { *(.eh_frame*) *(.comment) *(.note*) }
}