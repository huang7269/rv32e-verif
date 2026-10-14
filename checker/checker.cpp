#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "checker.h"

ExpReg exp_reg{
    .regs = {0}
};

int read_exp(const char *filename){
    FILE *fp = fopen(filename,"r");
    if(!fp){
        printf("[ERROR] fopen %s failed!\n",filename);
        return -1;
    }

    char line[128];
    char reg_name[16];
    uint32_t val;
    while(fgets(line,sizeof(line),fp)){
        if(sscanf(line,"%s %x",reg_name,&val) != 2){
            continue;
        }

        if(reg_name[0] == 'x'){
            int idx = atoi(reg_name + 1);
            if(idx >= 0 && idx < 16){
                exp_reg.regs[idx] = val;
            }
        }
    }
    fclose(fp);

    return 0;
}

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
