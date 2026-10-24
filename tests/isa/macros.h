#ifndef _MACROS_H_
#define _MACROS_H_

//检查reg里的值和expect是否相等，不等a0的bit位置1
#define CHECK_EQ(reg, expected, bit)  \
    "li t2," #expected "\n" \
    "beq t2," #reg ",1f\n" \
    "ori a0,a0," #bit "\n" \
    "1:\n"

//检查reg与unexpected不等
#define CHECK_NQ(reg, unexpected, bit)  \
    "li t2," #unexpected "\n" \
    "bne t2," #reg ",1f\n" \
    "ori a0,a0," #bit "\n" \
    "1:\n"

//测试组名
#define TEST_GROUP(name,bit) \
    #name":\n"

//测试结束
#define TEST_END() \
    "test_end:\n" \
    "li t0, 0\n" \
    "li t1, 0\n" \
    "li t2, 0\n" \
    "li x11, 0\n" \
    "li x12, 0\n" \
    "li x13, 0\n" \
    "li x14, 0\n" \
    "li x15, 0\n" \
    "ebreak\n"

#endif
