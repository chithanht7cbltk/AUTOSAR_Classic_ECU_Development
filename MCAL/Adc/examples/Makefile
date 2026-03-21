CC          := arm-none-eabi-gcc
TARGET      ?= main_combined
BUILD_DIR   ?= build

ROOT_DIR    := ../..

CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -g3 -Wall -Wextra \
         -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER \
         -I.. -I$(ROOT_DIR) \
         -I$(ROOT_DIR)/platform/include \
         -I$(ROOT_DIR)/platform/spl/inc \
         -I$(ROOT_DIR)/platform/bsp/cmsis

LDFLAGS = -T $(ROOT_DIR)/platform/bsp/linker/stm32f103.ld \
          -nostartfiles -nostdlib \
          -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

SRCS = \
    $(TARGET).c \
    ../Adc.c \
    ../Adc_Cfg.c \
    $(ROOT_DIR)/platform/spl/src/system_stm32f10x.c \
    $(ROOT_DIR)/platform/spl/src/stm32f10x_rcc.c \
    $(ROOT_DIR)/platform/spl/src/stm32f10x_adc.c \
    $(ROOT_DIR)/platform/spl/src/stm32f10x_dma.c \
    $(ROOT_DIR)/platform/spl/src/misc.c \
    $(ROOT_DIR)/platform/bsp/startup_stm32f10x_md.s

SRCS_C = $(filter %.c,$(SRCS))
SRCS_S = $(filter %.s,$(SRCS))

OBJS_C = $(addprefix $(BUILD_DIR)/,$(notdir $(SRCS_C:.c=.o)))
OBJS_S = $(addprefix $(BUILD_DIR)/,$(notdir $(SRCS_S:.s=.o)))
OBJS   = $(OBJS_C) $(OBJS_S)

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: ../%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(ROOT_DIR)/platform/spl/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(ROOT_DIR)/platform/bsp/%.s
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)
