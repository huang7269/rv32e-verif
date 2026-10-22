// ============================================================
// 模块名 : inst_mem
// 功能   : 指令存储器仿真模型,DPI-C 调用 pmem_read
//          组合读,无时钟
// 作者   : huangbang
// 日期   : 2026-10
// ============================================================

`timescale 1ns/1ps

module inst_mem(
    input  logic [31:0] inst_addr_i,
    output logic [31:0] inst_data_o
);
    import "DPI-C" function int unsigned pmem_read(input int unsigned addr);

    assign inst_data_o = pmem_read(inst_addr_i);

endmodule
