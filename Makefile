# ==================================================
#  路径配置
# ==================================================
TOP_MODULE  = rv32e_scpu
RTL_DIR     = ./rtl
SIM_DIR     = ./sim
DPI_DIR     = ./dpi_mem
CHECKER_DIR = ./checker
TEST_DIR    = ./tests
ISA_DIR     = $(TEST_DIR)/isa
PROG_DIR    = $(TEST_DIR)/prog
OBJ_DIR     = obj_dir
# ==================================================
#  工具链配置
# ==================================================
# RISC-V 编译工具链
RISCV_PREFIX = riscv64-linux-gnu
CC      = $(RISCV_PREFIX)-gcc
OBJCOPY = $(RISCV_PREFIX)-objcopy
CFLAGS = -march=rv32e -mabi=ilp32e -O2 -nostdlib -Wall -fno-pie -ffreestanding
LDFLAGS = -march=rv32e -mabi=ilp32e -nostdlib -Wl,-T,tests/link.ld -Wl,--build-id=none -Wl,-no-pie -Wl,-N


# Verilator 仿真工具
VERILATOR = verilator
VERILATOR_FLAGS = --cc --exe --build --trace
VERILATOR_FLAGS += -CFLAGS "-I../$(DPI_DIR)"
VERILATOR_FLAGS += -CFLAGS "-I../$(CHECKER_DIR)"
VERILATOR_FLAGS += -Wall
VERILATOR_FLAGS += --top-module $(TOP_MODULE)
VERILATOR_FLAGS += -Mdir $(OBJ_DIR)

# ==================================================
#  源文件列表
# ==================================================
# CPU 核心 RTL
RTL_SRCS := $(wildcard $(RTL_DIR)/*.v)

# 仿真模型 SV
SIM_SV_SRCS = \
	$(SIM_DIR)/inst_mem.sv \
	$(SIM_DIR)/data_mem.sv

# DPI 内存模型 + 仿真主入口
DPI_C_SRCS = $(DPI_DIR)/pmem.cpp
SIM_MAIN = $(SIM_DIR)/tb_main.cpp
CHECKER_C_SRCS = $(CHECKER_DIR)/checker.cpp
# 自动扫描测试用例
TEST_C_SRCS = $(wildcard $(ISA_DIR)/*.c) $(wildcard $(PROG_DIR)/*.c)
TEST_HEXS   = $(TEST_C_SRCS:.c=.hex)

# 最终目标
TARGET = $(OBJ_DIR)/V$(TOP_MODULE)

# ==================================================
#  目标定义
# ==================================================
# 默认目标：编译工程
all: $(TARGET)

# 编译仿真工程
$(TARGET): $(RTL_SRCS) $(SIM_SV_SRCS) $(DPI_C_SRCS) $(CHECKER_C_SRCS) $(SIM_MAIN)
	$(VERILATOR) $(VERILATOR_FLAGS) \
		$(RTL_SRCS) $(SIM_SV_SRCS) $(DPI_C_SRCS) $(CHECKER_C_SRCS) $(SIM_MAIN)

# 批量生成所有测试 hex
hex: $(TEST_HEXS)

# 模式规则：单个 .c → .hex
%.hex: %.c
	@echo "编译测试程序: $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $(@:.hex=.o)
	$(CC) $(LDFLAGS) $(@:.hex=.o) -o $(@:.hex=.elf)
	$(OBJCOPY) -O verilog $(@:.hex=.elf) $@
	rm -f $(@:.hex=.o) $(@:.hex=.elf)

# 运行仿真
HEX ?= isa/dummy.hex
EXP ?= $(HEX:.hex=.exp)
WAVE ?= 0

run: $(TARGET) $(TEST_DIR)/$(HEX)
	@test -f $(TEST_DIR)/$(EXP) || { echo "[ERROR] missing exp: $(TEST_DIR)/$(EXP)"; exit 1; }
	@if [ $(WAVE) -eq 1 ]; then \
		$(TARGET) $(TEST_DIR)/$(HEX) $(TEST_DIR)/$(EXP) +wave; \
	else \
		$(TARGET) $(TEST_DIR)/$(HEX) $(TEST_DIR)/$(EXP); \
	fi

wave:
	gtkwave wave.vcd

# 清理
clean:
	rm -rf $(OBJ_DIR) wave.vcd

clean_test:
	rm -f $(ISA_DIR)/*.hex $(ISA_DIR)/*.o $(ISA_DIR)/*.elf \
	      $(PROG_DIR)/*.hex $(PROG_DIR)/*.o $(PROG_DIR)/*.elf

clean_all: clean clean_test

.PHONY: all hex run wave clean clean_test clean_all
