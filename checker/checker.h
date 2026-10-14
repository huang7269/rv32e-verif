#ifndef _CHECKER_H_
#define _CHECKER_H_

#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vrv32e_scpu.h"
#include "Vrv32e_scpu_rv32e_scpu.h"
#include "Vrv32e_scpu_regfile.h"

struct ExpReg{
    uint32_t regs[16];
};

int read_exp(const char *filename);
int diff_reg(Vrv32e_scpu* top);

#endif