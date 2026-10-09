# RV32E IC 验证学习计划(12 周)

> **目标**:以 rv32e-verif 项目为载体,完成从"能跑"到"能验证"的转变,达到大厂 IC 验证岗位面试要求。
>
> **双线并行**:
> - **C++ difftest 线**(项目主线,Verilator):保证项目能跑、出结果、出覆盖率报告
> - **UVM 方法学线**(学习+实践,Questa/EDA Playground):面试核心竞争力
>
> 每周节奏:两条线各占一半时间,每天 2~3 小时。

---

## 一、项目当前状态与待修复清单

### 严重 Bug(必须修复)

| # | 位置 | 问题 | 影响 |
|---|---|---|---|
| 1 | rtl/rv32e_scpu.v:76 | `rd_1` 悬空,未连 `rdata_1` | ALU 操作数 A 未驱动,addi 结果错误 |
| 2 | dpi_mem/pmem.cpp:90 | `pmem_write` 所有字节写到 `align_addr+0`,应为 `align_addr+i` | store 指令写内存错误 |
| 3 | rtl/imm_gen.v:18 | IMM_B 缺最低位 `1'b0` | 分支目标地址错误 |
| 4 | rtl/alu_control.v:21 | 未处理 `alu_op=2'b10`(R 型),全部落入 default 变 ADD | 以后 sub 等 R 型指令静默错误 |
| 5 | rtl/alu.v:20 | case 无 default,`alu_result` 会保持 → 锁存器 | 综合出 latch,功能隐患 |

### 环境/工具链问题

| # | 位置 | 问题 | 修复方式 |
|---|---|---|---|
| 6 | Makefile + sim/tb_main.cpp:19 | tb 不解析 `+wave`,`WAVE=0` 时波形照样开 | 统一参数解析(`+wave` / `--no-wave`) |
| 7 | Makefile LDFLAGS | 用了 `-Ttext=0x0`,link.ld 是死文件 | 改用 `-T tests/link.ld` |
| 8 | .gitignore | `/tests` 把整个测试源码目录忽略 | 改为只忽略产物(`*.hex *.o *.elf obj_dir/ wave.vcd`) |
| 9 | sim/tb_main.cpp:45 | `max_cycles=100` 硬编码 | 改为命令行参数 |
| 10 | Makefile | `-Wno-fatal` 掩盖警告 | 加 `--Wall` |
| 11 | rtl/regfile.v:17 | `regfiles [0:REG_N]` 声明了 17 个 | 改为 `[0:REG_N-1]` |

### 验证方法学缺失(项目最大的短板)

- 没有 checker:只检查"跑到 ebreak",不检查任何寄存器/内存值
- 没有参考模型(golden model)
- 没有覆盖率
- 没有回归管理
- 没有测试计划文档
- README 只有一行标题

---

## 二、工具准备(开始前/随时)

- [ ] 注册 [EDA Playground](https://www.edaplayground.com)(免费在线跑 SV/UVM,学语法阶段用)
- [ ] 下载 Questa Intel FPGA Starter Edition(免费,支持 UVM,第 7 周前装好)
- [ ] 备书:《SystemVerilog 验证》(绿皮书)、《UVM 实战》(张强)
- [ ] 建立习惯:每完成一个功能就 git commit;每个 bug 记录到 `docs/bugs.md`

---

## 三、每周计划

### 第 1 周:修复 Bug + Checker 闭环 + SV 语法入门

**做什么(项目)**
- [ ] 修复严重 Bug #1~#5(见上方清单)
- [ ] 修复 pmem_read/pmem_write 的边界检查(`addr > PMEM_SIZE` 应为 `>=`)
- [ ] 建立 checker 最小闭环:ebreak 时 dump regfile x1~x15(用 Verilator 层次引用或 DPI export),与期望值对比,不匹配返回非零
  - dummy.c 的期望值:x10 = 6
- [ ] 修 Makefile:+wave/--no-wave 参数对齐、link.ld 接入、加 `--Wall`
- [ ] 修 .gitignore
- [ ] 建立 `docs/bugs.md`,记录每个 bug:发现时间、现象、根因、修复方式
- [ ] 补测试:dummy.c 之外增加一个负向用例(期望值故意写错,验证 checker 能报 FAIL)

**验收标准**
- `make run` 通过且 checker 真实生效:故意改错期望值,回归能报 FAIL
- Verilator 无 LATCH 警告

**学什么(SV)**
- SV 数据类型:`logic`/`bit`、整型、数组(定宽/动态/队列/关联数组)
- 过程块:`always_comb` / `always_ff` / `always` 的区别
- Verilator lint 警告类别(LATCH、WIDTH、UNOPTFLAT)及含义
- 练习:EDA Playground 上敲数组/过程块 demo

---

### 第 2 周:R 型指令 + SV OOP

**做什么(项目)**
- [ ] alu.v 补全 10 种运算(add/sub/sll/slt/sltu/xor/srl/sra/or/and)+ default
- [ ] alu_control 加 `2'b10` 分支(fun3 + fun7 区分 add/sub/sra)
- [ ] control.v 加 default 分支(非法指令的处理)
- [ ] rv32e_scpu.v 重命名 `rd_1`/`rd_2` → `operand_a`/`operand_b`(避免与指令字段 rd 混淆)
- [ ] 每条 R 型指令一个测试用例,覆盖边界值:
  - 操作数:0、1、-1(全 F)、最大值
  - x0 作为 rs1(恒读 0)、x0 作为 rd(写不进去,值不变)
  - sub 的借位、slt/sltu 的有无符号区别、sra/srl 的负数移位

**验收标准**
- 全部 R 型测试 PASS;写 sub 测试时能验证 alu_control 正确区分 add/sub

**学什么(SV)**
- OOP:class 定义与实例化、继承、虚方法(virtual)、多态
- 深拷贝(shallow copy)与 `copy`/`clone` 的区别
- 练习:用 class 写一个简单的 32 位寄存器模型 demo

---

### 第 3 周:I 型指令 + SV 随机化

**做什么(项目)**
- [ ] alu_control 的 `2'b01` 分支补全所有 fun3:slti/sltiu/xori/ori/andi/slli/srli/srai
- [ ] slli/srli/srai 的 shamt 处理(取 inst[24:20],RV32E 用低 5 位)
- [ ] 测试用例覆盖:
  - 立即数边界:0、1、-1、+2047、-2048
  - 符号扩展:负立即数参与运算后的 32 位结果
  - x0 作为 rs1/rd、srai 对负数的算术右移

**验收标准**
- I 型全部测试 PASS

**学什么(SV)**
- 随机化:`rand`/`randc`、`constraint`、`std::randomize`、随机种子(seed)
- 进程与同步:`fork-join`/`fork-join_any`/`fork-join_none`、mailbox、semaphore、event
- 练习:用 SV 写一个受约束的随机指令生成器 demo(寄存器 x0~x15、随机立即数)

---

### 第 4 周:U 型 + 分支跳转 + interface/clocking

**做什么(项目)**
- [ ] lui / auipc:control 加 opcode,imm_gen 已有
- [ ] **重构操作数选择**:废弃 `rd_2 = (alu_op==2'b01)? imm : rdata_2` 的写法,改独立信号 `alu_src_a`/`alu_src_b`(auipc 需要 pc+imm,lui 需要 imm+0)
- [ ] 分支:beq/bne/blt/bge/bltu/bgeu,加比较逻辑(branch 信号)
- [ ] 跳转:jal/jalr(jalr 注意 `(rs1+imm)&~1` 对齐、写回 pc+4 到 rd)
- [ ] pc_next 改为多路选择(pc+4 / pc+imm / (rs1+imm)&~1)
- [ ] 测试覆盖:
  - 每个分支指令:跳 / 不跳 两个方向
  - 负偏移(向后跳)、blt/bge 的负数比较、bltu/bgeu 的无符号比较
  - jal 链接 x1、jalr 的对齐、x0 作 rd(丢弃链接地址)

**验收标准**
- 分支跳转全部测试 PASS;能跑一个带循环的 C 测试(循环 10 次退出)

**学什么(SV)**
- interface、clocking block、modport、program block
- **重点理解**:tb 与 DUT 之间的采样/驱动竞争(race),clocking block 如何解决
- 练习:用 interface + clocking block 接一个简单 DUT,跑通驱动和采样

---

### 第 5 周:load/store + UVM 最小骨架

**做什么(项目)**
- [ ] data_mem.sv 补全,实例化进顶层,DPI 读写内存
- [ ] lb/lh/lw/lbu/lhu:符号/零扩展
- [ ] sb/sh/sw:wmask 生成(按地址低 2 位 + 宽度)
- [ ] ecall:简单处理(当 ebreak 对待)
- [ ] 测试覆盖:
  - 4 种 wmask(字节/半字/字的偏移组合)、字节/半字边界的 load
  - lb/lh 读负值的符号扩展、lbu/lhu 的零扩展
  - 先 store 再 load 的一致性(store-load 数据通路)
- [ ] 用 C 写一个带全局变量(放在 .data/.bss)的测试,验证 link.ld 生效

**验收标准**
- load/store 全部测试 PASS;.data 段地址正确(0x200000)

**学什么(UVM)**
- UVM 最小骨架:`test → env → agent(driver/monitor/sequencer) → scoreboard`,跑通 hello world
- `uvm_object` 与 `uvm_component` 的区别、`uvm_component_utils` 宏注册
- 练习:EDA Playground 上跑通第一个 UVM test,能打印 `uvm_info`

---

### 第 6 周:C++ 参考模型 + UVM 核心机制

**做什么(项目)**
- [ ] 用 C++ 写 RV32E 参考模型(ISS):逐条解析指令、维护寄存器状态,支持已实现的全部指令
- [ ] tb_main 接入逐指令对比:每条指令执行后对比 DUT regfile 与 ref model regfile
- [ ] mismatch 处理:打印 pc/inst/期望值/实际值,立即停止,返回非零
- [ ] 回归脚本骨架:`make regress` 跑全部测试并汇总

**验收标准**
- 所有已有测试在 difftest 模式下 PASS;故意注错一个 RTL 行为,ref model 能抓到

**学什么(UVM)**
- **factory 机制**:宏注册、`override`(类型重载)的几种方式、解决了什么问题
- **config_db**:`set`/`get` 的配对、scope 的理解
- **sequence/sequencer/driver 握手**:`seq_item_port`、`get_next_item`/`item_done`、sequence 分层
- 练习:给 hello world 加 sequence + factory override,替换 driver 验证 override 生效

---

### 第 7 周:随机 difftest + UVM phase/TLM

**做什么(项目)**
- [ ] 随机指令序列生成器(约束:寄存器 x0~x15、随机立即数、合法指令集)
- [ ] 随机种子控制:失败可复现(seed 打印 + 命令行指定)
- [ ] 长跑 difftest:几十万条指令无 mismatch
- [ ] `make regress` 完整化:PASS/FAIL 汇总表格,失败返回非零
- [ ] 每改一处 RTL 就跑全量回归(养成习惯)

**验收标准**
- 随机长跑无 mismatch;回归一键跑通,失败能一眼看到哪个 test 挂了

**学什么(UVM)**
- **phase 机制**:build/connect/run 等 phase 的执行顺序,哪些自顶向下、哪些自底向上
- **TLM**:analysis port、`uvm_tlm_fifo`、monitor → scoreboard 的数据通路
- **report 机制**:`uvm_info`/`uvm_warning`/`uvm_error`/`uvm_fatal`、verbosity
- 安装 Questa Intel FPGA Starter Edition,把 hello world 迁到本地跑
- 练习:hello world 加 TLM 连接 + 观察各 phase 执行顺序

---

### 第 8 周:覆盖率 + UVM 环境骨架

**做什么(项目)**
- [ ] C++ 环境功能覆盖率:指令类型分布、每个寄存器被写过、立即数正负、分支跳/不跳、4 种 wmask
- [ ] Verilator `--coverage` 行覆盖率作为补充
- [ ] 产出覆盖率报告(哪些点没覆盖、为什么)

**验收标准**
- 覆盖率报告能明确指出"哪些测试点缺失",并据此补测试

**学什么(SV/UVM)**
- 功能覆盖率:`covergroup`/`coverpoint`/`cross`、覆盖率选项
- 搭 rv32e 的 UVM 环境骨架:
  - interface(时钟、复位、pc/inst/写回信号)
  - driver:复位序列 + 驱动指令内存
  - monitor:每周期采样 pc/inst/regfile 写事务
- 练习:UVM 环境能驱动 DUT 完成复位并跑完 addi

---

### 第 9 周:README + UVM scoreboard + DPI 共用 ref model

**做什么(项目)**
- [ ] README 重写:项目背景、架构图、目录结构、编译运行方法、覆盖率报告、bug 记录链接
- [ ] 画架构图:DUT / ref model / tb 的框图(面试讲解用)
- [ ] docs/bugs.md 整理完整

**验收标准**
- 陌生人能按 README 从零跑通项目

**学什么(UVM)**
- UVM scoreboard:通过 analysis port 收 monitor 事务
- **DPI-C 集成:UVM 环境里调用 C++ 参考模型**(本项目最大亮点:两个环境共用同一 golden model)
- 练习:scoreboard 接上 C++ ref model,完成第一次对比,mismatch 报 uvm_error

---

### 第 10 周:回归完善 + UVM 随机 sequence + SVA

**做什么(项目)**
- [ ] 回归稳定化、边界测试补漏
- [ ] **debug 演练**:故意注入一个 bug,完整走一遍定位流程(看波形 → 分层定位 → 修复),把流程写进文档

**验收标准**
- debug 流程文档化,能讲清楚"挂掉的用例怎么查"

**学什么(UVM/SV)**
- UVM constrained random sequence:随机指令流
- covergroup 挂进 UVM 环境(coverage subscriber)
- **SVA 断言**:pc 恒 4 对齐、reg_write 时 rd≠x0、wmask 合法性、复位后 pc=0
- 练习:随机 sequence 跑通 + 覆盖率能收到 + 断言全部通过

---

### 第 11 周:面试材料 + UVM 环境全量跑通

**做什么(项目)**
- [ ] 整理面试材料:bug 记录、覆盖率报告、架构讲解稿(10 分钟版)
- [ ] UVM 环境与 C++ 环境跑同一组测试,交叉验证结论一致

**验收标准**
- 两环境对同一测试结论一致(不一致要能解释为什么)

**学什么(UVM)**
- UVM 环境全量跑通,修复遗留问题
- 复盘:C++ 环境 vs UVM 环境的架构取舍(面试必讲)
  - C++:贴近业界 CPU 验证真实做法(香山/一生一芯),性能好
  - UVM:标准化方法学,约束随机、覆盖率、可复用性
  - 共用 ref model 是亮点,重点讲

---

### 第 12 周:模拟面试

**做什么(项目)**
- [ ] 模拟面试:脱稿讲 10 分钟项目 + 回答高频问题
- [ ] 打磨项目展示(README 最终版、讲解稿)

**验收标准**
- 能脱稿讲完项目架构;高频问题全部能答

**学什么(复习)**
- UVM 高频问题:factory/config_db/phase/sequence 握手/TLM
- SV 高频问题:随机化与 seed/copy 与 clone/fork-join/覆盖率
- SVA:immediate vs concurrent、property 的写法

---

## 四、检查点(Checkpoint)

| 时间 | 检查内容 |
|---|---|
| 第 6 周末 | 能画出 C++ 环境架构图 + 讲清 difftest 原理(逐指令对比、mismatch 定位) |
| 第 10 周末 | 能画出 UVM 环境架构图 + 讲清 factory/config_db/phase/sequence 握手 |

---

## 五、面试自测清单(大厂验证岗高频)

1. 讲一下你的验证环境架构(C++ 和 UVM 两个版本,各画一遍框图)
2. factory 解决了什么问题?override 有哪几种方式?
3. config_db 的 set/get 为什么必须配对?scope 怎么理解?
4. sequence 和 driver 怎么握手?`get_next_item` 和 `get` 的区别?
5. UVM 有哪些 phase?执行顺序?哪些自顶向下?
6. monitor 到 scoreboard 为什么用 analysis port(TLM)?
7. 你的覆盖率怎么收的?哪些点没收到?为什么?
8. 随机测试怎么保证失败可复现(seed 管理)?
9. DUT 和 ref model 不一致时,你怎么判断谁对?怎么定位?
10. 你发现过哪些 bug?怎么发现的?怎么定位到根因?
11. 为什么 C++ 环境和 UVM 环境并存?各自的取舍?
12. 你写了哪些 SVA 断言?断言的采样时机?
13. 挂掉一条用例,你的 debug 流程是什么?
14. 你的测试计划怎么写?一条指令的测试点怎么列?

---

## 六、学习资源汇总

| 类型 | 资源 | 用途 |
|---|---|---|
| 书 | 《SystemVerilog 验证》(Chris Spear) | SV 语法 + 验证思想 |
| 书 | 《UVM 实战》(张强) | UVM 核心机制 + 实战 |
| 网站 | chipverify.com / verificationguide.com | SV/UVM 教程查漏 |
| 在线练习 | EDA Playground | 学语法阶段敲 demo |
| 视频 | 路科验证 V2 Pro | 系统学 UVM(收费,口碑最好) |
| 视频 | B 站搜 "UVM 验证" | 免费入门 |
| 工具 | Questa Intel FPGA Starter Edition | 本地跑 UVM(免费) |
| 工具 | Verilator + GTKWave | 项目主线仿真 + 波形 |
| 参考 | 一生一芯、香山 difftest 架构 | 理解业界 C++ 验证思路 |

---

## 七、通用建议

- **小步提交**:每完成一个功能点就 commit,commit message 写清楚动机
- **bug 记录是面试素材**:docs/bugs.md 持续维护,每个 bug 写四要素(现象/根因/修复/如何发现的)
- **回归是习惯不是任务**:第 7 周起,每改一处 RTL 必跑 `make regress`
- **每周复盘**:周末花 30 分钟回答"这周做了什么、学了什么、下周计划",写在 git commit 或笔记里
- **两条线都不放弃**:C++ 线保证项目结果,UVM 线保证面试深度,第 9 周的 DPI 共用 ref model 把两条线串起来,是最大亮点
