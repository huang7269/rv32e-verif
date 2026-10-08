`timescale 1ns/1ps

module inst_mem(
    input  wire [31:0] inst_addr_i,
    output wire [31:0] inst_data_o
);
    import "DPI-C" funtion int unsigned pmem_read(input wire [31:0] addr);

    always_comb begin
        inst_data_o = pmem_read(inst_addr_i);
    end

endmodule
