# Hướng dẫn kiểm thử tự động ADC (Renode + GDB)

Tài liệu này mô tả luồng kiểm thử đầy đủ cho module `Adc` sau khi đã viết lại theo SWS ADC.

## 1. Yêu cầu môi trường
- `arm-none-eabi-gcc`
- `arm-none-eabi-gdb`
- `renode`
- Python 3

Kiểm tra nhanh:

```bash
which arm-none-eabi-gcc
which arm-none-eabi-gdb
which renode
```

## 2. Build ví dụ ADC

```bash
cd Adc/examples
make clean all TARGET=main_irq
make clean all TARGET=main_dma
make clean all TARGET=main_combined
```

## 3. Chạy bộ test API đầy đủ

Tool chính:
- `Adc/tools/run_adc_api_renode_gdb_test.py`

Chạy mặc định:

```bash
cd /Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL
python3 Adc/tools/run_adc_api_renode_gdb_test.py
```

Tool sẽ tự động:
1. Build test harness ADC.
2. Khởi chạy Renode với `stm32f103_full.repl`.
3. Dùng GDB chạy toàn bộ scenario API.
4. Xuất report + traceability.

## 4. Bộ artifact sau khi chạy test

Report được lưu tại `Adc/test_reports/`:
- `adc_api_renode_gdb_report_*.md`
- `adc_api_renode_gdb_report_*.json`
- `adc_api_renode_gdb_details_*.json`
- `adc_api_renode_gdb_summary_*.json`
- `adc_api_traceability_*.json`
- `adc_api_traceability_*.csv`
- `adc_api_traceability_*.md`

## 5. Đồng bộ tài liệu kế hoạch/traceability

Sau khi sửa mã nguồn ADC, chạy lại:

```bash
python3 Adc/tools/generate_adc_planning_artifacts.py
```

Các file được cập nhật:
- `Adc/docs/ADC_TRACEABILITY_MATRIX.md`
- `Adc/docs/ADC_TRACEABILITY_MATRIX.csv`
- `Adc/docs/ADC_TEST_CHECKLIST.md`
- `Adc/docs/ADC_IMPLEMENTATION_PLAN_STM32F103_SPL.md`
