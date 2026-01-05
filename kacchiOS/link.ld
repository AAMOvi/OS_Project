/* link.ld - Linker script */
OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

PHDRS
{
    text PT_LOAD FLAGS(0x5); /* R X */
    data PT_LOAD FLAGS(0x6); /* R W */
}

SECTIONS {
    . = 1M;
    
    .text : {
        *(.multiboot)
        *(.text*)
        *(.rodata*)
    } :text
    
    .data : {
        *(.data*)
    } :data
    
    .bss : {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    } :data
    
    /* Future: Students will use memory beyond this point */
    . = ALIGN(4096);
    __kernel_end = .;
}