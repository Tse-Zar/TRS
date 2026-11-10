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
    __app_start = .;

    
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

    __app_file_end = .;

    . = ALIGN(4K);
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    } :data

    . = ALIGN(4K);
    __app_end = .;

    /DISCARD/ : { *(.eh_frame*) *(.comment) *(.note*) }
}