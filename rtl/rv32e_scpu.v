// ============================================================
// 模块名 : rv32e_scpu
// 功能   : RV32E 单周期 CPU 顶层
//          取指/译码/执行/访存/写回,ebreak/ecall 触发 halt
// 作者   : huangbang
// 日期   : 2026-10
// ============================================================

`timescale 1ns/1ps

module rv32e_scpu(
    input  wire clock,
    input  wire reset_n,
    output wire [31:0] pc,
    output wire [31:0] inst,
    output wire halt
);

// ----------取指令----------
    reg  [31:0] pc_reg;
    wire [31:0] pc_next;
    wire [31:0] inst_w;

    inst_mem u_inst_mem(
        .inst_addr_i   (pc_reg  ),
        .inst_data_o   (inst_w  )
    );

// ----------指令字段----------
    /* verilator lint_off UNUSEDSIGNAL */
    wire [6:0] fun7   = inst_w[31:25];
    wire [2:0] fun3   = inst_w[14:12];
    wire [4:0] rs1    = inst_w[19:15];
    wire [4:0] rs2    = inst_w[24:20];
    wire [4:0] rd     = inst_w[11:7];
    wire [6:0] opcode = inst_w[6:0];

// ----------控制----------
    wire reg_write,mem_write,mem_read;
    wire [1:0] alu_op;
    wire [1:0] alu_src_a;
    wire alu_src_b;
    wire branch;
    wire jump;
    wire jalr;
    wire [2:0] imm_sel;

    control u_control (
        .opcode      (opcode      ),
        .reg_write   (reg_write   ),
        .mem_write   (mem_write   ),
        .mem_read    (mem_read    ),
        .branch      (branch      ),
        .jump        (jump        ),
        .jalr        (jalr        ),
        .alu_op      (alu_op      ),
        .alu_src_a   (alu_src_a   ),
        .alu_src_b   (alu_src_b   ),
        .imm_sel     (imm_sel     )
    );

// ----------立即数----------
    wire [31:0] imm;

    imm_gen u_imm_gen(
        .inst    (inst_w[31:7]    ),
        .imm_sel (imm_sel         ),
        .imm     (imm             )
    );

// ----------分支单元----------
    wire branch_taken;

    branch_unit u_branch_unit(
        .rs1          (rdata_1     ),
        .rs2          (rdata_2     ),
        .fun3         (fun3        ),
        .branch_taken (branch_taken)
    );
// ----------寄存器堆----------
    wire [31:0] rdata_1,rdata_2,wdata;

    regfile u_regfile(
        .clock      (clock     ),
        .reg_write  (reg_write ),
        .rs1        (rs1       ),
        .rs2        (rs2       ),
        .rd         (rd        ),
        .wdata      (wdata     ),
        .rdata_1    (rdata_1   ),
        .rdata_2    (rdata_2   )
    );

// ----------数据存储器----------
    wire [31:0] reg_to_mem,mem_to_reg;
    assign reg_to_mem = rdata_2;

    data_mem u_data_mem(
        .clock          (clock       ),
        .mem_read       (mem_read    ),
        .mem_write      (mem_write   ),
        .addr           (alu_result  ),
        .fun3           (fun3        ),
        .wdata          (reg_to_mem  ),
        .data           (mem_to_reg  )
    );
// ----------运算器与控制----------
    wire [3:0] alu_ctrl;
    
    alu_control u_alu_control(
        .alu_op   	(alu_op    ),
        .fun3     	(fun3      ),
        .fun7_5     (fun7[5]   ),
        .alu_ctrl 	(alu_ctrl  )
    );
    
    wire [31:0] operand_a,operand_b,alu_result;
    assign operand_a = (alu_src_a == 2'b10) ? pc_reg :
                        (alu_src_a == 2'b01) ? 32'h0 :
                        rdata_1;
    assign operand_b = (alu_src_b == 1'b1) ? imm :
                        rdata_2;

    alu u_alu(
        .a          (operand_a      ),
        .b          (operand_b      ),
        .alu_ctrl   (alu_ctrl       ),
        .alu_result (alu_result     )
    );

// ----------写回-----------
    assign wdata = mem_read ? mem_to_reg :
                    jump ?     pc_plus :
                    alu_result;

// ----------下一个PC----------
    wire [31:0] jump_target = jalr ? (alu_result & ~32'h1) : alu_result;
    wire [31:0] pc_plus = pc_reg + 32'd4;
    assign pc_next = jump                   ? jump_target :
                    (branch & branch_taken) ? alu_result :
                    pc_plus;

    always @(posedge clock or negedge reset_n) begin
        if(!reset_n) begin
            pc_reg <= 32'h00000000;
        end
        else begin
            pc_reg <= pc_next;
        end
    end

// ----------ebreak/ecall检测----------
    assign pc = pc_reg;
    assign inst = inst_w;
    assign halt = (inst_w == 32'h00100073 | inst_w == 32'h00000073);  //ebreak/ecall

endmodule
