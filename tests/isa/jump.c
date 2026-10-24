// ============================================================
// 测试   : jump.c
// 覆盖   : testplan §第 4 周跳转全部测试点(jal 链接/负偏移/x0 作 rd,
//          jalr 基本跳转/奇数地址对齐/链接/立即数偏移)
// 协议   : a0(x10)=0 全通过;错误位:bit0 jal 链接,bit1 jal x0 作 rd,
//          bit2 jal 负偏移,bit3 jalr 奇数对齐,bit4 jalr 链接,
//          bit5 jalr 立即数偏移
// 期望   : x10 0x0
// 说明   : 链接寄存器用 x11(测试约定不占用 ra=x1)
// ============================================================
#include "macros.h"

void _start() __attribute__((noreturn));

void _start(){
    __asm__ volatile (
        "li a0, 0\n"

        // ---------- jal 前向跳 + 链接值校验 ----------
        // 链接值 = jal 地址 + 4 = 紧随其后那条指令的地址,故目标必须是下一条
        // 跳转本身是否发生由 test_jal_x0 组覆盖
        TEST_GROUP(test_jal_link,0)
        "jal x11, 1f\n"                     // x11 = 下一条指令(1:)的地址
        "1:\n"
        "la x12, 1b\n"                      // x12 = 1: 的地址
        "beq x11, x12, 2f\n"                // 链接值 == 目标地址
        "ori a0, a0, 0x1\n"
        "2:\n"

        // ---------- jal x0 作 rd(丢弃链接) ----------
        TEST_GROUP(test_jal_x0,0)
        "jal x0, 1f\n"
        "ori a0, a0, 0x2\n"                 // 错误:没跳
        "1:\n"

        // ---------- jal 负偏移(向后跳) ----------
        TEST_GROUP(test_jal_back,0)
        "li x13, 0\n"
        "li x12, 3\n"
        "1:\n"
        "addi x13, x13, 1\n"
        "beq x13, x12, 2f\n"
        "jal x0, 1b\n"                      // jal 向后跳
        "2:\n"
        CHECK_EQ(x13, 3, 0x4)

        // ---------- jalr 奇数地址对齐 ----------
        TEST_GROUP(test_jalr_align,0)
        "la x11, 1f\n"                      // x11 = 1: 地址(偶数)
        "addi x12, x11, 1\n"                // x12 = 奇数地址
        "jalr x0, x12, 0\n"                 // 应跳 (x12&~1) = 1:
        "ori a0, a0, 0x8\n"                 // 错误:未跳到对齐地址
        "1:\n"

        // ---------- jalr 链接值校验 ----------
        // 链接值 = jalr 地址 + 4,目标必须是紧随其后的指令
        TEST_GROUP(test_jalr_link,0)
        "la x11, 1f\n"                      // x11 = 下一条指令(1:)的地址
        "jalr x13, x11, 0\n"                // x13 = 1: 的地址,跳 1:
        "1:\n"
        "la x12, 1b\n"
        "beq x13, x12, 2f\n"
        "ori a0, a0, 0x10\n"
        "2:\n"

        // ---------- jalr 立即数偏移(落点精确区分) ----------
        TEST_GROUP(test_jalr_imm,0)
        "li x13, 0\n"
        "la x11, 1f\n"
        "jalr x0, x11, 8\n"                 // 目标 = 1:+8
        "ori a0, a0, 0x20\n"                // 错误:未跳
        "j 2f\n"
        "1:\n"
        "j 2f\n"                            // 1:+0 错误落点(marker 不变)
        "j 2f\n"                            // 1:+4 错误落点
        "addi x13, x13, 1\n"                // 1:+8 正确落点 ← 标记
        "2:\n"
        CHECK_EQ(x13, 1, 0x20)

        TEST_END()
        : : : "x11","x12","x13","x14","x15","t0","t1","t2","a0","memory"
    );
    __builtin_unreachable();
}
