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
    
    /* Объединяем заголовок и точку входа в ОДНУ секцию линкера */
    .text : {
        __text_start = .;
        *(.head)
        
        /* Принудительно выравниваем внутри секции на 64 байта, 
           чтобы убрать скрытые оптимизации линкера */
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
    
    /* Метка конца кода ДОЛЖНА быть строго ПОСЛЕ выравнивания на 4К, 
       чтобы размер в страницах включал в себя весь физический зазор */
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
        . = ALIGN(4K); /* Выравниваем конец BSS */
        __bss_end = .;
    } :data

    __kernel_end = .;

    /* Исправляем подсчет размеров */
    __hdr_image_size = __data_end - __kernel_start;
    __hdr_mem_pages  = (__kernel_end - __kernel_start) >> 12;
    __hdr_text_pages = (__text_end - __kernel_start) >> 12;
    __hdr_entry_off  = _start - __kernel_start;

    /DISCARD/ : { *(.eh_frame*) *(.comment) *(.note*) }
}