#ifndef _PMEM_H_
#define _PMEM_H_

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "svdpi.h"
// 内存大小:4MB
#define PMEM_SIZE 0x00400000
#define PMEM_BASE 0x00000000

//初始化内存，将hex文件载入内存
int pmem_init(const char *hex_file);

extern "C" {
//读内存
uint32_t pmem_read(uint32_t addr);

//往内存写数据
void pmem_write(uint32_t addr,uint32_t wdata,uint8_t wmask);
}
//释放内存
void pmem_free();

#endif
