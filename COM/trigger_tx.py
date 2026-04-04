#!/usr/bin/env python3
import subprocess
import sys

print("\033[93m[Trigger] Đang kết nối GDB vào mô phỏng Renode để thiết lập cờ 'trigger_tx = 1'...\033[0m")

# CẤU HÌNH GDB CHẠY TRONG MÔI TRƯỜNG DÒNG LỆNH TỰ ĐỘNG (BATCH MODE)
# Tập lệnh này khởi chạy GDB dưới dạng background process để nạp dữ liệu thẳng vào RAM vi điều khiển 
# mà không cần tương tác qua giao diện người dùng.
cmd = [
    "arm-none-eabi-gdb", "node_tx.elf",
    "--batch",  # Thực thi chuỗi lệnh và tự động thoát thay vì mở giao diện interactive REPL
    "-ex", "target remote localhost:3333",  # Kết nối với GDB Server của Node TX. Hành động này sẽ tạo sự kiện PAUSE (Halt) CPU.
    "-ex", "set trigger_tx=1",              # Ghi đè giá trị biến trigger_tx = 1 trong bộ nhớ RAM
    "-ex", "detach",                        # Ngắt kết nối GDB. Quá trình Detach sẽ kích hoạt CPU tiếp tục chu kỳ máy (Resume execution).
    "-ex", "quit"                           # Thoát phiên GDB
]

# Khởi chạy Process cha cho GDB, chuyển hướng output (stdout và stderr) vào PIPE để tránh làm trôi Terminal.
result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

if result.returncode == 0:
    # returncode = 0 biểu thị GDB đã thực thi toàn bộ chuỗi lệnh thành công không có lỗi
    print("\033[92m[Trigger] Lệnh truyền CAN đã được kích hoạt thành công! Vui lòng kiểm tra Terminal 'run_demo.py' để xem Output logs.\033[0m")
else:
    # Bắt lỗi nếu Renode chưa được mở hoặc Port 3333 không phản hồi
    print("\033[91m[Trigger] Kích hoạt thất bại. Xin đảm bảo tiến trình 'run_demo.py' đang hoạt động.\033[0m")
    print(result.stderr)
