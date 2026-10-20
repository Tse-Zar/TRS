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
    . = 0x100000;
    __kernel_start = .;
    
    .text : {
        *(.text.entry)
    } :text

    .text_rest : {
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

    . = ALIGN(4K);
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    } :data

    __kernel_end = .;

    /DISCARD/ : { *(.eh_frame*) *(.comment) *(.note*) }
}