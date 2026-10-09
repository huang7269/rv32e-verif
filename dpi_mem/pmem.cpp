#include "pmem.h"

static uint8_t *mem_buf = NULL;

// 初始化内存，加载hex文件
int pmem_init(const char *hex_file) {
    if (mem_buf == NULL) {
        mem_buf = (uint8_t *)calloc(PMEM_SIZE, sizeof(uint8_t));
        if (mem_buf == NULL) {
            printf("[pmem_init error]: mem_buf calloc failed\n");
            return -1;
        }
    }
    memset(mem_buf, 0, PMEM_SIZE);

    if (hex_file == NULL || strlen(hex_file) == 0) {
        printf("[pmem_init error]: hex_file NULL\n");
        return -1;
    }

    FILE *fp = fopen(hex_file, "r");
    if (fp == NULL) {
        printf("[pmem_init error]: fopen %s failed\n", hex_file);
        return -1;
    }

    char line[256];
    uint32_t base_addr = 0;  // 当前写入的基地址

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;

        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (*p == '\0') continue;

        if (*p == '@') {
            base_addr = strtoul(p + 1, NULL, 16);
            continue;
        }

        // 2. 逐字节解析数据行
        while (*p != '\0' && base_addr < PMEM_SIZE) {
            // 跳过空白
            while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
            if (*p == '\0') break;

            // 读取一个字节（2位十六进制）
            unsigned int byte_val;
            if (sscanf(p, "%2x", &byte_val) != 1) break;

            // 写入内存，地址+1
            mem_buf[base_addr] = (uint8_t)byte_val;
            base_addr++;

            // 指针跳过2个十六进制字符
            p += 2;
        }
    }

    fclose(fp);
    return 0;
}


uint32_t pmem_read(uint32_t addr){
    uint32_t align_addr = addr & ~3u;
    if(addr >= PMEM_SIZE){
        printf("[pmem_read error] 0x%08x over PMEM_SIZE\n",addr);
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
    if(addr >= PMEM_SIZE){
        printf("[pmem_write error] %d over PMEM_SIZE\n",addr);
        return;
    }

    // 按字节掩码写入
    for(int i = 0;i < 4;i++){
        if(wmask & (1 << i)){
            mem_buf[align_addr + i] = (wdata >> i * 8) & 0xff;
        }
    }
}

void pmem_free(){
    if(mem_buf){
        free(mem_buf);
        mem_buf = NULL;
    }
}
