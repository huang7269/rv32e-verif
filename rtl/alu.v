`timescale 1ns/1ps

module alu(
    input  reg [31:0] rd_1,
    input  reg [31:0] rd_2,
    input  reg [3:0]  alu_ctrl,
    output reg [31:0] alu_result   
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
            OP_ADD:  alu_result = rd_1 + rd_2;
            OP_SUB:  alu_result = rd_1 - rd_2;
            default: alu_result = 32'd0;
        endcase
    end 
endmodule
