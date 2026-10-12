#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "checker.h"

const ExpReg exp_reg{
    .regs = {
        0x00000000, // x0
        0x00000000, // x1
        0x00000000, // x2
        0x00000000, // x3
        0x00000000, // x4
        0x00000000, // x5
        0x00000000, // x6
        0x00000000, // x7
        0x00000000, // x8
        0x00000000, // x9
        0x00000006, // x10
        0x00000000, // x11
        0x00000000, // x12
        0x00000000, // x13
        0x00000000, // x14
        0x00000000  // x15
    }
};

int diff_reg(Vrv32e_scpu* top){
    int num = 0;
    for(int i = 0;i < 16;i++){
        uint32_t val = top->rv32e_scpu->u_regfile->regfiles[i];
        if(val != exp_reg.regs[i]){
            printf("[FAIL] x%d = 0x%x (exp:0x%x)\n",i,val,exp_reg.regs[i]);
            num++;
        }
    }
    return num;
}
