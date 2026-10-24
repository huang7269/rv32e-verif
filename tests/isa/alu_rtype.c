// ============================================================
// 测试   : alu_rtype.c
// 覆盖   : testplan §ALU R 型全部测试点(10 条指令、借位、有无符号、
//          负数移位、移位量 0/31 边界、溢出回绕、x0 作 rd)
// 协议   : a0(x10)=0 全通过;错误位:bit0 基本运算,bit1 移位,
//          bit2 移位边界,bit3 slt/sltu,bit4 逻辑运算,bit5 溢出回绕,
//          bit6 x0 作 rd
// 期望   : x10 0x0
// ============================================================
#include "macros.h"

void _start() __attribute__((noreturn));

void _start(){
    __asm__ volatile (
        "li a0, 0\n"

        TEST_GROUP(test_arith,0)
        "addi x11, x0, 4\n"
        "addi x12, x0, 5\n"
        "add x13, x11, x12\n"  CHECK_EQ(x13, 9, 0x1)
        "sub x13, x12, x11\n"  CHECK_EQ(x13, 1, 0x1)
        "sub x13, x11, x12\n"  CHECK_EQ(x13, 0xffffffff, 0x1)   // 借位

        TEST_GROUP(test_shift,0)
        "sll x13, x12, x11\n"  CHECK_EQ(x13, 0x50, 0x2)          // 5<<4
        "li x12, 0x10\n"
        "srl x13, x12, x11\n"  CHECK_EQ(x13, 1, 0x2)
        "sra x13, x12, x11\n"  CHECK_EQ(x13, 1, 0x2)
        "li x12, -1\n"
        "sra x13, x12, x11\n"  CHECK_EQ(x13, 0xffffffff, 0x2)    // 负数算术右移

        TEST_GROUP(test_shift_bound,0)
        "li x11, 0x12345678\n"
        "li x12, 0\n"
        "sll x13, x11, x12\n"  CHECK_EQ(x13, 0x12345678, 0x4)    // 移位量 0
        "li x11, 1\n"
        "li x12, 31\n"
        "sll x13, x11, x12\n"  CHECK_EQ(x13, 0x80000000, 0x4)    // 移位量 31
        "li x11, -1\n"
        "srl x13, x11, x12\n"  CHECK_EQ(x13, 1, 0x4)
        "sra x13, x11, x12\n"  CHECK_EQ(x13, 0xffffffff, 0x4)

        TEST_GROUP(test_slt,0)
        "li x11, 1\n"
        "li x12, 2\n"
        "li x14, -1\n"
        "slt x13, x11, x12\n"  CHECK_EQ(x13, 1, 0x8)    // 1<2
        "slt x13, x12, x11\n"  CHECK_EQ(x13, 0, 0x8)    // 2<1 假
        "slt x13, x11, x14\n"  CHECK_EQ(x13, 0, 0x8)    // 1<-1 假(正<负)
        "slt x13, x14, x14\n"  CHECK_EQ(x13, 0, 0x8)    // 相等不置位
        "sltu x13, x11, x14\n" CHECK_EQ(x13, 1, 0x8)    // 无符号 1<0xffffffff
        "sltu x13, x14, x11\n" CHECK_EQ(x13, 0, 0x8)

        TEST_GROUP(test_logic,0)
        "li x11, 1\n"
        "li x12, 2\n"
        "xor x13, x11, x12\n"  CHECK_EQ(x13, 3, 0x10)
        "or x13, x11, x12\n"   CHECK_EQ(x13, 3, 0x10)
        "and x13, x11, x12\n"  CHECK_EQ(x13, 0, 0x10)

        TEST_GROUP(test_overflow,0)
        "li x11, 0x7fffffff\n"
        "addi x12, x0, 1\n"
        "add x13, x11, x12\n"  CHECK_EQ(x13, 0x80000000, 0x20)   // 溢出回绕
        "sub x13, x13, x12\n"  CHECK_EQ(x13, 0x7fffffff, 0x20)   // 负向回绕

        TEST_GROUP(test_x0_rd,0)
        "add x0, x11, x12\n"   CHECK_EQ(x0, 0, 0x40)             // x0 写不进去

        TEST_END()
        : : : "x11","x12","x13","x14","x15","t0","t1","t2","a0","memory"
    );
    __builtin_unreachable();
}
