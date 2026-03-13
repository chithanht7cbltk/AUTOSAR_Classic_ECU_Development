# ============================================
#  STM32F103C8T6 + SPL + Renode support
#  Sample Project Makefile
# ============================================

# ===========================
# Project & toolchain
# ===========================
BUILDDIR      := build
TARGET_NAME   := FIRMWARE

TARGET        := $(BUILDDIR)/$(TARGET_NAME)

CROSS         ?= arm-none-eabi-
CC            := $(CROSS)gcc
AS            := $(CROSS)gcc
OBJCOPY       := $(CROSS)objcopy
OBJDUMP       := $(CROSS)objdump
SIZE          := $(CROSS)size

# ===========================
# MCU / CMSIS / SPL
# ===========================
# STM32F103 (Cortex-M3)
CPUFLAGS      := -mcpu=cortex-m3 -mthumb -mfloat-abi=soft

# Default Defines
DEFINES_BASE  := -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 \
                 -DRTE_DEVICE_STDPERIPH_RCC -DRTE_DEVICE_STDPERIPH_GPIO

# Detect Build Mode (Thêm define RUN_ON_RENODE nếu chạy Renode)
ifeq ($(TARGET_ENV),renode)
  DEFINES_MODE := -DRUN_ON_RENODE
else
  DEFINES_MODE :=
endif

DEFINES       := $(DEFINES_BASE) $(DEFINES_MODE)

# Include Directories
INC_DIRS := \
  platform/include \
  platform/spl/inc \
  platform/bsp/cmsis \
  src

INCLUDES := $(addprefix -I, $(INC_DIRS))

# ===========================
# C/ASM/LD flags
# ===========================
CFLAGS_COMMON := -O2 -g3 -Wall -Wextra -Wno-unused-parameter \
                 -ffreestanding -fno-builtin \
                 -ffunction-sections -fdata-sections \
                 -MMD -MP

CFLAGS        := $(CPUFLAGS) $(DEFINES) $(INCLUDES) $(CFLAGS_COMMON)
ASFLAGS       := $(CPUFLAGS) $(DEFINES) $(INCLUDES) -x assembler-with-cpp

# Linker Script
LDSCRIPT      := platform/bsp/linker/stm32f103.ld

LDFLAGS       := -T$(LDSCRIPT) -nostartfiles -nostdlib -static \
                 -Wl,--gc-sections -Wl,-Map=$(TARGET).map
LDFLAGS      += -specs=nano.specs -specs=nosys.specs

LDLIBS        := -Wl,--start-group -lc -lm -lgcc -Wl,--end-group

# ===========================
# Sources Configuration
# ===========================
SRCS_C := \
  $(wildcard platform/spl/src/*.c) \
  $(wildcard platform/bsp/cmsis/*.c) \
  $(wildcard platform/debug/*.c) \
  $(wildcard src/*.c)

# Startup Code
SRCS_S := \
  platform/bsp/startup_stm32f10x_md.s

# ===========================
# Objects / Deps
# ===========================
OBJS_C := $(patsubst %.c,$(BUILDDIR)/%.o,$(SRCS_C))
OBJS_S := $(patsubst %.s,$(BUILDDIR)/%.o,$(filter %.s,$(SRCS_S))) \
          $(patsubst %.S,$(BUILDDIR)/%.o,$(filter %.S,$(SRCS_S)))

OBJS   := $(OBJS_C) $(OBJS_S)
DEPS   := $(OBJS_C:.o=.d)

# ===========================
# Default goal
# ===========================
.PHONY: all
all: $(TARGET).bin size

# ===========================
# Compile rules
# ===========================
$(BUILDDIR)/%.o: %.c
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(AS) $(CPUFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.S
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(AS) $(ASFLAGS) -c $< -o $@

# ===========================
# Link
# ===========================
$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CC) $(CPUFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# ===========================
# BIN/HEX/SIZE/Listing
# ===========================
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

.PHONY: size
size: $(TARGET).elf
	$(SIZE) --format=berkeley $<

.PHONY: list
list: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $(TARGET).list

# ===============================
# Nạp firmware (OpencOCD)
# ===============================
.PHONY: flash
flash: $(TARGET).elf
	@test -f $< || { echo "ELF not found: $<"; exit 2; }
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
		-c "adapter speed 2000" \
		-c "reset_config none separate" \
		-c "init; halt" \
		-c "stm32f1x options_read 0" \
		-c "program {$(abspath $<)} verify" \
		-c "reset run; shutdown"

# ===============================
# Clean
# ===============================
.PHONY: clean
clean:
	@echo "Cleaning target: $(TARGET_NAME)..."
	@-del /F /Q $(subst /,\,$(TARGET).elf) $(subst /,\,$(TARGET).bin) $(subst /,\,$(TARGET).hex) $(subst /,\,$(TARGET).map) $(subst /,\,$(TARGET).list) 2>nul
	@-del /F /Q $(subst /,\,$(OBJS)) $(subst /,\,$(DEPS)) 2>nul
	@echo "Done."

.PHONY: clean-all
clean-all:
	@echo "Cleaning entire build directory..."
	@-rmdir /S /Q $(subst /,\,$(BUILDDIR)) 2>nul
	@echo "Done."

# ===============================
# Help
# ===============================
.PHONY: help
help:
	@echo "STM32F103 Sample Build System"
	@echo ""
	@echo "  make                          Build FIRMWARE"
	@echo "  make flash                    Flash firmware to hardware"
	@echo "  make clean                    Clean artifacts"
	@echo ""
-include $(DEPS)
