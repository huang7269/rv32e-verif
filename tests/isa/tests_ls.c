// RV32E 存取指令定向测试：lb/lh/lw/lbu/lhu/sb/sh/sw
// 内存范围：0x200000 ~ 0x200007（8字节，低于0x400000上限，可读写、字对齐）
// 结束方式：全部测试完成后执行 ebreak 停止
// 结果寄存器：a0(x10)，0=全通过；bit0=wmask错误，bit1=扩展错误，bit2=通路错误
void _start(void)
{
    __asm__ volatile(
        "li   t0, 0x200000\n"     // t0 = 测试内存基地址 (x5)
        "li   a0, 0\n"            // a0 = 结果寄存器，初始清零 (x10)

    // ==============================================================
    // 测试1：4种wmask + 字节/半字/字边界加载
    // 错误位：a0[0]
    // ==============================================================
    "test_wmask_%=:\n"
        "li   t2, 0xDEADBEEF\n"
        "sw   t2, 0(t0)\n"

        // 字节4种偏移 → 4种单字节读掩码
        "lb   t1, 0(t0)\n"
        "li   t2, 0xFFFFFFEF\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x01\n"
        "1:\n"

        "lb   t1, 1(t0)\n"
        "li   t2, 0xFFFFFFBE\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x01\n"
        "1:\n"

        "lb   t1, 2(t0)\n"
        "li   t2, 0xFFFFFFAD\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x01\n"
        "1:\n"

        "lb   t1, 3(t0)\n"
        "li   t2, 0xFFFFFFDE\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x01\n"
        "1:\n"

        // 半字2种对齐偏移 → 2种半字读掩码
        "lh   t1, 0(t0)\n"
        "li   t2, 0xFFFFBEEF\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x01\n"
        "1:\n"

        "lh   t1, 2(t0)\n"
        "li   t2, 0xFFFFDEAD\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x01\n"
        "1:\n"

        // 字对齐加载 → 全字读掩码
        "lw   t1, 0(t0)\n"
        "li   t2, 0xDEADBEEF\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x01\n"
        "1:\n"

    // ==============================================================
    // 测试2：符号扩展(lb/lh) vs 零扩展(lbu/lhu)
    // 错误位：a0[1]
    // ==============================================================
    "test_extend_%=:\n"
        // 写入符号边界测试向量
        "li   t2, 0x80\n"
        "sb   t2, 0(t0)\n"
        "li   t2, 0x7F\n"
        "sb   t2, 1(t0)\n"
        "li   t2, 0x8000\n"
        "sh   t2, 2(t0)\n"
        "li   t2, 0x7FFF\n"
        "sh   t2, 4(t0)\n"

        // 字节负数场景
        "lb   t1, 0(t0)\n"
        "li   t2, 0xFFFFFF80\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x02\n"
        "1:\n"

        "lbu  t1, 0(t0)\n"
        "li   t2, 0x00000080\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x02\n"
        "1:\n"

        // 字节正数场景
        "lb   t1, 1(t0)\n"
        "li   t2, 0x0000007F\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x02\n"
        "1:\n"

        "lbu  t1, 1(t0)\n"
        "li   t2, 0x0000007F\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x02\n"
        "1:\n"

        // 半字负数场景
        "lh   t1, 2(t0)\n"
        "li   t2, 0xFFFF8000\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x02\n"
        "1:\n"

        "lhu  t1, 2(t0)\n"
        "li   t2, 0x00008000\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x02\n"
        "1:\n"

        // 半字正数场景
        "lh   t1, 4(t0)\n"
        "li   t2, 0x00007FFF\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x02\n"
        "1:\n"

        "lhu  t1, 4(t0)\n"
        "li   t2, 0x00007FFF\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x02\n"
        "1:\n"

    // ==============================================================
    // 测试3：Store-Load 数据通路一致性
    // 错误位：a0[2]
    // ==============================================================
    "test_store_load_%=:\n"
        "sw   zero, 0(t0)\n"
        "sw   zero, 4(t0)\n"

        // 同粒度回环：sb → lb/lbu
        "li   t2, 0x78\n"
        "sb   t2, 0(t0)\n"
        "lb   t1, 0(t0)\n"
        "li   t2, 0x00000078\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        "lbu  t1, 0(t0)\n"
        "li   t2, 0x00000078\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        // 同粒度回环：sh → lh/lhu
        "li   t2, 0xABCD\n"
        "sh   t2, 2(t0)\n"
        "lh   t1, 2(t0)\n"
        "li   t2, 0xFFFFABCD\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        "lhu  t1, 2(t0)\n"
        "li   t2, 0x0000ABCD\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        // 同粒度回环：sw → lw
        "li   t2, 0xCAFEBABE\n"
        "sw   t2, 4(t0)\n"
        "lw   t1, 4(t0)\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        // 写掩码验证：sb仅修改指定字节
        "li   t2, 0x11223344\n"
        "sw   t2, 0(t0)\n"
        "li   t2, 0x99\n"
        "sb   t2, 1(t0)\n"
        "lw   t1, 0(t0)\n"
        "li   t2, 0x11229944\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        // 写掩码验证：sh仅修改指定半字
        "li   t2, 0xAABBCCDD\n"
        "sw   t2, 0(t0)\n"
        "li   t2, 0x1234\n"
        "sh   t2, 2(t0)\n"
        "lw   t1, 0(t0)\n"
        "li   t2, 0x1234CCDD\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        // 跨粒度验证：整字写入 → 分字节/半字读取
        "li   t2, 0x11223344\n"
        "sw   t2, 0(t0)\n"

        "lbu  t1, 0(t0)\n"
        "li   t2, 0x44\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        "lbu  t1, 1(t0)\n"
        "li   t2, 0x33\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        "lbu  t1, 2(t0)\n"
        "li   t2, 0x22\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        "lbu  t1, 3(t0)\n"
        "li   t2, 0x11\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        "lhu  t1, 0(t0)\n"
        "li   t2, 0x3344\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

        "lhu  t1, 2(t0)\n"
        "li   t2, 0x1122\n"
        "beq  t1, t2, 1f\n"
        "ori  a0, a0, 0x04\n"
        "1:\n"

    // 清零临时寄存器，匹配仿真对比规则
    "test_end_%=:\n"
        "li   t0, 0\n"
        "li   t1, 0\n"
        "li   t2, 0\n"
        "ebreak\n"

        :
        :
        : "t0", "t1", "t2", "a0", "memory"
    );
}

