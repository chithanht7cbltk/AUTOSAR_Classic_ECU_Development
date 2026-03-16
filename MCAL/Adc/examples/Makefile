CC = arm-none-eabi-gcc
TARGET = main_combined
BUILD_DIR = build

CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -g3 -Wall -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -I../ -I../../ -I../../../spl/inc -I../../../bsp/cmsis -I../../../include
LDFLAGS = -T ../../../stm32f103.ld -nostdlib

SRCS = ../Adc.c ../Adc_Cfg.c ../../../startup_stm32f103.s ../../../spl/src/stm32f10x_adc.c ../../../spl/src/stm32f10x_rcc.c ../../../spl/src/stm32f10x_gpio.c ../../../spl/src/stm32f10x_dma.c ../../../spl/src/misc.c

SRCS_C = $(filter %.c,$(SRCS))
SRCS_S = $(filter %.s,$(SRCS))

OBJS_C = $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS_C:.c=.o)))
OBJS_S = $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS_S:.s=.o)))
OBJS_MAIN = $(BUILD_DIR)/$(TARGET).o
OBJS = $(OBJS_MAIN) $(OBJS_C) $(OBJS_S)

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: ../%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: ../../../spl/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: ../../../%.s
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)
