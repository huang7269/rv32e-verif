# Bug 记录

> 每个 bug 记录:现象、根因、修复、如何发现。**"发现方式"是验证能力的直接体现**,面试时会被重点询问。
>
> 发现方式分类:代码走查 / lint 警告 / 实测暴露 / 变异测试 / 波形分析。

---

## BUG-01 ALU 操作数 A 悬空

- **日期**:2026-10
- **模块**:rtl/rv32e_scpu.v
- **现象**:addi 结果错误。`addi x10, x0, 1` 恰好通过(rs1=x0 恒为 0),但 `addi x10, x10, 2` 读不到 x10 的值,结果为错误值
- **根因**:声明了 `wire rd_1` 但从未 `assign rd_1 = rdata_1`,ALU 第一操作数未驱动
- **修复**:`assign rd_1 = rdata_1;`
- **发现方式**:代码走查(初版评审)
- **验证**:修复后 x10 按 0→1→3→6 正确变化,波形逐周期确认

## BUG-02 pmem_write 字节掩码失效

- **日期**:2026-10
- **模块**:dpi_mem/pmem.cpp
- **现象**:4 字节全写到对齐地址的最低字节,高 3 字节数据丢失
- **根因**:循环体内写成 `mem_buf[align_addr + 0]`,应为 `align_addr + i`
- **修复**:改为 `mem_buf[align_addr + i]`
- **发现方式**:代码走查
- **验证**:尚未执行到(store 指令第 5 周实现),修复后由 load/store 测试回归验证

## BUG-03 B 型立即数缺最低位 0

- **日期**:2026-10
- **模块**:rtl/imm_gen.v
- **现象**:分支偏移减半,跳转目标错误(beq +8 实际跳 +4)
- **根因**:IMM_B 拼接公式少 `1'b0`(imm[0] 硬连线为 0),符号扩展位数 20 应为 19
- **修复**:`{{19{inst[31]}}, inst[31], inst[7], inst[30:25], inst[11:8], 1'b0}`
- **发现方式**:代码走查(对照 RISC-V 手册 B 型编码位表逐位核对)
- **验证**:尚未执行到(分支指令第 4 周实现),修复后由分支测试回归验证

## BUG-04 alu case 无 default(锁存器隐患)

- **日期**:2026-10
- **模块**:rtl/alu.v
- **现象**:alu_ctrl 为未实现值时 alu_result 保持原值,综合出 latch
- **根因**:组合逻辑 `always @(*)` 未覆盖全路径
- **修复**:加 `default: alu_result = 32'd0;`
- **发现方式**:代码走查 + Verilator lint
- **验证**:编译无 LATCH 警告

## BUG-05 寄存器堆声明 17 个寄存器

- **日期**:2026-10
- **模块**:rtl/regfile.v
- **现象**:多浪费 1 个寄存器(功能无影响)
- **根因**:`regfiles [0:REG_N]` 应为 `[0:REG_N-1]`
- **修复**:改为 `[0:REG_N-1]`
- **发现方式**:代码走查

## BUG-06 pmem 边界检查 off-by-one

- **日期**:2026-10
- **模块**:dpi_mem/pmem.cpp
- **现象**:addr 恰为 PMEM_SIZE 时越界读/写 4 字节(未定义行为)
- **根因**:`addr > PMEM_SIZE` 应为 `addr >= PMEM_SIZE`
- **修复**:read/write 两处均改为 `>=`
- **发现方式**:代码走查

## BUG-07 波形开关参数解析失效

- **日期**:2026-10
- **模块**:Makefile + sim/tb_main.cpp
- **现象**:`WAVE=0`(默认)时 wave.vcd 照样生成;Makefile 传 `+wave` 但 tb 不解析
- **根因**:tb 只认 `--no-wave` 且默认开启波形,两侧参数约定不一致
- **修复**:tb 默认关闭,统一解析 `+wave`/`--no-wave`;同时用 sim_time 变量统一 dump 时间轴,消除"previous dump"时间戳回退警告
- **发现方式**:实测暴露(运行后 wave.vcd 意外存在)
- **验证**:WAVE=0 无 wave.vcd;WAVE=1 正常生成且无 dump 警告

## BUG-08 link.ld 未生效(.data/.bss 布局错误)

- **日期**:2026-10
- **模块**:Makefile + tests/link.ld
- **现象**:链接脚本是死文件,全局变量不会放入规划的 DMEM 区(0x200000);`_stack_top` 符号不存在,栈无法初始化
- **根因**:LDFLAGS 用 `-Ttext=0x0` 而非 `-T link.ld`;link.ld 本身还缺 .sdata/.sbss 处理(RISC-V GCC 默认 `-msmall-data-limit=8`,小全局变量进 .sdata)、.rodata 误放 IMEM
- **修复**:LDFLAGS 改用 `-Wl,-T,tests/link.ld`;link.ld 补 .sdata/.sbss/.srodata、.rodata 移入 DMEM、加 ENTRY(_start) 和 _sbss/_ebss
- **发现方式**:代码走查
- **验证**:objdump -h 确认 .data/.bss VMA 为 0x200000 区

## BUG-09 checker 寄存器索引反转

- **日期**:2026-10
- **模块**:rtl/regfile.v + checker/
- **现象**:测试写 x10=6,checker 却在 C++ 下标 5 读到 6——期望比对全部错位,正确程序也报 FAIL
- **根因**:regfiles 改成 packed 二维数组 `reg [0:15][31:0]` 后,Verilator 将其拍平为 512 位向量,C++ 侧元素 0 对应最低 32 位(即 Verilog 的 regfiles[15]),索引完全反转
- **修复**:改回 unpacked 声明 `reg [31:0] regfiles [0:15] /*verilator public*/`,Verilator 以 VlUnpacked 暴露且保持声明序
- **发现方式**:实测暴露(运行 checker 后 reg[5] 出现 x10 的值)
- **验证**:修复后 x10 比对正确;变异测试(临时编译 x10=1 的错误测试)正确报 FAIL、退出码 3

## BUG-10 测试程序超前 RTL(未实现指令静默空转)

- **日期**:2026-10
- **模块**:tests/dummy.c
- **现象**:测试改用 `_start → main()` 结构后,编译出 sw/auipc/jal 等未实现指令,这些指令静默空转:main() 从未执行、sp 为垃圾值,仿真却打印"正常结束"——**程序逻辑全错,仿真照样通过**
- **根因**:测试程序与 RTL 进度不匹配(当时的 RTL 只实现 addi/ebreak);且 checker 尚不存在,无任何结果比对
- **修复**:测试退回与 RTL 对齐的简单版本;含栈/函数调用的版本留待第 5 周启用
- **发现方式**:波形分析(人工检查发现 main 未执行)
- **经验**:此 bug 是 checker 必要性的最直接证明——只检测"跑到 ebreak"远不够,必须比对结果

## BUG-11 缺 exp 参数时 argv 越界读

- **日期**:2026-10
- **模块**:sim/tb_main.cpp
- **现象**:只传 hex 不传 exp 时 `argv[2]` 越界读,`fopen(NULL)` 行为未定义(glibc 恰好不崩)
- **根因**:参数个数检查 `argc < 2` 未随参数增加而更新
- **修复**:改为 `argc < 3`,用法提示补齐 exp 文件
- **发现方式**:边界测试(手动运行缺参命令)

## BUG-12 .gitignore 忽略整个测试目录

- **日期**:2026-10
- **模块**:.gitignore
- **现象**:`/tests` 把测试源码整个忽略,新测试将无法入库;已跟踪的 dummy.hex 不受 ignore 影响持续出现在 git status
- **根因**:忽略了源码目录而非产物
- **修复**:改为忽略产物(`*.hex *.o *.elf obj_dir/ wave.vcd`);`git rm --cached tests/dummy.hex` 移出跟踪
- **发现方式**:git status 检查

## BUG-13 data_mem 组合逻辑锁存器 ×2

- **日期**:2026-10
- **模块**:sim/data_mem.sv
- **现象**:编译报 `%Warning-LATCH`:`rdata` 和 `mem_to_reg` 在 mem_read=0 时保持原值
- **根因**:`always @(*)` 中 `if(mem_read)` 无 else,未覆盖全路径
- **修复**:块首默认赋值 `data = 32'h0; rdata = 32'h0;` 再进 if
- **发现方式**:Verilator lint 警告
- **验证**:修复后 LATCH 警告清零

## BUG-14 load 字节/半字选择忽略地址偏移

- **日期**:2026-10
- **模块**:sim/data_mem.sv
- **现象**:`lb t1, 1(t0)` 读到的是字节 0 的值;tests_ls 自检 a0 bit0 置位(wmask 组全挂)
- **根因**:`pmem_read` 返回 4 字节对齐整字,但 load 逻辑固定取 `rdata[7:0]`,未按 `addr[1:0]` 选择
- **修复**:`rdata[addr[1:0]*8 +: 8]`(字节)、`rdata[addr[1]*16 +: 16]`(半字)
- **发现方式**:定向测试自检(tests_ls 错误位编码,a0=0x7 一次暴露三组错误)
- **验证**:修复后 a0 bit0 清零

## BUG-15 store wmask 忽略地址偏移

- **日期**:2026-10
- **模块**:sim/data_mem.sv
- **现象**:`sb t2, 1(t0)` 写进字节 0;tests_ls 自检 a0 bit0/bit2 置位
- **根因**:sb/sh 的 wmask 恒为 `8'h1`/`8'h3`,未按 `addr[1:0]` 移位
- **修复**:`8'h1 << addr[1:0]`、`8'h3 << addr[1:0]`
- **发现方式**:定向测试自检
- **验证**:修复后 pmem 调试打印确认 `sb @1` 只写 byte1

## BUG-16 store 数据未按偏移复制(连锁暴露)

- **日期**:2026-10
- **模块**:sim/data_mem.sv
- **现象**:修完 BUG-15 后,非零偏移的 sb/sh 会写入错误数据(pmem_write 对 wmask 第 i 位写 `wdata >> i*8`,数据在 wdata[7:0] 但写入 byte1 取的是 wdata[15:8])
- **根因**:store 数据未按字节/半字复制到全字位宽
- **修复**:`{4{wdata[7:0]}}`(sb)、`{2{wdata[15:0]}}`(sh)——复制 + 掩码选择的经典组合
- **发现方式**:修复 BUG-15 后推演暴露的连锁问题
- **验证**:tests_ls 写掩码验证组通过(sw 0x11223344 + sb 0x99@1 → 读回 0x11229944)

## BUG-17 期望值笔误(golden value bug)

- **日期**:2026-10
- **模块**:tests/tests_ls.c
- **现象**:checker 报 `[FAIL] x10 = 0x4`;测试自检 a0 bit2 置位,疑似 store-load 通路错误
- **根因**:sb 掩码验证的期望值手推错误:`0x11223344` 的 byte1(0x33)被 sb 0x99 替换后应为 `0x11229944`,测试却写 `0x11993344`(错改了 byte2)。**DUT 正确,期望值错**
- **修复**:期望值改为 `0x11229944`
- **发现方式**:checker FAIL → pmem 调试打印排除写路径 → 波形读回确认 lw=0x11229944 → 手算字节序定位笔误
- **经验**:本项目第一个 golden value bug。定位流程是"判断 DUT 错还是期望值错"的完整示范:先怀疑 RTL,用打印/波形逐层排除,最后手算字节序。difftest 时代此类判断是日常。

## BUG-18 TEST_GROUP 宏 %= 编译失败

- **日期**:2026-10
- **模块**:tests/isa/macros.h
- **现象**:`unrecognized opcode 'test_sll_%='`,所有使用 TEST_GROUP 的测试编译失败
- **根因**:gcc 13.3.0 不展开内联汇编模板中的 `%=`(实测 x86/riscv 两个编译器均不处理),标签原样传给汇编器
- **修复**:TEST_GROUP/TEST_END 改用普通命名标签(组名在单文件内唯一,无需 %= 唯一后缀)
- **发现方式**:编译错误 + 最小复现实验

## BUG-19 测试 clobber sp(x2)触发栈序言,负地址写内存

- **日期**:2026-10
- **模块**:tests/isa/*.c
- **现象**:运行时报 `[pmem_write error] -8/-4 over PMEM_SIZE`
- **根因**:测试把 x2(sp)当数据寄存器并写入 clobber 列表,GCC 被迫生成栈保护代码(addi sp,sp,-12; sw ra,8(sp)),而 sp 初始为 0 → 栈写到负地址。编译警告已明示:"listing the stack pointer register 'x2' in a clobber list is deprecated"
- **修复**:确立寄存器约定——测试数据只用 x11~x15(a1~a5),不碰 x1~x4(ra/sp/gp/tp);配合 noreturn + __builtin_unreachable
- **发现方式**:运行时 pmem_write 边界报错 + 编译警告
- **经验**:以前测试无 clobber 列表时用 x1"侥幸能跑"是对编译器隐瞒真相,一旦诚实声明 clobber,GCC 立即要保护 ra——规范测试必须避开 ABI 语义寄存器

## BUG-20 测试设计:auipc 相对校验差 4 字节

- **日期**:2026-10
- **模块**:tests/isa/alu_utype.c
- **现象**:checker 报 a0=0x18(auipc 两个组全 FAIL)
- **根因**:`auipc x11,1` 与 `auipc x12,0` 位于不同地址(相差 4 字节),两者 pc 基准不同,相减得 0xffc 而非 0x1000
- **修复**:补偿 4 字节 pc 差(sub 后 addi 4/-4)
- **发现方式**:checker FAIL + 手推指令地址

## BUG-21 测试设计:jal/jalr 链接值目标错位

- **日期**:2026-10
- **模块**:tests/isa/jump.c
- **现象**:checker 报 a0=0x11(链接校验两组全 FAIL)
- **根因**:链接值 = 跳转指令地址+4,即紧随其后那条指令的地址;测试却把目标标签放在下下条(中间隔了防护跳转)
- **修复**:链接校验的目标标签紧贴跳转指令;跳转本身是否发生由独立的 x0-rd 组覆盖
- **发现方式**:checker FAIL + 手推链接值语义

## BUG-22 测试地址边界笔误(0x3fffffc 超 PMEM 范围)

- **日期**:2026-10
- **模块**:tests/isa/load_store.c、doc/testplan.md
- **现象**:checker 报 a0=0x8(边界组 FAIL),0x3fffffc 处 lw 读回 0
- **根因**:`0x3fffffc`(约 64MB)远超 PMEM_SIZE(4MB),正确的最高对齐字是 `0x003ffffc`;**pmem 边界检查正确拦截了越界写**,mem 保持 0
- **修复**:测试与 testplan 均改为 0x003ffffc
- **发现方式**:checker FAIL → 波形确认读回 0 → 核对 PMEM_SIZE 发现地址位数笔误
- **经验**:负面收获——此 bug 反向验证了 pmem 边界保护真实生效

## BUG-23 GAS 拒绝负 auipc 立即数

- **日期**:2026-10
- **模块**:tests/isa/alu_utype.c
- **现象**:`Error: lui expression not in range 0..1048575`(`auipc x11, -1`)
- **根因**:GAS 对 auipc 立即数只接受无符号 20 位范围
- **修复**:改用 `0xfffff`(位型相同,RTL 侧 pc+0xfffff000 = pc-0x1000)
- **发现方式**:编译错误
