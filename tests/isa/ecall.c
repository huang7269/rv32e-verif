// ============================================================
// 测试   : ecall.c
// 覆盖   : testplan §系统 ecall(halt 处理,与 ebreak 同)
// 协议   : a0(x10)=0 全通过;ecall 未触发 halt 时执行到 ori → a0=1 → FAIL
// 期望   : x10 0x0
// ============================================================
#include "macros.h"

void _start() __attribute__((noreturn));

void _start(){
    __asm__ volatile (
        "li a0, 0\n"

        TEST_GROUP(test_ecall_halt,0)
        "ecall\n"                           // 应触发 halt,仿真在此停止
        "ori a0, a0, 0x1\n"                 // 未 halt 才执行到这 → 错误位

        TEST_END()
        : : : "x11","x12","x13","x14","x15","t0","t1","t2","a0","memory"
    );
    __builtin_unreachable();
}
