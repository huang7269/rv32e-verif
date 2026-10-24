// ============================================================
// 测试   : alu_utype.c
// 覆盖   : testplan §第 4 周 U 型全部测试点(lui 基本/最大值/x0 作 rd,
//          auipc pc+imm、负立即数)
// 协议   : a0(x10)=0 全通过;错误位:bit0 lui 基本,bit1 lui 最大值,
//          bit2 lui x0 作 rd,bit3 auipc 正立即数,bit4 auipc 负立即数
// 期望   : x10 0x0
// ============================================================
#include "macros.h"

void _start() __attribute__((noreturn));

void _start(){
    __asm__ volatile (
        "li a0, 0\n"

        TEST_GROUP(test_lui_basic,0)
        "lui x11, 1\n"         CHECK_EQ(x11, 0x1000, 0x1)        // 低 12 位为 0
        "lui x12, 0\n"         CHECK_EQ(x12, 0, 0x1)

        TEST_GROUP(test_lui_max,0)
        "lui x13, 0xfffff\n"   CHECK_EQ(x13, 0xfffff000, 0x2)    // 最大值

        TEST_GROUP(test_lui_x0,0)
        "lui x0, 0xfffff\n"    CHECK_EQ(x0, 0, 0x4)              // x0 作 rd 丢弃

        TEST_GROUP(test_auipc_pos,0)
        "auipc x11, 1\n"                                        // x11 = pc+0x1000
        "auipc x12, 0\n"                                         // x12 = pc+4(下一条的 pc)
        "sub x14, x11, x12\n"                                    // = 0x1000-4
        "addi x14, x14, 4\n"  CHECK_EQ(x14, 0x1000, 0x8)         // 补偿 4 字节 pc 差,相对校验

        TEST_GROUP(test_auipc_neg,0)
        "auipc x11, 0xfffff\n"                                   // x11 = pc+0xfffff000 = pc-0x1000
        "auipc x12, 0\n"                                         // x12 = pc+4
        "sub x14, x12, x11\n"                                    // = 0x1000+4
        "addi x14, x14, -4\n" CHECK_EQ(x14, 0x1000, 0x10)
        "slt x14, x11, x0\n"   CHECK_EQ(x14, 1, 0x10)            // x11 为负(pc<0x1000)

        TEST_END()
        : : : "x11","x12","x13","x14","x15","t0","t1","t2","a0","memory"
    );
    __builtin_unreachable();
}
