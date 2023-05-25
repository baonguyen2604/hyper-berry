

ENTRY(_start)

SECTIONS
{
    /* Kernel load address for AArch64 by RPi bootloader */
    . = 0x0000008000280000; /* set current address to 0x80000 */
    __start = .; /* set __start to current address */
    __text_start = .; /* set __text_start to current address */
    .text : { 
        KEEP(*(.text.boot)) 
        *(.text .text.*) 
    }
    . = ALIGN(4096); /* align current address to page boundary */
    __text_end = .; /* set __text_end to current address */

    __rodata_start = .;
    .rodata : { *(.rodata .rodata.*) }
    __rodata_end = .;

    __data_start = .;
    PROVIDE(_data = .);
    .data : { 
        *(.data.page_aligned)
        . = ALIGN(8);
        *(.data .data.*)
    }
    __data_end = .;

    . = ALIGN(4096); /* align to page size */
    __init_begin = .;
    .init.text : {
        *(.init.text);
        . = ALIGN(4096);
    } :text
    . = ALIGN(4096);
    .init.data : {
        *(.init.data)
    } :text
    
    __init_end = .;
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    . = ALIGN(4096); /* align to page size */
    _end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
