# 测试计划(Testplan)

> 测试点矩阵是本项目的验证规划核心:每条指令拆解为最小颗粒度的测试点,每个测试点对应测试用例与完成状态。
> **新指令接入流程:先填矩阵 → 实现 RTL → 按矩阵写测试 → 回归 PASS → 勾选状态。**

## 一、测试点设计原则

- **操作数边界**:0、1、-1(全 F)、最大/最小值(0x7fffffff / 0x80000000)
- **立即数边界**:0、±1、+2047 / -2048(12 位立即数极值,符号扩展的关键边界)
- **x0 特例**:x0 作 rs1 恒读 0;x0 作 rd 写不进去(结果丢弃)
- **符号扩展**:负立即数参与运算后的 32 位结果
- **移位边界**:移位量 0、1、31(注意 RV32E 移位量只取低 5 位)
- **分支方向**:每条分支指令跳 / 不跳两个方向各测一次
- **有无符号区分**:slt/sltu、slti/sltiu、blt/bltu、bge/bgeu 用负数操作数区分

## 二、测试协议(统一约定)

- **结果寄存器**:a0(x10);`a0 == 0` 表示全部测试点通过
- **错误位编码**:每个测试组对应一个错误位,组内任一点失败置位——一组失败不掩盖其他组
- **期望文件**:统一 `x10 0x0`
- **寄存器约定**:数据寄存器用 x11~x15;x1~x4(ra/sp/gp/tp)不占用;t0~t2 为宏内部寄存器
- **测试宏**:tests/isa/macros.h(TEST_GROUP / CHECK_EQ / CHECK_NQ / TEST_END)

## 三、测试点矩阵

状态:✅ 已覆盖 | ❌ 缺失(待补) | 🔲 计划中

### ALU 指令(addi / R 型 / I 型)

| 功能点 | 测试点 | 取值/场景 | 测试用例 | 状态 |
|---|---|---|---|---|
| addi | 正立即数 | +1/+2/+3 | isa/smoke_addi.c | ✅ |
| addi | 负立即数 | -1(0xfff) | isa/smoke_addi.c | ✅ |
| addi | 立即数边界 | +2047 / -2048 | isa/smoke_addi.c | ✅ |
| addi | x0 作 rs1 | 恒读 0 | isa/smoke_addi.c | ✅ |
| addi | x0 作 rd | 写不进去 | isa/smoke_addi.c | ✅ |
| R 型 | 10 条指令基本运算 | 小正数组合 | isa/alu_rtype.c | ✅ |
| R 型 | sub 负数结果(借位) | 4-5 = -1 | isa/alu_rtype.c | ✅ |
| R 型 | slt/sltu 有无符号区分 | 正数 vs 0xffffffff | isa/alu_rtype.c | ✅ |
| R 型 | sra/srl 负数移位 | -1>>4 vs -1>>>4 | isa/alu_rtype.c | ✅ |
| R 型 | x0 作 rd | add x0, x1, x2 结果丢弃 | isa/alu_rtype.c | ✅ |
| R 型 | 移位量边界 | 0 和 31 | isa/alu_rtype.c | ✅ |
| R 型 | 加减溢出回绕 | 0x7fffffff+1 = 0x80000000 | isa/alu_rtype.c | ✅ |
| I 型 | 8 条指令基本运算 | 小立即数组合 | isa/alu_itype.c | ✅ |
| I 型 | -1 立即数符号扩展 | xori/ori/andi/slti 带 -1 | isa/alu_itype.c | ✅ |
| I 型 | 移位指令 funct7 混入立即数 | srai 立即数 = 0x404,移位量取低 5 位 | isa/alu_itype.c | ✅ |
| I 型 | 立即数边界 | +2047 / -2048 | isa/alu_itype.c | ✅ |
| I 型 | 移位量边界 | 0 和 31 | isa/alu_itype.c | ✅ |

### U 型 / 分支 / 跳转

| 功能点 | 测试点 | 取值/场景 | 测试用例 | 状态 |
|---|---|---|---|---|
| lui | 基本装载 | imm=1 → 0x1000,低 12 位为 0 | isa/alu_utype.c | ✅ |
| lui | 最大值 | imm=0xfffff → 0xfffff000 | isa/alu_utype.c | ✅ |
| lui | x0 作 rd | 结果丢弃 | isa/alu_utype.c | ✅ |
| auipc | pc+imm | 相对校验(补偿 4 字节 pc 差) | isa/alu_utype.c | ✅ |
| auipc | 负立即数 | 0xfffff → pc-0x1000,结果为负 | isa/alu_utype.c | ✅ |
| beq | 跳 | x11==x12 跳 | isa/branch.c | ✅ |
| beq | 不跳 | 不等不跳 | isa/branch.c | ✅ |
| beq | 负偏移(向后跳) | 循环回跳 | isa/branch.c | ✅ |
| beq | x0 与 x0 | 恒等,恒跳 | isa/branch.c | ✅ |
| bne | 跳 | 20≠21 跳 | isa/branch.c | ✅ |
| bne | 不跳 | 相等不跳 | isa/branch.c | ✅ |
| bne | 负偏移(向后跳) | 循环回跳 | isa/branch.c | ✅ |
| blt | 正负比较 | -5 < 3 跳 | isa/branch.c | ✅ |
| blt | 负数比较 | -5 < -1 跳 | isa/branch.c | ✅ |
| blt | 相等时不跳 | 5<5 为假 | isa/branch.c | ✅ |
| bge | 正数比较 | 100 ≥ 50 跳 | isa/branch.c | ✅ |
| bge | 负数比较 | -1 ≥ -5 跳 | isa/branch.c | ✅ |
| bge | 相等时跳(≥含相等) | 5 ≥ 5 | isa/branch.c | ✅ |
| bltu | 无符号比较 | 1 < 0x80000000 跳 | isa/branch.c | ✅ |
| bltu | 相等时不跳 | 5<5 为假 | isa/branch.c | ✅ |
| bgeu | 无符号比较 | 0xfffff000 ≥ 0x7ffff000 跳 | isa/branch.c | ✅ |
| bgeu | 相等时跳(≥含相等) | 5 ≥ 5 | isa/branch.c | ✅ |
| jal | 链接 = pc+4 | 链接值与目标地址比对 | isa/jump.c | ✅ |
| jal | 负偏移 | 向后跳 | isa/jump.c | ✅ |
| jal | x0 作 rd | 丢弃链接地址 | isa/jump.c | ✅ |
| jalr | 基本跳转 | 目标 = rs1+imm | isa/jump.c | ✅ |
| jalr | **地址对齐** | 奇数地址目标 → (rs1+imm)&~1 | isa/jump.c | ✅ |
| jalr | 链接 rd | 链接值与目标地址比对 | isa/jump.c | ✅ |
| jalr | x0 作 rd | 丢弃链接地址 | isa/jump.c | ✅ |
| jalr | 立即数偏移 | imm=8 落点精确区分 | isa/jump.c | ✅ |

### load/store / 系统

| 功能点 | 测试点 | 取值/场景 | 测试用例 | 状态 |
|---|---|---|---|---|
| lw | 字读取 | 4 字节对齐地址 | isa/load_store.c | ✅ |
| lb / lbu | 符号扩展 vs 零扩展 | 读 0x80:0xFFFFFF80 vs 0x80 | isa/load_store.c | ✅ |
| lh / lhu | 符号扩展 vs 零扩展 | 读 0x8000:0xFFFF8000 vs 0x8000 | isa/load_store.c | ✅ |
| sw | 字写入 | 4 字节对齐地址 | isa/load_store.c | ✅ |
| sb | 字节写入 | 4 种字节位置(地址低 2 位 00/01/10/11) | isa/load_store.c | ✅ |
| sh | 半字写入 | 2 种半字位置 | isa/load_store.c | ✅ |
| 综合 | store→load 一致性 | 同粒度回环 sb→lb/lbu、sh→lh/lhu、sw→lw | isa/load_store.c | ✅ |
| 综合 | 写掩码验证 | sb/sh 只改目标字节/半字(0x11229944 / 0x1234CCDD) | isa/load_store.c | ✅ |
| 综合 | 跨粒度验证 | 整字写 → 分字节/半字读 | isa/load_store.c | ✅ |
| 综合 | 地址边界 | 0x0、0x003ffffc(PMEM 最高对齐字) | isa/load_store.c | ✅ |
| 综合 | 全局变量 | .sdata/.bss 落在 DMEM 区,验证 link.ld | prog/global_var.c | ✅(第 5 周验收) |
| 系统 | ecall | halt 处理(与 ebreak 同) | isa/ecall.c | ✅ |

## 四、新指令接入流程

1. **填矩阵**:在上表中新增指令的测试点行(先写文档再写代码)
2. **实现 RTL**
3. **写测试**:`tests/isa/<name>.c` + 同名 `.exp`(统一 `x10 0x0`)
4. **回归**:`make run HEX=isa/<name>.hex`,全部 PASS
5. **勾状态**:矩阵行 ✅
6. **提交**:milestone commit(如 "add branch instructions, tests all pass!")

## 五、覆盖率映射(第 8 周)

第 8 周的功能覆盖率直接从此矩阵推导:

| 矩阵测试点 | coverpoint |
|---|---|
| 每条指令被执行过 | `cp_instr_type: coverpoint inst_type` |
| 分支跳/不跳 | `cp_branch_dir: coverpoint taken` |
| x0 作 rs1/rd | `cp_x0_usage` |
| 立即数正/负 | `cp_imm_sign` |
| 4 种字节掩码 | `cp_wmask` |
