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
