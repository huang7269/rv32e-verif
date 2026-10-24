// ============================================================
// 测试   : load_store.c
// 覆盖   : testplan §第 5 周访存全部测试点(4 种 wmask、符号/零扩展、
//          store-load 一致性、写掩码验证、跨粒度、地址边界 0x0/0x3fffffc)
// 协议   : a0(x10)=0 全通过;错误位:bit0 wmask 组,bit1 扩展组,
//          bit2 一致性组,bit3 地址边界组
// 期望   : x10 0x0
// 说明   : 基地址寄存器 x15,读回值寄存器 x14
// ============================================================
#include "macros.h"

void _start() __attribute__((noreturn));

void _start(){
    __asm__ volatile (
        "li a0, 0\n"
        "li x15, 0x200000\n"                // 测试内存基地址

        // ---------- 组1:wmask(bit0) ----------
        TEST_GROUP(test_wmask,0)
        "li x14, 0xDEADBEEF\n"
        "sw x14, 0(x15)\n"

        "lb x14, 0(x15)\n"  CHECK_EQ(x14, 0xffffffef, 0x1)   // 字节 0
        "lb x14, 1(x15)\n"  CHECK_EQ(x14, 0xffffffbe, 0x1)   // 字节 1
        "lb x14, 2(x15)\n"  CHECK_EQ(x14, 0xffffffad, 0x1)   // 字节 2
        "lb x14, 3(x15)\n"  CHECK_EQ(x14, 0xffffffde, 0x1)   // 字节 3
        "lh x14, 0(x15)\n"  CHECK_EQ(x14, 0xffffbeef, 0x1)   // 半字 0
        "lh x14, 2(x15)\n"  CHECK_EQ(x14, 0xffffdead, 0x1)   // 半字 2
        "lw x14, 0(x15)\n"  CHECK_EQ(x14, 0xdeadbeef, 0x1)   // 整字

        // ---------- 组2:符号/零扩展(bit1) ----------
        TEST_GROUP(test_extend,0)
        "li x14, 0x80\n"
        "sb x14, 0(x15)\n"
        "li x14, 0x7F\n"
        "sb x14, 1(x15)\n"
        "li x14, 0x8000\n"
        "sh x14, 2(x15)\n"
        "li x14, 0x7FFF\n"
        "sh x14, 4(x15)\n"

        "lb x14, 0(x15)\n"  CHECK_EQ(x14, 0xffffff80, 0x2)   // 负字节符号扩展
        "lbu x14, 0(x15)\n" CHECK_EQ(x14, 0x80, 0x2)         // 零扩展
        "lb x14, 1(x15)\n"  CHECK_EQ(x14, 0x7f, 0x2)         // 正字节
        "lbu x14, 1(x15)\n" CHECK_EQ(x14, 0x7f, 0x2)
        "lh x14, 2(x15)\n"  CHECK_EQ(x14, 0xffff8000, 0x2)   // 负半字符号扩展
        "lhu x14, 2(x15)\n" CHECK_EQ(x14, 0x8000, 0x2)       // 零扩展
        "lh x14, 4(x15)\n"  CHECK_EQ(x14, 0x7fff, 0x2)       // 正半字
        "lhu x14, 4(x15)\n" CHECK_EQ(x14, 0x7fff, 0x2)

        // ---------- 组3:一致性 + 写掩码 + 跨粒度(bit2) ----------
        TEST_GROUP(test_consist,0)
        "sw x0, 0(x15)\n"
        "sw x0, 4(x15)\n"

        // 同粒度回环 sb→lb/lbu
        "li x14, 0x78\n"
        "sb x14, 0(x15)\n"
        "lb x14, 0(x15)\n"  CHECK_EQ(x14, 0x78, 0x4)
        "lbu x14, 0(x15)\n" CHECK_EQ(x14, 0x78, 0x4)

        // 同粒度回环 sh→lh/lhu
        "li x14, 0xABCD\n"
        "sh x14, 2(x15)\n"
        "lh x14, 2(x15)\n"  CHECK_EQ(x14, 0xffffabcd, 0x4)
        "lhu x14, 2(x15)\n" CHECK_EQ(x14, 0xabcd, 0x4)

        // 同粒度回环 sw→lw
        "li x14, 0xCAFEBABE\n"
        "sw x14, 4(x15)\n"
        "lw x14, 4(x15)\n"  CHECK_EQ(x14, 0xcafebabe, 0x4)

        // 写掩码验证:sb 只改目标字节
        "li x14, 0x11223344\n"
        "sw x14, 0(x15)\n"
        "li x14, 0x99\n"
        "sb x14, 1(x15)\n"
        "lw x14, 0(x15)\n"  CHECK_EQ(x14, 0x11229944, 0x4)

        // 写掩码验证:sh 只改目标半字
        "li x14, 0xAABBCCDD\n"
        "sw x14, 0(x15)\n"
        "li x14, 0x1234\n"
        "sh x14, 2(x15)\n"
        "lw x14, 0(x15)\n"  CHECK_EQ(x14, 0x1234ccdd, 0x4)

        // 跨粒度:整字写 → 分字节/半字读
        "li x14, 0x11223344\n"
        "sw x14, 0(x15)\n"
        "lbu x14, 0(x15)\n" CHECK_EQ(x14, 0x44, 0x4)
        "lbu x14, 1(x15)\n" CHECK_EQ(x14, 0x33, 0x4)
        "lbu x14, 2(x15)\n" CHECK_EQ(x14, 0x22, 0x4)
        "lbu x14, 3(x15)\n" CHECK_EQ(x14, 0x11, 0x4)
        "lhu x14, 0(x15)\n" CHECK_EQ(x14, 0x3344, 0x4)
        "lhu x14, 2(x15)\n" CHECK_EQ(x14, 0x1122, 0x4)

        // ---------- 组4:地址边界(bit3) ----------
        TEST_GROUP(test_boundary,0)
        "li x14, 0x12345678\n"
        "li x15, 0\n"
        "sw x14, 0(x15)\n"                  // 写地址 0x0
        "lw x14, 0(x15)\n"  CHECK_EQ(x14, 0x12345678, 0x8)
        "li x15, 0x003ffffc\n"               // 最高对齐字地址(PMEM_SIZE=4MB)
        "sw x14, 0(x15)\n"
        "lw x14, 0(x15)\n"  CHECK_EQ(x14, 0x12345678, 0x8)

        TEST_END()
        : : : "x11","x12","x13","x14","x15","t0","t1","t2","a0","memory"
    );
    __builtin_unreachable();
}
