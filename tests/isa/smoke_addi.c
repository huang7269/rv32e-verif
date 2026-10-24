// ============================================================
// 测试   : smoke_addi.c
// 覆盖   : testplan §ALU addi 全部测试点(正/负立即数、边界 +2047/-2048、
//          x0 作 rd/rs1)
// 协议   : a0(x10)=0 全通过;错误位:bit0 正立即数,bit1 负立即数,
//          bit2 立即数边界,bit3 x0 作 rd,bit4 x0 作 rs1
// 期望   : x10 0x0
// ============================================================
#include "macros.h"

void _start() __attribute__((noreturn));

void _start(){
    __asm__ volatile (
        "li a0, 0\n"

        TEST_GROUP(test_pos_imm,0)
        "addi x11, x0, 1\n"
        "addi x11, x11, 2\n"
        "addi x11, x11, 3\n"  CHECK_EQ(x11, 6, 0x1)

        TEST_GROUP(test_neg_imm,0)
        "addi x12, x0, -1\n"  CHECK_EQ(x12, 0xffffffff, 0x2)
        "addi x12, x12, 0\n"  CHECK_EQ(x12, 0xffffffff, 0x2)

        TEST_GROUP(test_boundary,0)
        "addi x13, x0, 2047\n"  CHECK_EQ(x13, 0x7ff, 0x4)
        "addi x13, x0, -2048\n" CHECK_EQ(x13, 0xfffff800, 0x4)

        TEST_GROUP(test_x0_rd,0)
        "addi x0, x0, 5\n"     CHECK_EQ(x0, 0, 0x8)

        TEST_GROUP(test_x0_rs1,0)
        "addi x14, x0, 7\n"    CHECK_EQ(x14, 7, 0x10)

        TEST_END()
        : : : "x11","x12","x13","x14","x15","t0","t1","t2","a0","memory"
    );
    __builtin_unreachable();
}
