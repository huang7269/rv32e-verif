void _start(){
    __asm__ volatile(
    /* ========== 1. beq: x1 == x2 跳 ========== */
    "addi x1, x0, 10\n"
    "addi x2, x0, 10\n"
    "beq x1, x2, 1f\n"
    "ebreak\n"
    "1:\n"
    "addi x10, x0, 1\n"

    /* ========== 2. bne: x1 != x2 跳 ========== */
    "addi x1, x0, 20\n"
    "addi x2, x0, 21\n"
    "bne x1, x2, 2f\n"
    "ebreak\n"
    "2:\n"
    "addi x11, x0, 1\n"

    /* ========== 3. blt (有符号): x1 < x2 跳 ========== */
    "addi x1, x0, -5\n"
    "addi x2, x0, 3\n"
    "blt x1, x2, 3f\n"
    "ebreak\n"
    "3:\n"
    "addi x12, x0, 1\n"

    /* ========== 4. bge (有符号): x1 >= x2 跳 ========== */
    "addi x1, x0, 100\n"
    "addi x2, x0, 50\n"
    "bge x1, x2, 4f\n"
    "ebreak\n"
    "4:\n"
    "addi x13, x0, 1\n"

    /* ========== 5. bltu (无符号): x1 < 0x80000000 跳 ========== */
    "addi x1, x0, 1\n"
    "lui x2, 0x80000\n"          // x2 = 0x80000000
    "bltu x1, x2, 5f\n"
    "ebreak\n"
    "5:\n"
    "addi x14, x0, 1\n"

    /* ========== 6. bgeu (无符号): 0xffffffff >= 0x7fffffff 跳 ========== */
    "lui x1, 0xfffff\n"          // x1 = 0xffffffff
    "lui x2, 0x7ffff\n"          // x2 = 0x7ffff000
    "bgeu x1, x2, 6f\n"
    "ebreak\n"
    "6:\n"
    "addi x15, x0, 1\n"

    /* ========== 额外补充一组【不跳转用例】方便测反逻辑 ========== */
    "addi x1, x0, 5\n"
    "addi x2, x0, 5\n"
    "blt x1, x2, 7f\n"
    "ebreak\n"                   // ✅ 预期走到这里
    "7:\n"
    "addi x9, x0, 1\n"
    );
}

