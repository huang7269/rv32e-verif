`timescale 1ns/1ps

module control(
    input  wire [6:0] opcode,
    output reg reg_write,
    output reg mem_write,
    output reg mem_read,
    output reg [1:0] alu_op,
    output reg [2:0] imm_sel
);
    localparam OP_R = 7'b0110011;
    localparam OP_I = 7'b0010011;

    localparam IMM_I = 3'd0;
    localparam IMM_S = 3'd1;
    localparam IMM_B = 3'd2;
    localparam IMM_U = 3'd3;
    localparam IMM_J = 3'd4;

    always @(*) begin
        reg_write = 1'b0;mem_write = 1'b0;mem_read = 1'b0;alu_op = 2'b00;
        imm_sel = IMM_I;

        case(opcode)
            OP_R: begin
                reg_write = 1'b1;
                alu_op    = 2'b10;
            end
            OP_I: begin
                reg_write = 1'b1;
                imm_sel = IMM_I;
                alu_op = 2'b01;
            end
        endcase
    end
endmodule
