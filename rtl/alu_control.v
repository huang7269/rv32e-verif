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
            2'b00: alu_ctrl = OP_ADD;             //lui
            2'b01:begin                           //I型
                case(fun3)
                    3'b000: alu_ctrl = OP_ADD;    //addi
                    3'b010: alu_ctrl = OP_SLT;    //slti
                    3'b011: alu_ctrl = OP_SLTU;   //sltiu
                    3'b100: alu_ctrl = OP_XOR;    //xori
                    3'b110: alu_ctrl = OP_OR;     //ori
                    3'b111: alu_ctrl = OP_AND;    //andi
                    3'b001: alu_ctrl = OP_SLL;    //slli
                    3'b101: alu_ctrl = fun7[5] ? OP_SRA : OP_SRL;   //srli/srai
                endcase
            end
            2'b10:begin                  //R型
                case(fun3)
                    3'b000: alu_ctrl = fun7[5] ? OP_SUB : OP_ADD;
                    3'b001: alu_ctrl = OP_SLL;
                    3'b010: alu_ctrl = OP_SLT;
                    3'b011: alu_ctrl = OP_SLTU;
                    3'b100: alu_ctrl = OP_XOR;
                    3'b101: alu_ctrl = fun7[5] ? OP_SRA : OP_SRL;
                    3'b110: alu_ctrl = OP_OR;
                    3'b111: alu_ctrl = OP_AND;
                endcase
            end
            default: alu_ctrl = OP_ADD;
        endcase
    end
endmodule
