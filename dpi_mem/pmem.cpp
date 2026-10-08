#include "pmem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *mem_buf = NULL

// 初始化内存，加载hex文件
int pmem_init(const char *hex_file){
    if(mem_buf == NULL){
        mem_buf = (uint8_t *)calloc(PMEM_SIZE,1);
        if(mem_buf == NULL){
            printf("[pmem_init error]:mem_buf calloc failed\n");
            return -1;
        }
    }
    memset(mem_buf,0,PMEM_SIZE);

    if(hex_file == NULL || strlen(hex_file) == 0){
        printf("[pmem_init error]:hex_file NULL\n");
        return -1;
    }

    FILE *fp = fopen(hex_file,"r");
    if(fp == NULL){
        printf("[pmem_init error]:fopen %s failed\n",hex_file);
        return -1;
    }

    char line[32];
    uint32_t offset = 0;
    while(fgets(line,sizeof(line),fp) && offset < PMEM_SIZE){
        unsigned int data;
        if (sscanf(line, "%x", &data) == 1) {
            // 小端序存储，和RISC-V一致
            mem_buf[offset + 0] = (data >> 0)  & 0xFF;
            mem_buf[offset + 1] = (data >> 8)  & 0xFF;
            mem_buf[offset + 2] = (data >> 16) & 0xFF;
            mem_buf[offset + 3] = (data >> 24) & 0xFF;
            offset += 4;
        }
    }

    fclose(fp);
    printf("[pmem_init info] load hex_file success: %s size: %d\n",hex_file,offset);
    return 0;
}


uint32_t pmem_read(uint32_t addr){
    uint32_t align_addr = addr & ~3u;
    if(addr > PMEM_SIZE){
        printf("[pmem_read error] %d over PMEM_SIZE\n",addr);
        return 0;
    }

    uint32_t data = 0;
    data |= (uint32_t)mem_buf[align_addr + 0] << 0;
    data |= (uint32_t)mem_buf[align_addr + 1] << 8;
    data |= (uint32_t)mem_buf[align_addr + 2] << 16;
    data |= (uint32_t)mem_buf[align_addr + 3] << 24;
    return data;
}

void pmem_write(uint32_t addr,uint32_t wdata,uint8_t wmask){
    uint32_t align_addr = addr & ~3u;
    if(addr > PMEM_SIZE){
        printf("[pmem_write error] %d over PMEM_SIZE\n",addr);
        return;
    }

    // 按字节掩码写入
    for(int i = 0;i < 4;i++){
        if(wmask & (1 << i)){
            mem_buf[align_addr + 0] = (wdata >> i * 8) & 0xff;
        }
    }
}

void pmem_free(){
    if(mem_buf){
        free(mem_buf);
        mem_buf = NULL;
    }
}
