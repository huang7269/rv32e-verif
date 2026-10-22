// ============================================================
// 模块名 : alu
// 功能   : RV32E 算术逻辑单元,支持 10 种运算
//          移位量取低 5 位(b[4:0]),SLT/SRA 区分有无符号
// 作者   : huangbang
// 日期   : 2026-10
// ============================================================

`timescale 1ns/1ps

module alu(
    input  wire [31:0] a,
    input  wire [31:0] b,
    input  wire [3:0]  alu_ctrl,
    output reg  [31:0] alu_result   
);
    localparam OP_ADD  = 4'd0;   //加法
    localparam OP_SUB  = 4'd1;   //减法
    localparam OP_SLL  = 4'd2;   //逻辑左移
    localparam OP_SLT  = 4'd3;   //小于则置位
    localparam OP_SLTU = 4'd4;   //无符号小于则置位
    localparam OP_XOR  = 4'd5;   //异或
    localparam OP_SRL  = 4'd6;   //逻辑右移
    localparam OP_SRA  = 4'd7;   //算术右移
    localparam OP_OR   = 4'd8;   //或运算
    localparam OP_AND  = 4'd9;   //与运算

    always @(*) begin
        case(alu_ctrl)
            OP_ADD:  alu_result = a + b;
            OP_SUB:  alu_result = a - b;
            OP_SLL:  alu_result = a << b[4:0];
            OP_SLT:  alu_result = ($signed(a) < $signed(b)) ? 32'h1 : 32'h0;
            OP_SLTU: alu_result = (a < b) ? 32'h1 : 32'h0;
            OP_XOR:  alu_result = a ^ b;
            OP_SRL:  alu_result = a >> b[4:0];
            OP_SRA:  alu_result = $signed(a) >>> b[4:0];
            OP_OR:   alu_result = a | b;
            OP_AND:  alu_result = a & b;
            default: alu_result = 32'd0;
        endcase
    end 
endmodule
