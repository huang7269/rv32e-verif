// ============================================================
// 测试   : alu_itype.c
// 覆盖   : testplan §ALU I 型全部测试点(8 条指令、-1 符号扩展、
//          srai 的 funct7 混入立即数、立即数边界 +2047/-2048、
//          移位量 0/31)
// 协议   : a0(x10)=0 全通过;错误位:bit0 基本运算,bit1 移位,
//          bit2 立即数边界,bit3 移位边界
// 期望   : x10 0x0
// ============================================================
#include "macros.h"

void _start() __attribute__((noreturn));

void _start(){
    __asm__ volatile (
        "li a0, 0\n"

        TEST_GROUP(test_itype_basic,0)
        "li x11, 4\n"
        "slti x13, x11, 5\n"   CHECK_EQ(x13, 1, 0x1)             // 4<5
        "slti x13, x11, -1\n"  CHECK_EQ(x13, 0, 0x1)             // 4<-1 假
        "sltiu x13, x11, -1\n" CHECK_EQ(x13, 1, 0x1)             // 无符号 4<0xffffffff
        "xori x13, x11, -1\n"  CHECK_EQ(x13, 0xfffffffb, 0x1)    // 4^0xffffffff
        "ori x13, x11, -1\n"   CHECK_EQ(x13, 0xffffffff, 0x1)
        "andi x13, x11, -1\n"  CHECK_EQ(x13, 4, 0x1)

        TEST_GROUP(test_itype_shift,0)
        "li x11, 4\n"
        "slli x13, x11, 2\n"   CHECK_EQ(x13, 0x10, 0x2)
        "srli x13, x11, 2\n"   CHECK_EQ(x13, 1, 0x2)
        "li x11, -1\n"
        "srai x13, x11, 4\n"   CHECK_EQ(x13, 0xffffffff, 0x2)    // 立即数=0x404,
        "srli x13, x11, 4\n"   CHECK_EQ(x13, 0x0fffffff, 0x2)    // funct7 混入但不污染移位量

        TEST_GROUP(test_itype_bound,0)
        "li x11, 1\n"
        "slti x13, x11, 2047\n"  CHECK_EQ(x13, 1, 0x4)
        "slti x13, x11, -2048\n" CHECK_EQ(x13, 0, 0x4)
        "xori x13, x11, 2047\n"  CHECK_EQ(x13, 0x7fe, 0x4)       // 1^0x7ff
        "xori x13, x11, -2048\n" CHECK_EQ(x13, 0xfffff801, 0x4)  // 1^0xfffff800

        TEST_GROUP(test_itype_shift_bound,0)
        "li x11, 0x12345678\n"
        "slli x13, x11, 0\n"   CHECK_EQ(x13, 0x12345678, 0x8)    // 移位量 0
        "li x11, 1\n"
        "slli x13, x11, 31\n"  CHECK_EQ(x13, 0x80000000, 0x8)    // 移位量 31
        "li x11, -1\n"
        "srli x13, x11, 31\n"  CHECK_EQ(x13, 1, 0x8)
        "srai x13, x11, 31\n"  CHECK_EQ(x13, 0xffffffff, 0x8)

        TEST_END()
        : : : "x11","x12","x13","x14","x15","t0","t1","t2","a0","memory"
    );
    __builtin_unreachable();
}
