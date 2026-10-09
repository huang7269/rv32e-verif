`timescale 1ns/1ps

module inst_mem(
    input  logic [31:0] inst_addr_i,
    output logic [31:0] inst_data_o
);
    import "DPI-C" function int unsigned pmem_read(input int unsigned addr);

    assign inst_data_o = pmem_read(inst_addr_i);

endmodule
