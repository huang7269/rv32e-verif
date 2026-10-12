`timescale 1ps/1ps

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

    assign rdata_1 = (rs1 == 5'b000000) ? {DATA_W{1'b0}}:
                     (rs1 < REG_N) ? regfiles[rs1] : {DATA_W{1'b0}};
    assign rdata_2 = (rs2 == 5'b000000) ? {DATA_W{1'b0}}:
                     (rs2 < REG_N) ? regfiles[rs2] : {DATA_W{1'b0}};

    always @(posedge clock) begin
        if(reg_write && (rd != 5'd0) && (rd < REG_N)) begin
            regfiles[rd] <= wdata;
        end
    end
endmodule
