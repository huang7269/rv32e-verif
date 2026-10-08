#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vrv32e_scpu.h"
#include "pmem.h" 

int main(int argc, char **argv) {
    // ========== 1. 参数解析 ==========
    if (argc < 2) {
        printf("用法: %s <程序.hex> [--no-wave]\n", argv[0]);
        return 1;
    }
    const char* hex_file = argv[1];
    bool enable_wave = true;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--no-wave") == 0) enable_wave = false;
    }

    // ========== 2. 初始化内存，加载测试程序 ==========
    if (pmem_init(hex_file) != 0) {
        printf("[ERROR] 加载程序失败: %s\n", hex_file);
        return 1;
    }

    // ========== 3. 仿真环境初始化 ==========
    VerilatedContext* contextp = new VerilatedContext;
    contextp->commandArgs(argc, argv);
    contextp->traceEverOn(enable_wave);

    Vrv32e_scpu* top = new Vrv32e_scpu(contextp);

    // 波形配置
    VerilatedVcdC* tfp = nullptr;
    if (enable_wave) {
        tfp = new VerilatedVcdC;
        top->trace(tfp, 99);  // 99: 跟踪最大层级深度
        tfp->open("wave.vcd");
    }

    // ========== 4. 时序参数 ==========
    const int clk_half = 5;       // 半周期5ns → 周期10ns → 100MHz
    const int max_cycles = 10000; // 超时保护：最大10000周期，防止死循环
    int cycle_count = 0;
    int ret_code = 0;             // 返回码: 0=正常停机 1=加载失败 2=超时

    // ========== 5. 复位序列（高有效） ==========
    top->clock = 0;
    top->reset = 1;
    // 先跑2个完整周期的复位
    for (int i = 0; i < 4; i++) {
        top->clock = !top->clock;
        top->eval();
        if (tfp) tfp->dump(i * clk_half);
    }
    top->reset = 0; // 低半周期释放复位，避免时钟沿亚稳态

    // ========== 6. 主仿真循环 ==========
    while (!Verilated::gotFinish()) {
        // 检测ebreak触发的halt信号，自动停机
        if (top->halt) {
            printf("[INFO] 遇到EBREAK，仿真结束，总周期数: %d\n", cycle_count);
            ret_code = 0;
            break;
        }

        // 超时检测
        if (cycle_count >= max_cycles) {
            printf("[WARN] 仿真超时 (%d周期)，程序可能死循环\n", max_cycles);
            ret_code = 2;
            break;
        }

        // ----- 时钟低半周期 -----
        top->clock = 0;
        top->eval();
        if (tfp) tfp->dump(cycle_count * 2 * clk_half);

        // ----- 时钟高半周期 -----
        top->clock = 1;
        top->eval();
        if (tfp) tfp->dump(cycle_count * 2 * clk_half + clk_half);

        cycle_count++;
    }

    // ========== 7. 收尾清理 ==========
    if (tfp) {
        tfp->close();
        delete tfp;
    }
    delete top;
    delete contextp;
    pmem_free(); // 释放内存模型

    if (ret_code == 0) {
        std::cout << "=== 仿真正常结束 ===" << std::endl;
    }
    return ret_code;
}
