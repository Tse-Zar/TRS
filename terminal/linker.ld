OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 0x0;
    __term_start = .;

    .text : {
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
    .data : {
        *(.data)
        *(.data.*)
        *(.data.rel.ro*)
    } :data

    . = ALIGN(8);
    .got : { *(.got) *(.got.plt) } :data
    .rela.dyn : { *(.rela.dyn) *(.rela.*) }

    __term_file_end = .;

    . = ALIGN(4K);
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    } :data

    . = ALIGN(4K);
    __term_end = .;

    __term_image_size = __term_file_end - __term_start;
    __term_mem_size   = __term_end - __term_start;

    /DISCARD/ : { *(.eh_frame*) *(.comment) *(.note*) }
}