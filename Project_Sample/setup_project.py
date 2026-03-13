#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
STM32 Project Generator Tool
Supports: Windows, macOS, Ubuntu (Linux)
Description: Automates the creation of a standalone STM32 (F103) project with SPL,
             Makefile, VSCode configuration, and Renode simulation support.
"""

import os
import sys
import shutil
import platform
import subprocess
import glob

# ==========================================
# FILE TEMPLATES
# ==========================================

TEMPLATE_MAKEFILE = """\
# ============================================
#  STM32F103C8T6 + SPL + Renode support
#  Auto-Generated Makefile
# ============================================

# ===========================
# Project & toolchain
# ===========================
BUILDDIR      := build
TARGET_NAME   := {target_name}

TARGET        := $(BUILDDIR)/$(TARGET_NAME)

CROSS         ?= {cross_compile}
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
DEFINES_BASE  := -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 \\
                 -DRTE_DEVICE_STDPERIPH_RCC -DRTE_DEVICE_STDPERIPH_GPIO

# Detect Build Mode (Thêm define RUN_ON_RENODE nếu chạy Renode)
ifeq ($(TARGET_ENV),renode)
  DEFINES_MODE := -DRUN_ON_RENODE
else
  DEFINES_MODE :=
endif

DEFINES       := $(DEFINES_BASE) $(DEFINES_MODE)

# Include Directories
INC_DIRS := \\
  platform/include \\
  platform/spl/inc \\
  platform/bsp/cmsis \\
  src

INCLUDES := $(addprefix -I, $(INC_DIRS))

# ===========================
# C/ASM/LD flags
# ===========================
CFLAGS_COMMON := -O2 -g3 -Wall -Wextra -Wno-unused-parameter \\
                 -ffreestanding -fno-builtin \\
                 -ffunction-sections -fdata-sections \\
                 -MMD -MP

CFLAGS        := $(CPUFLAGS) $(DEFINES) $(INCLUDES) $(CFLAGS_COMMON)
ASFLAGS       := $(CPUFLAGS) $(DEFINES) $(INCLUDES) -x assembler-with-cpp

# Linker Script
LDSCRIPT      := platform/bsp/linker/stm32f103.ld

LDFLAGS       := -T$(LDSCRIPT) -nostartfiles -nostdlib -static \\
                 -Wl,--gc-sections -Wl,-Map=$(TARGET).map
LDFLAGS      += -specs=nano.specs -specs=nosys.specs

LDLIBS        := -Wl,--start-group -lc -lm -lgcc -Wl,--end-group

# ===========================
# Sources Configuration
# ===========================
SRCS_C := \\
  $(wildcard platform/spl/src/*.c) \\
  $(wildcard platform/bsp/cmsis/*.c) \\
  $(wildcard platform/debug/*.c) \\
  $(wildcard src/*.c)

# Startup Code
SRCS_S := \\
  platform/bsp/startup_stm32f10x_md.s

# ===========================
# Objects / Deps
# ===========================
OBJS_C := $(patsubst %.c,$(BUILDDIR)/%.o,$(SRCS_C))
OBJS_S := $(patsubst %.s,$(BUILDDIR)/%.o,$(filter %.s,$(SRCS_S))) \\
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
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(CPUFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# ===========================
# Link
# ===========================
$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@mkdir -p $(dir $@)
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
	@test -f $< || {{ echo "ELF not found: $<"; exit 2; }}
	{openocd_cmd} -f interface/stlink.cfg -f target/stm32f1x.cfg \\
		-c "adapter speed 2000" \\
		-c "reset_config none separate" \\
		-c "init; halt" \\
		-c "stm32f1x options_read 0" \\
		-c "program {{$(abspath $<)}} verify" \\
		-c "reset run; shutdown"

# ===============================
# Clean
# ===============================
.PHONY: clean
clean:
	@echo "Cleaning target: $(TARGET_NAME)..."
	@rm -f $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).map $(TARGET).list
	@rm -f $(OBJS) $(DEPS)
	@echo "Done."

.PHONY: clean-all
clean-all:
	@echo "Cleaning entire build directory..."
	rm -rf $(BUILDDIR)

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
"""

TEMPLATE_MAIN_C = """\
#include "stm32f10x.h"

// Hàm delay đơn giản (không dùng timer, không chính xác)
void Delay(volatile uint32_t nCount) {
    for(; nCount != 0; nCount--);
}

int main(void) {
    // 1. Cấp clock cho Port C
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // 2. Cấu hình chân PC13 là Output Push-Pull, tốc độ 50MHz
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 3. Vòng lặp nháy LED trên PC13
    while (1) {
        // Tắt LED (vì PC13 trên board Bluepill thường tích cực mức thấp: 1 = tắt, 0 = sáng)
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
        Delay(0x000FFFFF); // Chờ

        // Bật LED
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        Delay(0x000FFFFF); // Chờ
    }
}
"""

TEMPLATE_VSCODE_TASKS = """\
{{
    "version": "2.0.0",
    "tasks": [
        {{
            "label": "Make: build",
            "type": "shell",
            "command": "make",
            "args": [],
            "options": {{
                "cwd": "${{workspaceFolder}}"
            }},
            "group": {{
                "kind": "build",
                "isDefault": true
            }},
            "problemMatcher": [
                "$gcc"
            ],
            "presentation": {{
                "reveal": "always",
                "panel": "dedicated"
            }}
        }},
        {{
            "label": "Make: clean",
            "type": "shell",
            "command": "make",
            "args": [
                "clean"
            ],
            "options": {{
                "cwd": "${{workspaceFolder}}"
            }},
            "group": "none",
            "problemMatcher": []
        }},
        {{
            "label": "Make: build (Renode)",
            "type": "shell",
            "command": "make",
            "args": [
                "clean",
                "all",
                "TARGET_ENV=renode"
            ],
            "options": {{
                "cwd": "${{workspaceFolder}}"
            }},
            "problemMatcher": [
                "$gcc"
            ],
            "presentation": {{
                "reveal": "always",
                "panel": "dedicated"
            }}
        }},
        {{
            "label": "Renode: cleanup",
            "type": "shell",
            "command": "{kill_cmd}",
            "options": {{
                "cwd": "${{workspaceFolder}}"
            }},
            "presentation": {{
                "reveal": "silent",
                "panel": "shared"
            }}
        }},
        {{
            "label": "Renode: start stm32",
            "type": "shell",
            "command": "{renode_path}",
            "args": [
                "--console",
                "--disable-gui",
                "-e",
                "i @scripts/stm32_sample.resc"
            ],
            "options": {{
                "cwd": "${{workspaceFolder}}"
            }},
            "isBackground": true,
            "problemMatcher": {{
                "pattern": {{
                    "regexp": ".*"
                }},
                "background": {{
                    "activeOnStart": true,
                    "beginsPattern": ".*Renode, version.*",
                    "endsPattern": ".*Renode ready for GDB connection.*"
                }}
            }},
            "presentation": {{
                "reveal": "always",
                "panel": "shared"
            }}
        }},
        {{
            "label": "Renode: prepare",
            "dependsOn": [
                "Renode: cleanup",
                "Make: build (Renode)",
                "Renode: start stm32"
            ],
            "dependsOrder": "sequence"
        }}
    ]
}}"""

TEMPLATE_VSCODE_LAUNCH = """\
{{
    "version": "0.2.0",
    "configurations": [
        {{
            "name": "HW Debug",
            "type": "cortex-debug",
            "request": "launch",
            "servertype": "openocd",
            "serverpath": "{openocd_path}",
            "gdbPath": "{gdb_path}",
            "cwd": "${{workspaceFolder}}",
            "executable": "${{workspaceFolder}}/build/{target_name}.elf",
            "device": "STM32F103C8T6",
            "configFiles": [
                "interface/stlink.cfg",
                "target/stm32f1x.cfg"
            ],
            "svdFile": "${{workspaceFolder}}/platform/debug/STM32F103.svd",
            "runToEntryPoint": "main",
            "postLaunchCommands": [
                "monitor reset halt",
                "load"
            ],
            "postRestartCommands": [
                "monitor reset halt",
                "load"
            ],
            "showDevDebugOutput": "none",
            "liveWatch": {{
                "enabled": false
            }},
            "preLaunchTask": "Make: build"
        }},
        {{
            "name": "Renode Debug",
            "type": "cortex-debug",
            "request": "launch",
            "servertype": "external",
            "cwd": "${{workspaceFolder}}",
            "executable": "${{workspaceFolder}}/build/{target_name}.elf",
            "device": "STM32F103C8T6",
            "gdbPath": "{gdb_path}",
            "gdbTarget": "localhost:3333",
            "runToEntryPoint": "main",
            "postLaunchCommands": [
                "monitor reset",
                "monitor start"
            ],
            "showDevDebugOutput": "none",
            "liveWatch": {{
                "enabled": false
            }},
            "preLaunchTask": "Renode: prepare",
            "postDebugTask": "Renode: cleanup"
        }}
    ]
}}"""

TEMPLATE_VSCODE_CPP = """\
{{
    "configurations": [
        {{
            "name": "{os_name}",
            "includePath": [
                "${{workspaceFolder}}/**",
                "${{workspaceFolder}}/platform/include",
                "${{workspaceFolder}}/platform/spl/inc",
                "${{workspaceFolder}}/platform/bsp/cmsis",
                "{arm_include_1}",
                "{arm_include_2}"
            ],
            "defines": [
                "STM32F10X_MD",
                "USE_STDPERIPH_DRIVER",
                "HSE_VALUE=8000000"
            ],
            "compilerPath": "{gcc_path}",
            "cStandard": "gnu11",
            "cppStandard": "gnu++14",
            "intelliSenseMode": "{intellisense}"
        }}
    ],
    "version": 4
}}"""

TEMPLATE_RENODE_SCRIPT = """\
:name: STM32F103 Sample Debug
:description: Script chay Sample tren Renode

$elf_path ?= @build/{target_name}.elf

# 1. Tao may ao
mach create "stm32"

# 2. Nap cau hinh phan cung STM32F103
machine LoadPlatformDescription @platform/debug/stm32f103_full.repl

# 3. Log file
logFile @gpio_log.txt

# 4. Console
showAnalyzer sysbus.usart1

# 5. Button PA0 (Input Pull-up, Active Low)
machine LoadPlatformDescriptionFromString "userButtonPA0: Miscellaneous.Button @ gpioPortA 0 {{ invert: true; -> gpioPortA@0 }}"

# 6. LED PC13 (Active Low)
machine LoadPlatformDescriptionFromString "ledPC13: Miscellaneous.LED @ gpioPortC 13 {{ invert: true }}; gpioPortC: {{ 13 -> ledPC13@0 }}"

# 7. Nap file ELF
sysbus LoadELF $elf_path

# 8. Alias
alias btn_down "sysbus.gpioPortA.userButtonPA0 Press"
alias btn_up   "sysbus.gpioPortA.userButtonPA0 Release"
alias led_check "sysbus.gpioPortC.ledPC13 State"

# 9. GDB Server
machine StartGdbServer 3333
mach set "stm32"

logLevel 3 sysbus
echo "Renode ready for GDB connection on port 3333. Use 'start' to begin simulation."
"""

# ==========================================
# HELPER FUNCTIONS
# ==========================================

def get_os_kind():
    s = platform.system().lower()
    if 'linux' in s: return 'linux'
    if 'darwin' in s or 'mac' in s: return 'macos'
    if 'win' in s: return 'windows'
    return 'unknown'

def check_command(cmd):
    """Kiem tra xem mot lenh co ton tai trong PATH hay khong."""
    if get_os_kind() == 'windows':
        return shutil.which(cmd + ".exe") is not None or shutil.which(cmd) is not None
    return shutil.which(cmd) is not None

def prompt(text, default=""):
    """Hoi nguoi dung, chap nhan gia tri mac dinh."""
    val = input(f"{text} [{default}]: ").strip()
    return val if val else default

def find_arm_includes(gcc_path):
    """Co gang tuple 2 duong dan include cua ARM GCC tu duong dan GCC."""
    # Vi du: /path/to/bin/arm-none-eabi-gcc -> /path/to/arm-none-eabi/include
    bin_dir = os.path.dirname(gcc_path)
    base_dir = os.path.dirname(bin_dir)
    inc1 = os.path.join(base_dir, "arm-none-eabi", "include")
    
    # Tim thu muc lib/gcc/arm-none-eabi/VERSION/include
    inc2 = ""
    gcc_lib_dir = os.path.join(base_dir, "lib", "gcc", "arm-none-eabi")
    if os.path.exists(gcc_lib_dir):
        versions = os.listdir(gcc_lib_dir)
        if versions:
            # Chon version moi nhat or dau tien
            inc2 = os.path.join(gcc_lib_dir, versions[0], "include")
    
    # Dung tam thoi neu khong tim thay
    if not os.path.exists(inc1): inc1 = "${workspaceFolder}/**"
    if not os.path.exists(inc2): inc2 = "${workspaceFolder}/**"
    
    return inc1, inc2

# ==========================================
# MAIN LOGIC
# ==========================================

def main():
    print("=" * 60)
    print("STM32 + Renode C/C++ Project Generator")
    print("Su ho tro đa nen tang (Windows, macOS, Linux)")
    print("=" * 60)
    
    os_kind = get_os_kind()
    print(f"Phat hien he dieu hanh: {os_kind.upper()}")
    
    # 1. Thu thap thong tin tu nguoi dung
    project_dir = prompt("Thu muc tao project (tuong doi hoac tuyet doi)", "./STM32_MyProject")
    project_dir = os.path.abspath(project_dir)
    
    target_name = prompt("Ten ELF/BIN build ra", "FIRMWARE")
    
    # Cấu hình ARM GCC
    print("\n--- CAU HINH ARM TOOLCHAIN ---")
    default_gcc = shutil.which("arm-none-eabi-gcc") or ""
    gcc_path = prompt("Duong dan den 'arm-none-eabi-gcc' (de trong neu da co trong PATH)", default_gcc)
    while not gcc_path and not check_command("arm-none-eabi-gcc"):
        print("[Loi] Khong tim thay arm-none-eabi-gcc trong PATH. Vui long nhap duong dan tuyet doi!")
        gcc_path = prompt("Duong dan den 'arm-none-eabi-gcc'", "")
    
    if gcc_path:
        cross_compile = gcc_path.replace("gcc", "")
        # Thu them trailing slash neu cross_compile la folder => Phai su dung de prefix
        if os.path.isdir(cross_compile):
            cross_compile = os.path.join(cross_compile, "arm-none-eabi-")
    else:
        cross_compile = "arm-none-eabi-"
        gcc_path = shutil.which("arm-none-eabi-gcc")

    gdb_path = gcc_path.replace("gcc", "gdb")
    if not os.path.exists(gdb_path):
        gdb_path = prompt(f"Khong the tu dong tim thay gdb tu gcc. Nhap duong dan toi arm-none-eabi-gdb", "")
        
    inc1, inc2 = find_arm_includes(gcc_path)

    # Cấu hình OpenOCD
    print("\n--- CAU HINH OPENOCD (cho nap chip that) ---")
    default_openocd = shutil.which("openocd") or "openocd"
    openocd_path = prompt("Duong dan den 'openocd' (de trong neu da co trong PATH)", default_openocd)
    openocd_cmd = "openocd" if (openocd_path == "openocd" or not openocd_path) else f"\"{openocd_path}\""

    # Cấu hình Renode
    print("\n--- CAU HINH RENODE (cho mo phong) ---")
    default_renode = "renode"
    if os_kind == "macos" and os.path.exists("/Applications/Renode.app/Contents/MacOS/renode"):
        default_renode = "/Applications/Renode.app/Contents/MacOS/renode"
    elif check_command("renode"):
        default_renode = shutil.which("renode")
        
    renode_path = prompt("Duong dan den 'renode' (executable)", default_renode)

    # 2. Tao cau truc thu muc
    print(f"\nDang khoi tao project tai: {project_dir}")
    os.makedirs(os.path.join(project_dir, "src"), exist_ok=True)
    os.makedirs(os.path.join(project_dir, "scripts"), exist_ok=True)
    os.makedirs(os.path.join(project_dir, ".vscode"), exist_ok=True)
    
    platform_dir = os.path.join(project_dir, "platform")
    if not os.path.exists(platform_dir):
        print(f"[Canh bao] Chua co thu muc 'platform' (chua cac file SPL, CMSIS).")
        print(f"          Hay copy 'platform' tu OSEK-RTOS hoac thu muc Sample vao day: {platform_dir} sau khi chay xong tool nhe.")

    # 3. Ghi file Makefile
    make_content = TEMPLATE_MAKEFILE.format(target_name=target_name, cross_compile=cross_compile, openocd_cmd=openocd_cmd)
    with open(os.path.join(project_dir, "Makefile"), "w", encoding='utf-8') as f:
        f.write(make_content)
    print(" -> Tao Makefile thanh cong.")

    # 4. Ghi file main.c (neu chua co)
    main_c = os.path.join(project_dir, "src", "main.c")
    if not os.path.exists(main_c):
        with open(main_c, "w", encoding='utf-8') as f:
            f.write(TEMPLATE_MAIN_C)
        print(" -> Tao file src/main.c thanh cong.")
    else:
        print(" -> File src/main.c da ton tai, bo qua.")

    # 5. Ghi file Renode resc
    resc_file = os.path.join(project_dir, "scripts", "stm32_sample.resc")
    with open(resc_file, "w", encoding='utf-8') as f:
        f.write(TEMPLATE_RENODE_SCRIPT.format(target_name=target_name))
    print(" -> Tao config Renode stm32_sample.resc thanh cong.")

    # 6. .vscode/tasks.json
    # Lenh kill quy trinh Renode khac nhau tuy OS
    kill_cmd = "pkill -f 'Renode' 2>/dev/null; lsof -ti:3333 | xargs kill -9 2>/dev/null; sleep 0.5; echo '✅ Port 3333 da duoc don sach'"
    if os_kind == "windows":
        kill_cmd = "taskkill /F /IM Renode.exe /T 2>NUL || echo ✅ Port don sach"
    
    tasks_content = TEMPLATE_VSCODE_TASKS.format(kill_cmd=kill_cmd, renode_path=renode_path)
    with open(os.path.join(project_dir, ".vscode", "tasks.json"), "w", encoding='utf-8') as f:
        f.write(tasks_content)
    print(" -> Tao .vscode/tasks.json thanh cong.")

    # 7. .vscode/launch.json
    # Khong xai escape do f-string cua format kha nhay cam voi json, dung format tay
    launch_content = TEMPLATE_VSCODE_LAUNCH.format(openocd_path=openocd_path, target_name=target_name, gdb_path=gdb_path)
    with open(os.path.join(project_dir, ".vscode", "launch.json"), "w", encoding='utf-8') as f:
        f.write(launch_content)
    print(" -> Tao .vscode/launch.json thanh cong.")

    # 8. .vscode/c_cpp_properties.json
    intel_mode = "macos-gcc-arm"
    intel_os = "Mac"
    if os_kind == "windows":
        intel_mode = "windows-gcc-arm"
        intel_os = "Win32"
    elif os_kind == "linux":
        intel_mode = "linux-gcc-arm"
        intel_os = "Linux"

    cpp_content = TEMPLATE_VSCODE_CPP.format(
        os_name=intel_os,
        arm_include_1=inc1,
        arm_include_2=inc2,
        gcc_path=gcc_path,
        intellisense=intel_mode
    )
    with open(os.path.join(project_dir, ".vscode", "c_cpp_properties.json"), "w", encoding='utf-8') as f:
        f.write(cpp_content)
    print(" -> Tao .vscode/c_cpp_properties.json thanh cong.")

    print("\n" + "="*60)
    print("HOAN THANH SETUP!")
    print(f"Thu muc du an: {project_dir}")
    print("Dung quen copy thu muc 'platform' (chua thu vien SPL, CMSIS, SVD...) neu chua co nhe!")
    print("="*60)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nDa huy setup.")
        sys.exit(1)
