# rv32e-verif

RV32E 单周期 CPU 的仿真验证项目。

## 特性(当前进度)

- ✅ Verilator + C++ 仿真验证环境
- ✅ DPI-C 内存模型(pmem),支持 verilog-hex 格式程序加载
- ✅ 测试程序 C 交叉编译流程(riscv64-linux-gnu-gcc → hex)
- ✅ ebreak 后寄存器比对 checker(.exp 期望文件驱动,PASS/FAIL 判定 + 退出码)
- ✅ 已实现指令:addi、ebreak(详见[指令支持进度表](#指令支持进度表))
- 🚧 差分测试(difftest)与 C++ 参考模型 —— 计划中
- 🚧 功能覆盖率收集 —— 计划中
- 🚧 UVM 验证环境 —— 计划中

## 架构

当前仿真环境的架构(差分测试部分为计划中的扩展):

```
                       tests/*.c
                          │ riscv64-linux-gnu-gcc -march=rv32e + link.ld
                          ▼
                      tests/*.hex
                          │ 加载
               ┌──────────▼──────────┐
               │  pmem(C++ 内存模型) │◄──── DPI-C
               └──────────▲──────────┘
                  取指读  │  读写
               ┌──────────┴──────────┐
  tb_main ────>│  rv32e_scpu (DUT)   │
  (时钟/复位)  └──────────┬──────────┘
                          │ pc / inst / halt
                          ▼
                  仿真日志 + wave.vcd
                          │
                          ▼
              checker:ebreak 后比对 regfile
                          │
                          ▼
        (计划中) C++ 参考模型(ISS)逐指令比对(difftest)
```

- **tb_main.cpp**:仿真主入口,驱动时钟/复位,逐周期打印 pc/inst,检测 ebreak 停机
- **checker**:ebreak 后读取 DUT 内部 regfile,与 .exp 期望文件比对,FAIL 时退出码为 3
- **pmem**:统一 4MB 内存模型,指令与数据共用(与 tests/link.ld 的内存布局对应)
- **tests/\*.c**:测试程序,交叉编译为 hex 后加载进 pmem

## 目录结构

```
rv32e-verif/
├── rtl/               # DUT:RISC-V 单周期 CPU(RV32E)
│   ├── rv32e_scpu.v   #   顶层:取指/译码/执行/写回
│   ├── control.v      #   主控制单元(opcode 译码)
│   ├── alu_control.v  #   ALU 控制(fun3/fun7 译码)
│   ├── alu.v          #   运算器
│   ├── imm_gen.v      #   立即数生成(I/S/B/U/J 五种格式)
│   └── regfile.v      #   寄存器堆(16 个寄存器,x0 恒为 0)
├── sim/               # 验证环境(Verilator + C++)
│   ├── tb_main.cpp    #   仿真主入口:驱动/监测
│   ├── inst_mem.sv    #   指令内存(DPI-C 调用 pmem)
│   └── data_mem.sv    #   数据内存(计划中)
├── dpi_mem/           # DPI 内存模型(C++)
│   ├── pmem.h
│   └── pmem.cpp
├── checker/           # 结果比对(.exp 期望文件解析 + regfile 比对)
│   ├── checker.h
│   └── checker.cpp
├── tests/             # 测试程序(C → hex,同名 .exp 为期望值)
│   ├── dummy.c        #   addi + ebreak 冒烟测试
│   ├── dummy.exp      #   期望值:x10 0x6
│   └── link.ld        #   链接脚本:IMEM 0x0 / DMEM 0x200000
├── Makefile
└── ic_verif_study.md  # 12 周学习计划
```

## 快速上手

### 依赖

```bash
sudo apt install verilator gtkwave gcc-riscv64-linux-gnu
```

### 构建与运行

```bash
make        # 构建 Verilator 仿真工程
make hex    # 交叉编译测试程序生成 hex
make run    # 运行 tests/dummy.hex
```

预期输出(dummy.c 依次执行 3 条 addi 后 ebreak):

```
cycle=  0 | pc=0x00000000 | inst=0x00100513
cycle=  1 | pc=0x00000004 | inst=0x00250513
cycle=  2 | pc=0x00000008 | inst=0x00350513
cycle=  3 | pc=0x0000000c | inst=0x00100073
[INFO] enter EBREAK,clock cycle: 3
simulation succeeded!
```

### 期望文件(.exp)

每个测试程序配一个同名 `.exp` 文件,每行声明一个寄存器的期望值:`x<编号> <十六进制值>`,未声明的寄存器期望为 0。ebreak 后 checker 逐寄存器比对,FAIL 时退出码为 3。例如 `tests/dummy.exp`:

```
x10 0x6
```

运行指定测试时自动配对同名 .exp:

```bash
make run HEX=xxx.hex    # 自动加载 tests/xxx.exp
```

### 查看波形

```bash
make run WAVE=1   # 运行并生成 wave.vcd(默认关闭波形)
make wave         # 打开 GTKWave
```

常用 Makefile 目标:

| 目标 | 说明 |
|---|---|
| `make` | 构建仿真工程 |
| `make hex` | 批量编译 tests/ 下所有 .c 为 .hex |
| `make run HEX=xxx.hex` | 运行指定测试程序 |
| `make wave` | 打开波形 |
| `make clean` / `make clean_all` | 清理构建产物 / 全部产物 |

## 指令支持进度表

| 类型 | 指令 | 状态 | 测试用例 |
|---|---|---|---|
| I 型 | addi | ✅ 完成 | tests/dummy.c |
| 系统 | ebreak | ✅ 完成 | tests/dummy.c |
| R 型 | add / sub / sll / slt / sltu / xor / srl / sra / or / and | 🔲 计划(第 2 周) | — |
| I 型 | slti / sltiu / xori / ori / andi / slli / srli / srai | 🔲 计划(第 3 周) | — |
| U 型 | lui / auipc | 🔲 计划(第 4 周) | — |
| 分支 | beq / bne / blt / bge / bltu / bgeu | 🔲 计划(第 4 周) | — |
| 跳转 | jal / jalr | 🔲 计划(第 4 周) | — |
| 访存 | lb / lh / lw / lbu / lhu / sb / sh / sw | 🔲 计划(第 5 周) | — |
| 系统 | ecall | 🔲 计划(第 5 周) | — |

## 验证方法学(建设中)

### 测试计划

每条指令的测试点设计原则(逐步应用到 tests/ 中):

- **操作数边界**:0、1、-1(全 F)、最大/最小立即数
- **x0 特例**:x0 作为 rs1 恒读 0;x0 作为 rd 写不进去
- **符号扩展**:负立即数参与运算后的 32 位结果
- **分支方向**:跳 / 不跳两个方向各测一次

### 差分测试(difftest)

🚧 计划中(第 6~7 周):C++ 参考模型逐指令执行,与 DUT 的 regfile 比对,不一致即打印 pc/inst/期望值/实际值并退出。

### 随机测试

🚧 计划中(第 7 周):受约束随机指令序列生成,种子可控保证失败可复现。

### 覆盖率

🚧 计划中(第 8 周):指令类型、寄存器写回、分支方向、访存字节掩码等功能覆盖点。

### 回归

🚧 计划中(第 7 周):`make regress` 一键跑全部测试并汇总 PASS/FAIL。

## Bug 记录

已发现并修复 12 个 bug,完整记录(现象/根因/修复/发现方式)见 [bugs.md](bugs.md):

| 编号 | Bug | 发现方式 |
|---|---|---|
| BUG-01 | ALU 操作数 A 悬空 | 代码走查 |
| BUG-02 | pmem_write 字节掩码失效 | 代码走查 |
| BUG-03 | B 型立即数缺最低位 0 | 代码走查 |
| BUG-04 | alu case 无 default(锁存器) | 代码走查 + lint |
| BUG-05 | 寄存器堆声明 17 个 | 代码走查 |
| BUG-06 | pmem 边界检查 off-by-one | 代码走查 |
| BUG-07 | 波形开关参数解析失效 | 实测暴露 |
| BUG-08 | link.ld 未生效(.data/.bss 布局错误) | 代码走查 |
| BUG-09 | checker 寄存器索引反转 | 实测暴露 |
| BUG-10 | 测试程序超前 RTL(未实现指令静默空转) | 波形分析 |
| BUG-11 | 缺 exp 参数时 argv 越界读 | 边界测试 |
| BUG-12 | .gitignore 忽略整个测试目录 | git status 检查 |

## 已知限制

- .exp 解析对格式错误行静默跳过,暂无报错提示

## Roadmap

完整 12 周计划见 [ic_verif_study.md](ic_verif_study.md),摘要:

| 周次 | 里程碑 |
|---|---|
| 第 1 周 | 修复环境问题,建立 checker 闭环 |
| 第 2~3 周 | R 型 / I 型指令 + 逐指令测试 |
| 第 4~5 周 | U 型 / 分支跳转 / load-store |
| 第 6~7 周 | C++ 参考模型 + difftest + 回归 |
| 第 8 周 | 功能覆盖率收集 |
| 第 9~11 周 | UVM 验证环境(与 C++ 环境共用参考模型) |
| 第 12 周 | 面试材料整理 |

## 参考资料

- [RISC-V Unprivileged ISA Specification](https://riscv.org/technical/specifications/)
- [Verilator 官方文档](https://verilator.org/guide/latest/)
- [一生一芯计划](https://ysyx.oscc.cc/) —— C++ difftest 验证思路参考
