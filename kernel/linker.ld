OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

KERNEL_VBASE = 0xFFFFFFFF80000000;
SECTIONS
{
    . = KERNEL_VBASE;
    __kernel_start = .;

    .text : {
        __text_start = .;
        *(.head)

        . = ALIGN(64);
        *(.text.entry)

        *(.text)
        *(.text.*)
    } :text

    . = ALIGN(4K);
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } :text

    . = ALIGN(4K);
    __text_end = .;

    .data : {
        *(.data)
        *(.data.*)
        *(.data.rel.ro*)
    } :data

    . = ALIGN(4K);
    __data_end = .;

    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4K);
        __bss_end = .;
    } :data

    __kernel_end = .;

    __hdr_image_size = __data_end - __kernel_start;
    __hdr_mem_pages  = (__kernel_end - __kernel_start) >> 12;
    __hdr_text_pages = (__text_end - __kernel_start) >> 12;
    __hdr_entry_off  = _start - __kernel_start;

    /DISCARD/ : { *(.eh_frame*) *(.comment) *(.note*) }
}
