# PWM Exmaple Cases

Thư mục `exmaple` chứa các ví dụ nhỏ cho PWM AUTOSAR trên STM32F103 (SPL), tách thành nhiều nhóm case để dễ học và dễ build kiểm tra nhanh.

## Cấu trúc thư mục

- `basic/`
- `advanced/`
- `negative/`
- `integration/`
- `common/` (chứa hàm `Pwm_Example_PortInit()` cho init chân PWM)
- `build_examples.py`

## Danh sách case

- `basic/case_01_init_deinit`: Khởi tạo và de-init cơ bản.
- `basic/case_02_duty_sweep`: Quét duty cycle 2 kênh.
- `basic/case_03_period_and_duty`: Đổi period + duty cho kênh variable period.
- `advanced/case_04_notification`: Bật/tắt notification và thao tác output state.
- `advanced/case_05_power_state`: Luồng prepare/set/get power state.
- `negative/case_06_invalid_channel`: Gọi API với channel không hợp lệ.
- `negative/case_07_power_uninit`: Gọi power API khi chưa init.
- `integration/case_08_full_api_smoke`: Smoke test tích hợp nhiều API.

## Build tất cả case

```bash
python3 Pwm/exmaple/build_examples.py
```

Output ELF sẽ được đặt tại:

```text
Pwm/exmaple/build/<group>/<case>/<case>.elf
```

Report JSON tổng hợp:

```text
Pwm/exmaple/build/build_report.json
```
