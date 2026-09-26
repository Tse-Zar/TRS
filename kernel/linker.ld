ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    __kernel_start = .;
    
    .text : { *(.text.entry) *(.text) *(.text.*) }
    .rodata : { *(.rodata) *(.rodata.*) }
    .data : { *(.data) *(.data.*) *(.data.rel.ro*) }

    .bss : {
        __bss_start = .;
        *(.bss) *(.bss.*) *(COMMON)
        __bss_end = .;
    }

    __kernel_end = .;

    /DISCARD/ : { *(.eh_frame*) *(.comment) *(.note*) }
}