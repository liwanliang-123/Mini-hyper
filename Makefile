# 交叉编译工具链前缀
CROSS_COMPILE = aarch64-linux-gnu-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
AR = $(CROSS_COMPILE)ar

# 构建目录
BUILD_DIR = build

# 编译选项
ARCH_FLAGS = -march=armv8-a+nosimd+nofp
CFLAGS = $(ARCH_FLAGS) -ffreestanding -Wall -Wextra -Wfatal-errors -Werror -Wno-psabi -O0 -g3 -D__LITTLE_ENDIAN
CFLAGS += -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-override-init
ASFLAGS = $(ARCH_FLAGS) -ffreestanding -Wextra -Wfatal-errors -Werror -O0 -g3 -D__ASSEMBLY__

# 头文件路径
INCLUDES = -I./hypervisor/include

# 源文件
SRCS_ASM = ./hypervisor/src/head.S
SRCS_C   = ./hypervisor/src/main.c
OBJS     = $(BUILD_DIR)/head.o $(BUILD_DIR)/main.o

# 链接脚本生成
LDS_SRC = ./hypervisor/src/lds/linker.ld.S
LDS_OUT = $(BUILD_DIR)/linker.ld

# 最终目标
TARGET_ELF = $(BUILD_DIR)/Mini-Hyper.elf
TARGET_BIN = $(BUILD_DIR)/Mini-Hyper
MAP_FILE   = $(BUILD_DIR)/Mini_Hyper.map

# 静态库
LIB_NAME = $(BUILD_DIR)/libx_hyper_libs.a

# 默认目标
all: $(TARGET_BIN)

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 编译汇编文件
$(BUILD_DIR)/head.o: $(SRCS_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $(INCLUDES) -c $< -o $@

# 编译 C 文件
$(BUILD_DIR)/main.o: $(SRCS_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 创建静态库
$(LIB_NAME): $(OBJS) | $(BUILD_DIR)
	$(AR) rcs $@ $^

# 预处理链接脚本
$(LDS_OUT): $(LDS_SRC) | $(BUILD_DIR)
	cpp -P -E -D__ASSEMBLY__ $(INCLUDES) $< -o $@

# 链接生成 ELF
$(TARGET_ELF): $(LIB_NAME) $(LDS_OUT)
	$(LD) -pie -Map $(MAP_FILE) -T$(LDS_OUT) $(LIB_NAME) -o $@

# 生成原始二进制
$(TARGET_BIN): $(TARGET_ELF)
	$(OBJCOPY) -O binary -R .note -R .note.gnu.build-id -R .comment -S $< $@

# 清理
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean