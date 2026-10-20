OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

PRO_BASE = 0x400000;

SECTIONS
{
    . = PRO_BASE;
    __term_start = .;

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

    __term_file_end = .;

    . = ALIGN(4K);
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }

    . = ALIGN(4K);
    __term_end = .;

    __term_image_size = __term_file_end - __term_start;
    __term_mem_size   = __term_end - __term_start;

    /DISCARD/ : { *(.eh_frame*) *(.comment) *(.note*) }
}