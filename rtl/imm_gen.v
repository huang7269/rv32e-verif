// ============================================================
// 模块名 : imm_gen
// 功能   : 立即数生成,支持 I/S/B/U/J 五种格式
//          B/J 型最低位硬连线 0(2 字节对齐)
// 作者   : huangbang
// 日期   : 2026-10
// ============================================================

`timescale 1ns/1ps

module imm_gen(
    input  wire [31:7] inst,
    input  wire [2:0] imm_sel,
    output reg  [31:0] imm
);
    localparam IMM_I = 3'd0;
    localparam IMM_S = 3'd1;
    localparam IMM_B = 3'd2;
    localparam IMM_U = 3'd3;
    localparam IMM_J = 3'd4;

    always @(*) begin
        case(imm_sel)
            IMM_I: imm = {{20{inst[31]}},inst[31:20]};
            IMM_S: imm = {{20{inst[31]}},inst[31:25],inst[11:7]};
            IMM_B: imm = {{19{inst[31]}},inst[31],inst[7],inst[30:25],inst[11:8],1'b0};
            IMM_U: imm = {inst[31:12],12'b0};
            IMM_J: imm = {{11{inst[31]}},inst[31],inst[19:12],inst[20],inst[30:21],1'b0};
            default: imm = 32'd0;
        endcase
    end
endmodule
