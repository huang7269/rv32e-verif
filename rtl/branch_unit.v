// ============================================================
// 模块名 : branch_unit
// 功能   : 分支条件判断,支持 beq/bne/blt/bge/bltu/bgeu
//          blt/bge 有符号比较,bltu/bgeu 无符号比较
// 作者   : huangbang
// 日期   : 2026-10
// ============================================================

`timescale 1ns/1ps

module branch_unit(
    input wire [31:0] rs1,
    input wire [31:0] rs2,
    input wire [2:0] fun3,
    output reg branch_taken
);
    always @(*) begin
        case(fun3)
            3'b000 : branch_taken = (rs1 == rs2) ? 1'b1 : 1'b0;                    //beq
            3'b001 : branch_taken = (rs1 != rs2) ? 1'b1 : 1'b0;                    //bne
            3'b100 : branch_taken = ($signed(rs1) < $signed(rs2)) ? 1'b1 : 1'b0;   //blt
            3'b101 : branch_taken = ($signed(rs1) >= $signed(rs2)) ? 1'b1 : 1'b0;  //bge
            3'b110 : branch_taken = (rs1 < rs2) ? 1'b1 : 1'b0;                     //bltu
            3'b111 : branch_taken = (rs1 >= rs2) ? 1'b1 : 1'b0;                    //bgeu
            default: branch_taken = 1'b0;
        endcase
    end
endmodule
