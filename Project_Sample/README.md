# STM32F103 Sample Project

Project mẫu STM32F103C8T6 với thư viện Standard Peripheral Library (SPL) và hỗ trợ giả lập Renode.

---

## Mục lục

- [Yêu cầu hệ thống](#yêu-cầu-hệ-thống)
- [Cài đặt](#cài-đặt)
- [Cấu trúc project](#cấu-trúc-project)
- [Sử dụng](#sử-dụng)
- [Troubleshooting](#troubleshooting)

---

## Yêu cầu hệ thống

### Phần mềm bắt buộc

| Tool | Mục đích | Link |
|------|----------|------|
| **ARM GCC** | Biên dịch code ARM Cortex-M3 | [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) |
| **Make** | Build system | [GnuWin32 Make](http://gnuwin32.sourceforge.net/packages/make.htm) |
| **Python 3** | Scripting (optional) | [python.org](https://www.python.org/downloads/) |

### Phần mềm tùy chọn

| Tool | Mục đích | Link |
|------|----------|------|
| **Renode** | Giả lập vi điều khiển | [renode.io](https://renode.io/) |
| **OpenOCD** | Nạp firmware vào mạch thật | [openocd.org](http://openocd.org/) |
| **VSCode** | IDE với Cortex-Debug extension | [code.visualstudio.com](https://code.visualstudio.com/) |

---

## Cài đặt

### Windows (Tự động)

Chạy lệnh sau để cài đặt tự động:

```powershell
make install
```

Hoặc cài thủ công qua winget:

```powershell
winget install GnuWin32.Make
winget install Arm.GCC
winget install renode
winget install openocd
```

### Windows (Thủ công)

1. **Python 3**: Tải từ [python.org/downloads](https://www.python.org/downloads/)
2. **ARM GCC**: Tải `.exe` từ [Arm Developer](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads), chọn "Add to PATH"
3. **Make**: Tải từ [GnuWin32](http://gnuwin32.sourceforge.net/packages/make.htm) hoặc `winget install GnuWin32.Make`
4. **Renode**: Tải `.msi` từ [renode.io](https://renode.io/)
5. **OpenOCD** (tùy chọn): `winget install openocd`

Sau khi cài, chạy:

```bash
make verify
```

### macOS

```bash
# Cài Homebrew nếu chưa có
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Cài toolchain
brew install cmake ninja
brew tap ArmMbed/homebrew-formulae
brew install arm-none-eabi-gcc make

# Renode: tải .dmg từ renode.io, kéo vào /Applications
```

### Ubuntu/Linux

```bash
# Cập nhật và cài đặt
sudo apt update
sudo apt install python3 make build-essential
sudo apt install gcc-arm-none-eabi gdb-arm-none-eabi binutils-arm-none-eabi

# Renode: tải .deb từ https://github.com/renode/renode/releases
sudo dpkg -i renode_*.deb
sudo apt --fix-broken install

# OpenOCD
sudo apt install openocd
```

---

## Cấu trúc project

```
Project_Sample/
├── Makefile              # Build system (tất cả trong 1)
├── README.md             # Tài liệu này
├── src/
│   └── main.c            # Code chính (LED blink)
├── platform/             # Thư viện SPL & CMSIS (phải copy vào)
│   ├── bsp/              # Board Support Package
│   │   ├── cmsis/        # CMSIS core
│   │   ├── linker/       # Linker script
│   │   └── startup_*.s   # Startup code
│   ├── debug/            # SVD file & debug scripts
│   ├── include/          # Header files
│   └── spl/              # Standard Peripheral Library
│       ├── inc/          # SPL headers
│       └── src/          # SPL source
├── scripts/
│   └── stm32_sample.resc # Renode simulation script
└── build/                # Output directory (tự tạo khi build)
    ├── FIRMWARE.elf
    ├── FIRMWARE.bin
    └── FIRMWARE.hex
```

> **⚠️ Quan trọng**: Thư mục `platform/` không được đóng kèm do kích thước lớn. Bạn cần copy từ project gốc OSEK-RTOS hoặc gói STM32F10x_StdPeriph_Lib vào.

---

## Sử dụng

### Build firmware

```bash
# Build thường
make

# Build cho Renode (với define RUN_ON_RENODE)
make TARGET_ENV=renode

# Xem kích thước binary
make size

# Tạo file disassembly
make list
```

### Nạp firmware

```bash
# Nạp qua OpenOCD (ST-Link)
make flash
```

### Giả lập với Renode

```bash
# Start Renode với script mẫu
renode --console --disable-gui -e "i @scripts/stm32_sample.resc"
```

Trong Renode console:
```
(machine-0) start
(machine-0) sysbus LoadELF @build/FIRMWARE.elf
```

### VSCode Integration

Mở project trong VSCode, cài extension **Cortex-Debug**, sau đó:

1. **Build**: `Ctrl+Shift+B` → chọn "Make: build"
2. **Debug Renode**: F5 → chọn "Renode Debug"
3. **Debug Hardware**: F5 → chọn "HW Debug"

### Các lệnh hữu ích

| Lệnh | Mô tả |
|------|-------|
| `make setup` | Hiển thị hướng dẫn cài đặt |
| `make install` | Tự động cài tools (Windows) |
| `make verify` | Kiểm tra tools đã cài |
| `make add-path` | Thêm tools vào PATH |
| `make clean` | Xóa file build |
| `make clean-all` | Xóa thư mục build/ |
| `make help` | Hiển thị tất cả lệnh |

---

## Troubleshooting

### Lỗi "arm-none-eabi-gcc: command not found"

- Kiểm tra đã thêm toolchain vào PATH chưa
- Chạy `make verify` để xem lỗi cụ thể
- Windows: Restart terminal sau khi cài

### Lỗi "make: command not found" (Windows)

- Cài Make từ [GnuWin32](http://gnuwin32.sourceforge.net/packages/make.htm)
- Hoặc chạy: `winget install GnuWin32.Make`
- Chạy `make add-path` rồi restart terminal

### Lỗi thiếu header file (core_cm3.h, stm32f10x.h)

- Copy thư mục `platform/` từ project gốc vào project này
- Kiểm tra đường dẫn include trong Makefile

### Renode không kết nối được GDB

- Build với `make TARGET_ENV=renode`
- Đảm bảo port 3333 không bị chiếm dụng
- Chạy `taskkill /F /IM Renode.exe` (Windows) để kill process cũ

### Flash failed (OpenOCD)

- Kiểm tra ST-Link đã kết nối
- Cài driver ST-Link từ st.com
- Thử giảm tốc độ: sửa `adapter speed 2000` → `adapter speed 1000`

---

## License

Sample project for educational purposes.

---

**Tác giả**: AUTOSAR Project Sample  
**Version**: 1.0  
**Last Updated**: 2026
