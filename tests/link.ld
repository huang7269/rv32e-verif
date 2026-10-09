/* ================================================
 * rv32e 测试程序链接脚本
 * 内存布局与 dpi_mem/pmem.h 的 PMEM_SIZE=4MB 对应
 * ================================================ */
MEMORY {
    IMEM : ORIGIN = 0x00000000, LENGTH = 0x200000  /* 2MB 指令区 */
    DMEM : ORIGIN = 0x00200000, LENGTH = 0x200000  /* 2MB 数据区 */
}

ENTRY(_start)

SECTIONS {
    /* ---------- 指令区:只放代码 ---------- */
    .text : {
        *(.text)
        *(.text.*)
        . = ALIGN(4);
    } > IMEM

    /* ---------- 数据区:已初始化全局变量 ---------- */
    .data : {
        *(.data)
        *(.data.*)
        *(.sdata)          /* 小数据段,gcc 默认会把小变量放这里! */
        *(.sdata.*)
        . = ALIGN(4);
    } > DMEM

    /* ---------- 常量区:load 指令要读它,必须放数据区 ---------- */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
        *(.srodata)
        *(.srodata.*)
        . = ALIGN(4);
    } > DMEM

    /* ---------- 未初始化变量 ---------- */
    .bss : {
        _sbss = .;          /* bss 起始,供启动代码清零用 */
        *(.bss)
        *(.bss.*)
        *(.sbss)
        *(.sbss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;          /* bss 结束 */
    } > DMEM

    /* ---------- 丢弃不需要的段 ---------- */
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

/* 栈顶:从内存顶端向下生长(放 SECTIONS 外,PROVIDE 防重定义) */
PROVIDE(_stack_top = ORIGIN(DMEM) + LENGTH(DMEM));