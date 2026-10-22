// ============================================================
// 模块名 : regfile
// 功能   : 寄存器堆,16 个 32 位寄存器(RV32E)
//          x0 恒为 0;rs[4] 守卫非法寄存器 x16~x31 返回 0
// 作者   : huangbang
// 日期   : 2026-10
// ============================================================

`timescale 1ns/1ps

module regfile #(
    parameter ADDR_W = 5,
    parameter DATA_W = 32,
    parameter REG_N  = 16
)(
    input  wire clock,
    input  wire reg_write,
    input  wire [ADDR_W-1:0] rs1,
    input  wire [ADDR_W-1:0] rs2,
    input  wire [ADDR_W-1:0] rd,
    input  wire [DATA_W-1:0] wdata,
    output wire [DATA_W-1:0] rdata_1,
    output wire [DATA_W-1:0] rdata_2
);
    reg [DATA_W-1:0] regfiles [0:REG_N-1] /*verilator public*/;

    assign rdata_1 = (rs1[4]) ? {DATA_W{1'b0}} : regfiles[rs1[3:0]];
    assign rdata_2 = (rs2[4]) ? {DATA_W{1'b0}} : regfiles[rs2[3:0]];

    always @(posedge clock) begin
        if(reg_write && (rd != 5'd0) && !rd[4]) begin
            regfiles[rd[3:0]] <= wdata;
        end
    end
endmodule
