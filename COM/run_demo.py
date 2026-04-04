#!/usr/bin/env python3
import subprocess
import time
import os
import signal
import sys

# Hàm tiện ích: Khởi tạo tệp log trống để tránh đọc lại dữ liệu từ phiên trước
def touch_and_clear(path):
    with open(path, 'w') as f:
        f.truncate(0)

print("\033[96m[1/3] Đang Biên dịch mã nguồn C (ELFs)...\033[0m")
# Dọn dẹp môi trường: Chấm dứt các tiến trình Renode/Mono cũ để giải phóng tài nguyên Port (1234, 3333, 3334)
os.system("pkill -9 -f renode > /dev/null 2>&1")
os.system("killall -9 mono > /dev/null 2>&1")
# Gọi Makefile để biên dịch Firmware
os.system("make all -j4 > /dev/null 2>&1")

# Tạo thư mục con 'logs' nếu chưa tồn tại
os.makedirs("logs", exist_ok=True)

# Khởi tạo các tệp log giao tiếp
touch_and_clear("logs/tx_uart.log")
touch_and_clear("logs/rx_uart.log")
touch_and_clear("logs/renode_system.log")

print("\033[96m[2/3] Đang khởi động lõi mô phỏng Renode với kịch bản CANTP...\033[0m")
# Chuyển hướng System Log của Renode ra tệp riêng biệt để tránh nhiễu thông tin trên Terminal
renode_out = open("logs/renode_system.log", "w")
# KHỞI ĐỘNG RENODE: Thực thi dưới dạng background process (vô hiệu hóa GUI)
renode_process = subprocess.Popen(
    ["renode", "--disable-gui", "scripts/stm32_interactive_cantp.resc"], 
    stdout=renode_out, 
    stderr=renode_out,
    stdin=subprocess.PIPE
)

print("\033[92m[3/3] Hệ thống đã sẵn sàng hoạt động!\033[0m")
print("\033[93m>>> BẠN CÓ THỂ ĐÍNH KÈM DEBUGGER (F5 TẠI VS CODE) NẾU CẦN THIẾT <<<\033[0m")
print("\033[93m>>> Hoặc thực thi luồng 'python3 trigger_tx.py' ở Terminal khác để kiểm thử luồng Tx. <<<\033[0m")
print("\033[93m>>> Đang đính kèm luồng theo dõi UART Logs...\033[0m\n")

# THEO DÕI UART LOG QUA SYSTEM BASH
# Sử dụng cờ -F (Tail theo định dạng File Name thay vì Descriptor):
# Đảm bảo luồng theo dõi không bị ngắt quãng khi Renode thực hiện cơ chế Log Rotation (đổi tên tệp hiện tại và tạo tệp mới).
tail_proc = subprocess.Popen(["tail", "-F", "logs/tx_uart.log", "logs/rx_uart.log"])

try:
    # Vòng lặp giám sát (Watchdog cho tiến trình Renode)
    while True:
        # Kiểm tra trạng thái Exit Code của tiến trình con
        if renode_process.poll() is not None:
            print("\n\033[91m[LỖI] Tiến trình Renode đã dừng đột ngột. Chi tiết xem tại logs/renode_system.log\033[0m")
            break
        # Sleep để tối ưu hóa chu kỳ Polling
        time.sleep(1)
        
# XỬ LÝ SỰ KIỆN TERMINATION (Ctrl+C)
except KeyboardInterrupt:
    print("\nNhận tín hiệu dừng, đang giải phóng tài nguyên Renode...")
    try:
        renode_process.terminate()  # Gửi cờ SIGTERM
        # Đảm bảo dọn dẹp triệt để các Thread bảo lưu
        os.system("pkill -9 -f renode > /dev/null 2>&1")
        os.system("killall -9 mono > /dev/null 2>&1")
    except:
        pass
    sys.exit(0)
