# Makefile – AUTOSAR Classic COM Demo (STM32F103)
# Build: make
# Clean: make clean

TARGET_TX = node_tx
TARGET_RX = node_rx
CC     = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# STM32F103C8 (Medium-density)
CFLAGS  = -mcpu=cortex-m3 -mthumb -O0 -g3 -Wall -Wextra
CFLAGS += -ffreestanding -ffunction-sections -fdata-sections
CFLAGS += -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER
LDFLAGS = -T stm32f103.ld --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections

# ===== Include paths =====
INC  = -Iautosar/include
INC += -Iautosar/com
INC += -Iautosar/pdur
INC += -Iautosar/canif
INC += -Iautosar/can
INC += -Iautosar/lin
INC += -Iautosar/linif
INC += -Iautosar/cantp
INC += -Iconfig
INC += -Ispl/inc
INC += -I../bsp/cmsis

# ===== Source files =====
# Application
# Bỏ file main.c cũ, các file tx_main.c và rx_main.c được chèn vào OBJS_TX/RX
SRCS  = 

# AUTOSAR COM stack
SRCS += autosar/com/Com.c
SRCS += autosar/pdur/PduR.c
SRCS += autosar/canif/CanIf.c
SRCS += autosar/can/Can.c
SRCS += autosar/lin/Lin.c
SRCS += autosar/linif/LinIf.c
SRCS += autosar/cantp/CanTp.c

# Config
SRCS += config/Com_Cfg.c
SRCS += config/CanIf_Cfg.c
SRCS += config/PduR_Cfg.c
SRCS += config/CanTp_Cfg.c

# SPL (chỉ compile các module cần thiết)
SRCS += spl/src/stm32f10x_rcc.c
SRCS += spl/src/stm32f10x_gpio.c
SRCS += spl/src/stm32f10x_can.c
SRCS += spl/src/stm32f10x_usart.c
SRCS += spl/src/misc.c
SRCS += spl/src/system_stm32f10x.c

# Startup
ASM_SRCS = startup_stm32f103.s

# ===== Object files (chung) =====
# Không dùng file chung cho mảng có compile flag nên compile thẳng.
# Để đơn giản Makefile, ta sẽ build lại toàn bộ cho từng node với flag tương ứng.

# ===== Build rules =====
all: $(TARGET_TX).elf $(TARGET_TX).bin $(TARGET_RX).elf $(TARGET_RX).bin
	@$(SIZE) $(TARGET_TX).elf
	@$(SIZE) $(TARGET_RX).elf

tx: $(TARGET_TX).elf $(TARGET_TX).bin
rx: $(TARGET_RX).elf $(TARGET_RX).bin

# Phân tách file .o cho TX và RX
OBJS_TX = example/tx_main_tx.o $(SRCS:.c=_tx.o) $(ASM_SRCS:.s=_tx.o)
OBJS_RX = example/rx_main_rx.o $(SRCS:.c=_rx.o) $(ASM_SRCS:.s=_rx.o)

%_tx.o: %.c
	$(CC) $(CFLAGS) -DNODE_TX $(INC) -c $< -o $@

%_rx.o: %.c
	$(CC) $(CFLAGS) -DNODE_RX $(INC) -c $< -o $@

%_tx.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

%_rx.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_TX).elf: $(OBJS_TX)
	$(CC) $(CFLAGS) $(OBJS_TX) $(LDFLAGS) -o $@

$(TARGET_RX).elf: $(OBJS_RX)
	$(CC) $(CFLAGS) $(OBJS_RX) $(LDFLAGS) -o $@

$(TARGET_TX).bin: $(TARGET_TX).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET_RX).bin: $(TARGET_RX).elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(OBJS_TX) $(OBJS_RX) $(TARGET_TX).elf $(TARGET_TX).bin $(TARGET_RX).elf $(TARGET_RX).bin

.PHONY: all tx rx clean
