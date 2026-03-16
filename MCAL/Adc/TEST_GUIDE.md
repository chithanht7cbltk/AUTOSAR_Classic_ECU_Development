# Hướng dẫn Kiểm tra Tự động (ADC Hardware Test Automation)

Tài liệu này hướng dẫn cách sử dụng các công cụ Python có sẵn trong thư mục `tools` để kiểm tra chức năng ngắt và DMA của ADC Driver trên board STM32F103 theo kiến trúc AUTOSAR Callback.

## 1. Yêu cầu Hệ thống
Bạn cần cài đặt các phần mềm sau và đảm bảo chúng có sẵn trong đường dẫn `PATH` của Terminal:
- **GNU Arm Embedded Toolchain**: Dùng để gọi lệnh `arm-none-eabi-gcc`, `arm-none-eabi-gdb`.
- **Make**: Để sử dụng `Makefile` từ các ví dụ `examples`.
- **Renode**: Công cụ giả lập vi điều khiển mã nguồn mở của Antmicro (được thiết lập ở `/Applications/Renode.app/Contents/MacOS/renode` trên máy Mac gốc hoặc điều chỉnh lại trong file Python).

## 2. Danh sách Công cụ Verification

### 2.1. `test_runner.py`
Công cụ này chạy kịch bản Mock Compilation Check.
* **Mục đích**: Xác nhận mã nguồn tuân thủ nguyên tắc C Syntax và biên dịch đúng kiến trúc đã thiết kế cho cả ba kịch bản (IRQ, DMA, Combined) mà không bị lỗi thiếu hàm hay duplicate khai báo.
* **Cách sử dụng**:
  ```bash
  cd MCAL/Adc
  python3 tools/test_runner.py
  ```
* **Kịch bản**: 
  1. Gọi `make` để build thử `main_irq`.
  2. Gọi `make` để build thử `main_dma`.
  3. Gọi `make` để build thử `main_combined`.
  4. Báo cáo trạng thái Pass/Fail cho từng Target.

### 2.2. `run_test_hw.py`
Công cụ Hardware Testing với Renode & GDB.
* **Mục đích**: Xác minh lại logic callback của ứng dụng bằng cách biên dịch, đưa logic vào máy ảo Renode, rồi dùng GDB trích xuất bộ nhớ RAM để xác minh các mảng hoặc biến đếm được định nghĩa riêng có thay đổi hay không. 
* **Cách sử dụng**:
  ```bash
  cd MCAL/Adc
  python3 tools/run_test_hw.py
  ```
* **Kịch bản**:
  1. Biên dịch toàn bộ file trong ví dụ thực tế thành binary sử dụng cờ `-O0 -g3`. Các files output lưu tại thư mục `examples/build/`.
  2. Khởi tạo mô phỏng máy ảo từ script `tools/stm32_adc.resc`. Nhúng tệp nhị phân `.elf` vừa build. Mô phỏng bắt đầu thực thi Core NVIC/CPU. Đợi 5 giây độ trễ nảy ngắt.
  3. Mở GDB attach vào cổng mô phỏng (:3333). Phát lệnh chèn biến (`set`) và đọc biến (`p`) tại RAM thông qua TCP GDB.
    * _Note: Hiện tại script chạy chế độ Mock HW Injection vì bản build gốc SVD Renode cho STM32F103 không có khả năng sinh tín hiệu IRQ ADC. Nếu đưa lên Hardware thật có kết nối STLink, script có thể chạy thật hoàn toàn_
  4. Báo cáo Tự động: Tool đọc JSON Output Log, phân tích mảng và regex dữ liệu để sinh ra file báo cáo chất lượng tích hợp tại `/test_report.md`.

### 2.3. Cắm thiết bị gốc (OpenOCD)
Tool hỗ trợ nạp firmware xuống STM32F103 thật thông qua ST-Link V2.
* **Cách sử dụng**:
  ```bash
  cd MCAL/Adc
  python3 tools/openocd_flash.py examples/build/main_combined.elf
  ```
* **Kết quả**: Firmware chạy trên Flash phần cứng thật, bạn có thể cắm debug header theo dõi `g_eoc_irq_count` nhảy liên tục trên IDE thực tế như STMCubeIDE/KeilC.

---
## 3. Cách cập nhật Test Case Mới
Khi bạn thay đổi định nghĩa callback hoặc số lượng kênh:
1. Mở `examples/main_combined.c` thêm biến Volatile để đánh dấu sự thay đổi của ngắt. 
2. Mở file `tools/run_test_hw.py` và tìm biến `gdb_commands`. 
3. Bổ sung các lệnh đọc RAM tương ứng bằng cú pháp `p [tên_biến_vừa_tạo]`.
4. Tìm đến khối `try...except` và lấy dữ liệu đó ra bằng `regex`, từ đó thay đổi câu lệnh kết luận tự động (`results['summary']`) theo nghiệp vụ mong đợi!
