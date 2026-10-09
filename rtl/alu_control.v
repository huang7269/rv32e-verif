`timescale 1ns/1ps

module alu_control(
    input  wire [1:0] alu_op,
    input  wire [2:0] fun3,
    input  wire [6:0] fun7,
    output reg  [3:0] alu_ctrl
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
        case(alu_op)
            2'b01:begin                  //I型
                case(fun3)
                    3'b000: alu_ctrl = OP_ADD;    //addi
                endcase
            end
            default: alu_ctrl = OP_ADD;
        endcase
    end
endmodule
