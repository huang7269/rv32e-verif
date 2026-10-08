MEMORY {
    IMEM : ORIGIN = 0x00000000, LENGTH = 0x400000  /* 指令存储器 4MB */
    DMEM : ORIGIN = 0x00000000, LENGTH = 0x400000  /* 数据存储器 4MB，独立编址 */
}

SECTIONS {
    .text : {
        *(.text)
        *(.text.*)
        *(.rodata)
        . = ALIGN(4);
    } > IMEM

    .data : {
        *(.data)
        *(.data.*)
        . = ALIGN(4);
    } > DMEM

    .bss : {
        *(.bss)
        *(.bss.*)
        . = ALIGN(4);
    } > DMEM

    _stack_top = ORIGIN(DMEM) + LENGTH(DMEM);

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame)
        *(.phdr)
        *(.dynamic)
        *(.dynsym)
        *(.dynstr)
    }
}
