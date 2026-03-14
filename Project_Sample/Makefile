# ============================================
#  STM32F103C8T6 + SPL + Renode support
#  All-in-One Makefile (Build, Setup, Verify)
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
# Flash firmware (OpenOCD)
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
# Setup & Installation Targets
# ===============================

.PHONY: setup
setup:
	@echo "============================================"
	@echo "  STM32 Project Setup Guide"
	@echo "============================================"
	@echo ""
	@echo "Please install the following tools:"
	@echo ""
	@echo "  Windows:"
	@echo "    1. Python 3: https://python.org/downloads/"
	@echo "    2. ARM GCC: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads"
	@echo "    3. Make: winget install GnuWin32.Make"
	@echo "    4. Renode: winget install renode"
	@echo "    5. OpenOCD (optional): winget install openocd"
	@echo ""
	@echo "  macOS:"
	@echo "    1. brew install python cmake ninja arm-none-eabi-gcc make"
	@echo "    2. Renode: Download from https://renode.io/"
	@echo ""
	@echo "  Ubuntu/Linux:"
	@echo "    1. sudo apt install python3 make build-essential"
	@echo "    2. sudo apt install gcc-arm-none-eabi gdb-arm-none-eabi"
	@echo "    3. Renode: Download .deb from https://renode.io/"
	@echo "    4. sudo apt install openocd"
	@echo ""
	@echo "After installation, run: make verify"
	@echo ""

.PHONY: install
install:
	@echo "============================================"
	@echo "  Automated Installation (Windows)"
	@echo "============================================"
	@echo ""
	@echo "Installing Make..."
	@powershell -Command "if (!(Test-Path 'C:\Program Files (x86)\GnuWin32\bin\make.exe')) { Write-Host 'Downloading Make...'; Invoke-WebRequest -Uri 'https://gnulinux.mirror.liquidtelecom.com/gnuwin32/make-3.81-bin.zip' -OutFile '$$env:TEMP\make.zip' -UseBasicParsing; Expand-Archive -Path '$$env:TEMP\make.zip' -DestinationPath 'C:\make' -Force; $$userPath = [Environment]::GetEnvironmentVariable('Path', 'User'); if ($$userPath -notlike '*C:\make\bin*') { [Environment]::SetEnvironmentVariable('Path', '$$userPath;C:\make\bin', 'User') }; Write-Host 'Make installed!' } else { Write-Host 'Make already installed.' }"
	@echo ""
	@echo "Installing Renode..."
	@powershell -Command "$$msiPath = '$$env:TEMP\renode.msi'; if (!(Test-Path 'C:\Program Files\Renode\renode.exe') -and !(Test-Path 'C:\Program Files (x86)\Renode\renode.exe')) { Write-Host 'Downloading Renode...'; Invoke-WebRequest -Uri 'https://github.com/renode/renode/releases/download/v1.15.1/renode-1.15.1.msi' -OutFile $$msiPath -UseBasicParsing; Write-Host 'Installing Renode...'; Start-Process 'msiexec.exe' -ArgumentList '/i', $$msiPath, '/quiet', '/norestart' -Wait; Write-Host 'Renode installed!' } else { Write-Host 'Renode already installed.' }"
	@echo ""
	@echo "Installation complete! Run 'make verify' to check."
	@echo ""

.PHONY: verify
verify:
	@echo "============================================"
	@echo "  Verifying Toolchain Installation"
	@echo "============================================"
	@echo ""
	@echo "1. Python:"
	@python --version 2>&1 || echo "   [NOT FOUND] Install Python 3"
	@echo ""
	@echo "2. ARM GCC Toolchain:"
	@arm-none-eabi-gcc --version 2>&1 | head -1 || echo "   [NOT FOUND] Install ARM GCC"
	@echo ""
	@echo "3. Make:"
	@make --version 2>&1 | head -1 || echo "   [NOT FOUND] Install Make"
	@echo ""
	@echo "4. Renode:"
	@renode --version 2>&1 || echo "   [NOT FOUND] Install Renode"
	@echo ""
	@echo "============================================"
	@echo "  Verification Complete"
	@echo "============================================"

.PHONY: check
check: verify

.PHONY: add-path
add-path:
	@echo "Adding tools to PATH..."
	@powershell -Command "$$makePath = 'C:\Program Files (x86)\GnuWin32\bin'; $$renodePath = ''; if (Test-Path 'C:\Program Files\Renode\bin') { $$renodePath = 'C:\Program Files\Renode\bin' } elseif (Test-Path 'C:\Program Files (x86)\Renode\bin') { $$renodePath = 'C:\Program Files (x86)\Renode\bin' }; $$userPath = [Environment]::GetEnvironmentVariable('Path', 'User'); $$newPath = $$userPath; if ($$userPath -notlike '*$$makePath*') { $$newPath = '$$newPath;$$makePath'; Write-Host 'Make path added' }; if ($$renodePath -ne '' -and $$userPath -notlike '*$$renodePath*') { $$newPath = '$$newPath;$$renodePath'; Write-Host 'Renode path added' }; [Environment]::SetEnvironmentVariable('Path', $$newPath, 'User'); Write-Host 'PATH updated! Restart your terminal.'"

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
	@echo "Build Targets:"
	@echo "  make                          Build FIRMWARE"
	@echo "  make TARGET_ENV=renode        Build for Renode simulation"
	@echo "  make flash                    Flash firmware to hardware"
	@echo "  make size                     Show binary size"
	@echo "  make list                     Generate disassembly listing"
	@echo ""
	@echo "Setup Targets:"
	@echo "  make setup                    Show setup guide"
	@echo "  make install                  Auto-install tools (Windows)"
	@echo "  make verify                   Verify toolchain installation"
	@echo "  make add-path                 Add tools to system PATH"
	@echo ""
	@echo "Clean Targets:"
	@echo "  make clean                    Clean build artifacts"
	@echo "  make clean-all                Remove entire build directory"
	@echo ""

-include $(DEPS)
