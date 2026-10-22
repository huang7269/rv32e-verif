// ============================================================
// 模块名 : data_mem
// 功能   : 数据存储器仿真模型,DPI-C 调用 pmem
//          load 按 addr[1:0] 选字节/半字并符号/零扩展
//          store 按 fun3 生成 wmask,数据复制到全字位宽
// 作者   : huangbang
// 日期   : 2026-10
// ============================================================

`timescale 1ns/1ps

module data_mem(
    input  wire        clock,
    input  wire        mem_read,
    input  wire        mem_write,
    input  wire [31:0] addr,
    input  wire [2:0]  fun3,
    input  wire [31:0] wdata,
    output reg  [31:0] data
);
    reg [31:0] rdata;

    localparam OP_LB  = 3'b000;
    localparam OP_LH  = 3'b001;
    localparam OP_LW  = 3'b010;
    localparam OP_LBU = 3'b100;
    localparam OP_LHU = 3'b101;

    import "DPI-C" function int unsigned pmem_read(int unsigned addr);
    import "DPI-C" function void pmem_write(int unsigned addr,int unsigned data,byte unsigned wmask);

    always @(*) begin
        data  = 32'h0;
        rdata = 32'h0;
        if(mem_read) begin
            rdata = pmem_read(addr);
            case(fun3)
                OP_LB:  data = {{24{rdata[addr[1:0]*8 + 7]}},rdata[addr[1:0]*8 +: 8]};
                OP_LH:  data = {{16{rdata[addr[1]*16 + 15]}},rdata[addr[1]*16 +: 16]};
                OP_LW:  data = rdata;
                OP_LBU: data = {{24{1'b0}},rdata[addr[1:0]*8 +: 8]};
                OP_LHU: data = {{16{1'b0}},rdata[addr[1]*16 +: 16]};
                default data = 32'h0;
            endcase
        end
    end

    always @(posedge clock) begin
        if(mem_write) begin
            case(fun3)
                3'b000: pmem_write(addr,{4{wdata[7:0]}}, 8'h1 << addr[1:0]);
                3'b001: pmem_write(addr,{2{wdata[15:0]}},8'h3 << addr[1:0]);
                3'b010: pmem_write(addr,wdata,8'hf);
                default: ;
            endcase
        end
    end
endmodule
