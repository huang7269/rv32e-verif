`timescale 1ns/1ps

module control(
    input  wire [6:0] opcode,
    output reg reg_write,
    output reg mem_write,
    output reg mem_read,
    output reg branch,
    output reg jump,
    output reg jalr,
    output reg [1:0] alu_op,
    output reg [1:0] alu_src_a,
    output reg       alu_src_b,
    output reg [2:0] imm_sel
);
    localparam OP_R     = 7'b0110011;
    localparam OP_I     = 7'b0010011;
    localparam OP_LUI   = 7'b0110111;
    localparam OP_AUIPC = 7'b0010111;
    localparam OP_B     = 7'b1100011;
    localparam OP_JAL   = 7'b1101111;
    localparam OP_JALR  = 7'b1100111;

    localparam IMM_I = 3'd0;
    localparam IMM_S = 3'd1;
    localparam IMM_B = 3'd2;
    localparam IMM_U = 3'd3;
    localparam IMM_J = 3'd4;

    always @(*) begin
        reg_write = 1'b0; mem_write = 1'b0; mem_read = 1'b0; alu_op = 2'b00;
        branch = 1'b0; jump = 1'b0; jalr = 1'b0; alu_src_a = 2'b00; alu_src_b = 1'b0;
        imm_sel = IMM_I;

        case(opcode)
            OP_R: begin
                reg_write = 1'b1;
                mem_write = 1'b0;
                mem_read  = 1'b0;
                alu_op    = 2'b10;
                alu_src_a = 2'b00;
                alu_src_b = 1'b0;
            end
            OP_I: begin
                reg_write = 1'b1;
                imm_sel   = IMM_I;
                alu_op    = 2'b01;
                alu_src_a = 2'b00;
                alu_src_b = 1'b1;
            end
            OP_LUI:begin
                reg_write = 1'b1;
                imm_sel   = IMM_U;
                alu_op    = 2'b00;
                alu_src_a = 2'b01;
                alu_src_b = 1'b1;
            end
            OP_AUIPC:begin
                reg_write = 1'b1;
                imm_sel   = IMM_U;
                alu_op    = 2'b00;
                alu_src_a = 2'b10;
                alu_src_b = 1'b1;
            end
            OP_B:begin
                reg_write = 1'b0;
                imm_sel   = IMM_B;
                alu_op    = 2'b00;
                alu_src_a = 2'b10;
                alu_src_b = 1'b1;
                branch    = 1'b1;
            end
            OP_JAL: begin
                reg_write = 1'b1;
                imm_sel   = IMM_J;
                alu_op    = 2'b00;
                alu_src_a = 2'b10;
                alu_src_b = 1'b1;
                jump      = 1'b1;
            end
            OP_JALR: begin
                reg_write = 1'b1;
                imm_sel   = IMM_I;
                alu_op    = 2'b01;
                alu_src_a = 2'b00;
                alu_src_b = 1'b1;
                jump      = 1'b1;
                jalr      = 1'b1;
            end
            default: ;
        endcase
    end
endmodule
