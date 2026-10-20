void _start() {
    __asm__ volatile(
        "addi x10, x0, 0\n"    // x10=0
        "addi x11, x0, 10\n"   // x11=10
        "loop:\n"
        "jal x1, sub_func\n"
        "addi x10, x10, 1\n"
        "bne x10, x11, loop\n"
        "ebreak\n"
        "sub_func:\n"
        "nop\n"
        "jalr x0, x1, 0\n"
    );
}

