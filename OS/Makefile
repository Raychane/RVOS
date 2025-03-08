include ../common.mk

# 定义构建目录
BUILD_DIR = build

# 创建构建目录
$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(BUILD_DIR)/arch/riscv)
$(shell mkdir -p $(BUILD_DIR)/kernel/core)
$(shell mkdir -p $(BUILD_DIR)/kernel/mm)
$(shell mkdir -p $(BUILD_DIR)/kernel/syscall)
$(shell mkdir -p $(BUILD_DIR)/drivers)
$(shell mkdir -p $(BUILD_DIR)/lib)
$(shell mkdir -p $(BUILD_DIR)/user/tasks)

# 定义包含路径
INCLUDE_PATH = -I./include -I./include/kernel -I.

# 架构相关源文件
SRCS_ASM = \
    arch/riscv/start.S \
    arch/riscv/mem.S \
    arch/riscv/entry.S

<<<<<<< Updated upstream
SRCS_C = \
	kernel.c \
	uart.c \
	printf.c \
	page.c \
	sched.c \
	malloc.c \
	user.c \
	trap.c \
	plic.c \
	timer.c \
	lock.c \
=======
# C源文件
SRCS_KERNEL = \
    kernel/core/kernel.c \
    kernel/core/task.c \
    kernel/core/trap.c \
    kernel/core/lock.c \
    kernel/mm/page.c \
    kernel/mm/malloc.c \
    kernel/syscall/syscall.c
>>>>>>> Stashed changes

SRCS_DRIVERS = \
    drivers/uart.c \
    drivers/plic.c \
    drivers/timer.c

SRCS_LIB = \
    lib/printf.c \
    lib/algorithm.c

SRCS_USER = \
    user/tasks/user.c

SRCS_C = $(SRCS_KERNEL) $(SRCS_DRIVERS) $(SRCS_LIB) $(SRCS_USER)

# 生成目标文件路径
OBJS_ASM = $(patsubst %.S, $(BUILD_DIR)/%.o, $(SRCS_ASM))
OBJS_C = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS_C))
OBJS = $(OBJS_ASM) $(OBJS_C)

# 默认目标
.DEFAULT_GOAL := all
all: $(BUILD_DIR)/os.elf

# 编译规则
$(BUILD_DIR)/os.elf: $(OBJS)
	$(CC) $(CFLAGS) -T os.ld -o $@ $^
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/os.bin

# 编译C源文件规则
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -c -o $@ $<

# 编译汇编源文件规则
$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -c -o $@ $<

# 构建和运行
.PHONY: run
run: all
	@$(QEMU) -M ? | grep virt >/dev/null || exit
	@echo "Press Ctrl-A and then X to exit QEMU"
	@echo "------------------------------------"
	@$(QEMU) $(QFLAGS) -kernel $(BUILD_DIR)/os.elf

# 调试
.PHONY: debug
debug: all
	@echo "Press Ctrl-C and then input 'quit' to exit GDB and QEMU"
	@echo "-------------------------------------------------------"
	@$(QEMU) $(QFLAGS) -kernel $(BUILD_DIR)/os.elf -s -S &
	@$(GDB) $(BUILD_DIR)/os.elf -q -x ../gdbinit

# 生成反汇编代码
.PHONY: code
code: all
	@$(OBJDUMP) -S $(BUILD_DIR)/os.elf | less

# 生成文本文件
.PHONY: txt
txt: all
	@$(OBJDUMP) -S $(BUILD_DIR)/os.elf > os.txt

# 清理
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf os.txt
