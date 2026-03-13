# Hướng dẫn chi tiết sử dụng Tool Khởi tạo Project STM32 đa nền tảng

_(Hỗ trợ Windows, macOS, Ubuntu / Linux)_

Công cụ `setup_project.py` giúp bạn tự động sinh ra một cấu trúc thư mục dự án hoàn chỉnh cho vi điều khiển STM32F103 (tích hợp thư viện Standard Peripheral Library - SPL), bao gồm luôn cấu hình Makefile và các file cấu hình Visual Studio Code (`.vscode/`) để có thể nhấn nút Build và chạy giả lập Renode ngay lập tức!

---

## Phần 1: Các yêu cầu môi trường cần cài đặt

Trước khi có thể chạy công cụ này, bạn cần cài đặt **Python 3** cùng các công cụ chuẩn của việt phát triển hệ thống nhúng (Embedded) là **ARM GCC Toolchain**, **Make**, và **Renode** (cho phần giả lập).

### 1. Dành cho Windows:

- **Python**: Tải và cài đặt tại [python.org/downloads](https://www.python.org/downloads/).
- **ARM GCC Toolchain**: Tải file cài đặt `.exe` từ web của [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) và cài đặt thông thường. Hãy đánh dấu vào mục "Add to Path" nếu có (hoặc ghi nhớ đường dẫn cài đặt để nhập vào tool).
- **Make**: Cách dễ nhất là cài đặt MSYS2 hoặc Git-Bash for Windows, sau đó cài gói `make` thông qua MSYS2 (`pacman -S make`) hoặc sử dụng [Make for Windows](http://gnuwin32.sourceforge.net/packages/make.htm).
- **Renode**: Tải file setup `.msi` mới nhất từ [trang chủ Renode](https://renode.io/) và tiến hành cài đặt.
- _(Tuỳ chọn)_ **OpenOCD**: Nếu muốn nạp xuống mạch vật lý, bạn tải OpenOCD release cho Windows và extract, có thể thêm vào System PATH.

### 2. Dành cho macOS (máy Mac/Macbook):

Bật ứng dụng Terminal và dán các lệnh sau (nếu chưa cài thẻ Homebrew, hãy xem ở brew.sh):

- **Python**: `brew install python`
- **ARM GCC Toolchain**: Máy Mac cần GCC dành cho mạch ARM.
  ```bash
  brew install cmake ninja
  brew tap ArmMbed/homebrew-formulae
  brew install arm-none-eabi-gcc
  ```
- **Make**: Thường đã có sẵn nếu cài `xcode-select --install` hoặc `brew install make`.
- **Renode**: Tải bản Portabl `.dmg` hoặc `.app` từ trang Release của [Renode.io](https://renode.io/). Mặc định ở máy Mac, kéo ứng dụng `Renode.app` vào thư mục `Applications` (`/Applications/Renode.app/Contents/MacOS/renode`).

### 3. Dành cho Ubuntu / Linux Debian:

- **Python & Make**: (Thường đã có sẵn, cập nhật cho chắc)
  ```bash
  sudo apt-update
  sudo apt install python3 make build-essential
  ```
- **ARM GCC Toolchain**:
  ```bash
  sudo apt install gcc-arm-none-eabi gdb-arm-none-eabi binutils-arm-none-eabi
  ```
- **Renode**:
  Tải file `.deb` trên trang chủ [Renode releases](https://github.com/renode/renode/releases) và cài bằng lệnh: `sudo dpkg -i renode_*.deb` (nếu có lỗi dependency thì gõ thêm `sudo apt --fix-broken install`).
- _(Tuỳ chọn)_ **OpenOCD**: `sudo apt install openocd`

---

## Phần 2: Hướng dẫn khởi chạy công cụ (`setup_project.py`)

1. Mở Terminal / Command Prompt tại nơi bạn lưu trữ file `setup_project.py`.
2. Gõ lệnh chạy script:

   ```bash
   python setup_project.py
   # hoặc trên Mac/Ubuntu:
   python3 setup_project.py
   # hoặc:
   ./setup_project.py
   ```

3. Công cụ sẽ hiển thị lên giao diện hỏi-đáp trên Terminal, hãy nhập câu trả lời, hoặc **bấm Enter trực tiếp để chọn thông số mặc định trong ngoặc vuông `[ ]`**.

### Chi tiết các phần câu hỏi (Input):

- **Thư mục tạo project**: Đường dẫn thư mục nơi dự án được chứa.
  - _Ví dụ_: `./My_First_Blink` hoặc thư mục đường dẫn tuyệt đối kiểu `C:\Projects\STM32_Blink`.
- **Tên ELF/BIN build ra**: Tên phần mềm bạn sẽ biên dịch tạo thành.
  - _Mặc định_: `FIRMWARE` (tạo ra FIRMWARE.elf và FIRMWARE.bin).
- **CAU HINH ARM TOOLCHAIN**:
  - Khuyến nghị: **Nhấn Enter** nếu bạn đã setup biến PATH chuẩn ở bước 1. Nó sẽ tự động dùng "arm-none-eabi-gcc". Phải đảm bảo lệnh arm-none-eabi-gcc hoạt động trên cmd.
  - _Sử dụng đường dẫn tuyệt đối_: Ví dụ đối với Mac tải bộ cài chay: `/Users/name/Tools/arm-toolchain/bin/arm-none-eabi-gcc`.
- **CAU HINH RENODE**:
  - Nhập đường dẫn chạy tool Renode (Enter để cài đặt mặc định). Đối với windows có thế là `renode` hoặc đường dẫn tuyệt đối tới `Renode.exe`.

### Sự kỳ diệu (Phép màu tự động hoá):

Sau khi có thông tin, Tool sẽ lo lắng tất cả:

- Tạo thư mục con: `src`, `scripts`, `.vscode`.
- Gen code mẫu Blink LED vào `src/main.c`.
- Sinh **Makefile** có sẵn lệnh biên dịch, kết nối thư viện.
- Sinh `.vscode/tasks.json` cung cấp các tác vụ dọn dẹp port, build code, chạy hệ thống giả lập với Renode.
- Sinh `.vscode/launch.json` hỗ trợ Debugger (chỉ với 1 phát F5).
- Sinh `.vscode/c_cpp_properties.json` để IntelliSense bắt đúng các file của arm-gcc nhằm xóa các lỗi gạch chân xanh/đỏ cho C/C++.
- Sinh ra kịch bản máy ảo `/scripts/stm32_sample.resc` có log, nạp phím ấn và LED PA13 ảo.

---

## Phần 3: Chạy Project Trên VSCode

> [!IMPORTANT]
> **COPY THƯ VIỆN SPL VÀO PROJECT!**
> Công cụ chỉ tạo Project. Do thư viện chuẩn Peripheral Library của STM32 và CMSIS khá nặng, không được đóng gói kèm Script.
>
> - Cần copy toàn bộ thư mục `platform` (từ dự án gốc OSEK-RTOS hoặc gói STM32F10x_StdPeriph_Lib) dán đè trực tiếp vào bên trong thư mục Project mới (`./My_First_Blink/platform/`).

1. Bật Visual Studio Code, nhấn **File -> Open Folder** và trỏ chuột mở Project vừa tạo.
2. Cài đặt các Extensions bổ trợ (nếu chưa có): _"C/C++" (của Microsoft)_ và _"Cortex-Debug"_.
3. Nhấn tổ hợp phím **Ctrl + Shift + B** (Windows/Linux) hoặc **Cmd + Shift + B** (Mac). Tuỳ chọn Build Menu hiện ra:
   - Nhấn **Make: build**, VSCode sẽ gọi Make và biên dịch code thành `.bin` và `.elf` trong thư mục `/build/`.
   - Nếu lỗi thiếu "core_cm3.h" hay SPL báo lỗi, bạn đã quên bước quan trọng COPY thư mục Platform ở phía trên.
4. **Mô phỏng Renode**: Nhấn sang bảng điều khiển **Run and Debug** ở thanh bên cạnh (cửa sổ VSCode cạnh trái) -> Ở chỗ combobox chọn `Renode Debug` -> Bấm icon Hình tam giác xanh lá cây (hoặc ấn `F5`).
   - Tool tự build file bằng Makefile, gọi máy ảo Renode, trỏ lệnh mở gdb localhost, nối máy chủ với VSCode.
   - Code sẽ bắt đầu nhảy vào điểm Break tại hàm `main` - Trải nghiệm Debug hệt như với mạch thật!
