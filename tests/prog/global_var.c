// ============================================================
// 测试   : global_var.c(程序级)
// 覆盖   : testplan §第 5 周"全局变量"验收测试点
//          验证 link.ld 生效:.sdata/.bss 落在 DMEM 区(0x200000),
//          访存与 auipc/addi(la)组合工作
// 协议   : a0(x10)=0 全通过;result==6 时 a0=0,否则 a0≠0 → FAIL
// 期望   : x10 0x0
// ============================================================

// 4 字节 < gcc 默认 -msmall-data-limit=8 → 进入 .sdata(link.ld 已收编)
volatile int counter = 5;
// 未初始化 → 进入 .bss
volatile int result = 0;

void _start() __attribute__((noreturn));

void _start(){
    __asm__ volatile (
        "la x14, counter\n"                 // auipc+addi → .sdata 地址
        "lw x15, 0(x14)\n"                  // x15 = 5
        "addi x15, x15, 1\n"                // x15 = 6
        "la x14, result\n"                  // auipc+addi → .bss 地址
        "sw x15, 0(x14)\n"                  // result = 6
        "lw a0, 0(x14)\n"                   // a0 = 6
        "addi a0, a0, -6\n"                 // a0 = 0 表示通过
        "li x14, 0\n"
        "li x15, 0\n"
        "ebreak\n"
        : : : "x14","x15","a0","memory"
    );
    __builtin_unreachable();
}
