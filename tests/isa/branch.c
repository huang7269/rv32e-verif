// ============================================================
// 测试   : branch.c
// 覆盖   : testplan §第 4 周分支全部测试点(6 条指令跳/不跳、负偏移、
//          负数比较、相等比较、x0 与 x0 恒跳)
// 协议   : a0(x10)=0 全通过;错误位:bit0 beq 跳,bit1 beq 不跳,
//          bit2 bne,bit3 负偏移,bit4 beq 负偏移+x0x0 恒跳,
//          bit5 blt,bit6 bge,bit7 bltu,bit8 bgeu
// 期望   : x10 0x0
// ============================================================
#include "macros.h"

void _start() __attribute__((noreturn));

void _start(){
    __asm__ volatile (
        "li a0, 0\n"

        // ---------- beq 跳(相等) ----------
        TEST_GROUP(test_beq_taken,0)
        "li x11, 10\n"
        "li x12, 10\n"
        "beq x11, x12, 1f\n"
        "ori a0, a0, 0x1\n"                 // 错误:该跳没跳
        "1:\n"

        // ---------- beq 不跳(不等) ----------
        TEST_GROUP(test_beq_not,0)
        "li x11, 10\n"
        "li x12, 11\n"
        "beq x11, x12, 1f\n"
        "j 2f\n"                            // 正确:顺序执行,跳过错误区
        "1:\n"
        "ori a0, a0, 0x2\n"                 // 错误:不该跳
        "2:\n"

        // ---------- bne 跳 / 不跳 ----------
        TEST_GROUP(test_bne_taken,0)
        "li x11, 20\n"
        "li x12, 21\n"
        "bne x11, x12, 1f\n"
        "ori a0, a0, 0x4\n"
        "1:\n"

        TEST_GROUP(test_bne_not,0)
        "li x11, 5\n"
        "li x12, 5\n"
        "bne x11, x12, 1f\n"
        "j 2f\n"
        "1:\n"
        "ori a0, a0, 0x4\n"
        "2:\n"

        // ---------- 负偏移(向后跳)循环 ----------
        TEST_GROUP(test_bne_back,0)
        "li x11, 0\n"
        "li x12, 3\n"
        "1:\n"
        "addi x11, x11, 1\n"
        "bne x11, x12, 1b\n"                // 向后跳,循环 3 次
        CHECK_EQ(x11, 3, 0x8)

        // ---------- beq 负偏移 + beq x0,x0 恒跳 ----------
        TEST_GROUP(test_beq_back,0)
        "li x11, 0\n"
        "li x12, 3\n"
        "2:\n"
        "beq x11, x12, 3f\n"                // 前向退出
        "addi x11, x11, 1\n"
        "beq x0, x0, 2b\n"                  // beq 负偏移,恒跳(x0==x0)
        "3:\n"
        CHECK_EQ(x11, 3, 0x10)

        // ---------- blt:正负/负数/相等 ----------
        TEST_GROUP(test_blt,0)
        "li x11, -5\n"
        "li x12, 3\n"
        "blt x11, x12, 1f\n"                // -5<3 跳
        "ori a0, a0, 0x20\n"
        "1:\n"
        "li x11, -5\n"
        "li x12, -1\n"
        "blt x11, x12, 1f\n"                // -5<-1 跳(负数比较)
        "ori a0, a0, 0x20\n"
        "1:\n"
        "li x11, 5\n"
        "li x12, 5\n"
        "blt x11, x12, 1f\n"                // 5<5 假,不跳
        "j 2f\n"
        "1:\n"
        "ori a0, a0, 0x20\n"
        "2:\n"

        // ---------- bge:正数/负数/相等 ----------
        TEST_GROUP(test_bge,0)
        "li x11, 100\n"
        "li x12, 50\n"
        "bge x11, x12, 1f\n"                // 100>=50 跳
        "ori a0, a0, 0x40\n"
        "1:\n"
        "li x11, -1\n"
        "li x12, -5\n"
        "bge x11, x12, 1f\n"                // -1>=-5 跳(负数比较)
        "ori a0, a0, 0x40\n"
        "1:\n"
        "li x11, 5\n"
        "li x12, 5\n"
        "bge x11, x12, 1f\n"                // 5>=5 跳(>= 含相等)
        "ori a0, a0, 0x40\n"
        "1:\n"

        // ---------- bltu:无符号/相等 ----------
        TEST_GROUP(test_bltu,0)
        "li x11, 1\n"
        "lui x12, 0x80000\n"                // x12 = 0x80000000
        "bltu x11, x12, 1f\n"               // 无符号 1<0x80000000 跳
        "ori a0, a0, 0x80\n"
        "1:\n"
        "li x11, 5\n"
        "li x12, 5\n"
        "bltu x11, x12, 1f\n"               // 相等不跳
        "j 2f\n"
        "1:\n"
        "ori a0, a0, 0x80\n"
        "2:\n"

        // ---------- bgeu:无符号/相等 ----------
        TEST_GROUP(test_bgeu,0)
        "lui x11, 0xfffff\n"                // x11 = 0xfffff000
        "lui x12, 0x7ffff\n"                // x12 = 0x7ffff000
        "bgeu x11, x12, 1f\n"               // 0xfffff000>=0x7ffff000 跳
        "ori a0, a0, 0x100\n"
        "1:\n"
        "li x11, 5\n"
        "li x12, 5\n"
        "bgeu x11, x12, 1f\n"               // 相等:>= 成立,跳
        "ori a0, a0, 0x100\n"
        "1:\n"

        TEST_END()
        : : : "x11","x12","x13","x14","x15","t0","t1","t2","a0","memory"
    );
    __builtin_unreachable();
}
