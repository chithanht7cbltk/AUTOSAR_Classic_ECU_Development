# PWM API Test Report (Renode + GDB)

- Generated at: `2026-03-21T18:28:04.976706`
- ELF: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/Pwm/tests/renode/build/pwm_api_test.elf`
- Repl: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/stm32f103_full.repl`
- Renode: `/usr/local/bin/renode`
- GDB: `/Users/phamvanvu/Projects/Embedded/ARM-Toolchain/arm-toolchain/bin/arm-none-eabi-gdb`

## Summary: 1/19 scenarios passed

| Scenario | APIs | Result |
|---|---|---|
| init_default_values | Pwm_Init | FAIL |
| set_duty_ch0_half | Pwm_Init, Pwm_SetDutyCycle | FAIL |
| set_duty_ch1_full | Pwm_Init, Pwm_SetDutyCycle | FAIL |
| set_period_and_duty | Pwm_Init, Pwm_SetPeriodAndDuty | FAIL |
| set_output_to_idle | Pwm_Init, Pwm_SetDutyCycle, Pwm_SetOutputToIdle | FAIL |
| get_output_state_high_low | Pwm_Init, Pwm_GetOutputState | FAIL |
| enable_notification | Pwm_Init, Pwm_EnableNotification | FAIL |
| disable_notification | Pwm_Init, Pwm_EnableNotification, Pwm_DisableNotification | FAIL |
| deinit_disables_counter | Pwm_Init, Pwm_DeInit | FAIL |
| version_info | Pwm_GetVersionInfo | FAIL |
| invalid_channel_set_duty_no_change | Pwm_Init, Pwm_SetDutyCycle | FAIL |
| invalid_channel_get_output_state | Pwm_Init, Pwm_GetOutputState | FAIL |
| power_set_without_prepare | Pwm_Init, Pwm_SetPowerState | FAIL |
| power_prepare_set_ok | Pwm_Init, Pwm_PreparePowerState, Pwm_SetPowerState, Pwm_GetCurrentPowerState | FAIL |
| power_get_target | Pwm_Init, Pwm_PreparePowerState, Pwm_GetTargetPowerState | FAIL |
| power_prepare_uninit | Pwm_PreparePowerState | FAIL |
| power_get_current_uninit | Pwm_GetCurrentPowerState | FAIL |
| power_get_target_uninit | Pwm_GetTargetPowerState | FAIL |
| main_power_transition_uninit_safe | Pwm_Main_PowerTransitionManager | PASS |

## Failed Details

### init_default_values
- Description: Pwm_Init phải set ARR/CCR theo cấu hình mặc định cho 2 kênh.
- GDB exit code: 0
- Check `tim2_arr`: actual=None expected=999 comparator=eq -> FAIL
- Check `tim2_ccr1`: actual=None expected=0 comparator=eq -> FAIL
- Check `tim3_arr`: actual=None expected=999 comparator=eq -> FAIL
- Check `tim3_ccr2`: actual=None expected=0 comparator=eq -> FAIL

### set_duty_ch0_half
- Description: Pwm_SetDutyCycle channel 0 với 50%.
- GDB exit code: 0
- Check `tim2_ccr1`: actual=None expected=499 comparator=eq -> FAIL

### set_duty_ch1_full
- Description: Pwm_SetDutyCycle channel 1 với 100%.
- GDB exit code: 0
- Check `tim3_ccr2`: actual=None expected=999 comparator=eq -> FAIL

### set_period_and_duty
- Description: Pwm_SetPeriodAndDuty phải cập nhật ARR/CCR kênh variable period.
- GDB exit code: 0
- Check `tim2_arr`: actual=None expected=2000 comparator=eq -> FAIL
- Check `tim2_ccr1`: actual=None expected=1000 comparator=eq -> FAIL

### set_output_to_idle
- Description: Pwm_SetOutputToIdle phải đưa duty về 0.
- GDB exit code: 0
- Check `tim2_ccr1`: actual=None expected=0 comparator=eq -> FAIL

### get_output_state_high_low
- Description: Pwm_GetOutputState trả HIGH/LOW theo CCER bit enable.
- GDB exit code: 0
- Check `state_high`: actual=None expected=0 comparator=eq -> FAIL
- Check `state_low`: actual=None expected=1 comparator=eq -> FAIL

### enable_notification
- Description: Pwm_EnableNotification bật bit DIER CC1IE.
- GDB exit code: 0
- Check `dier_cc1`: actual=None expected=1 comparator=eq -> FAIL

### disable_notification
- Description: Pwm_DisableNotification xóa bit DIER CC1IE.
- GDB exit code: 0
- Check `dier_cc1`: actual=None expected=0 comparator=eq -> FAIL

### deinit_disables_counter
- Description: Pwm_DeInit phải tắt counter timer.
- GDB exit code: 0
- Check `tim2_cen`: actual=None expected=0 comparator=eq -> FAIL
- Check `tim3_cen`: actual=None expected=0 comparator=eq -> FAIL

### version_info
- Description: Pwm_GetVersionInfo trả đúng phiên bản cấu hình cứng trong driver.
- GDB exit code: 0
- Check `vendor`: actual=None expected=4660 comparator=eq -> FAIL
- Check `module`: actual=None expected=43981 comparator=eq -> FAIL
- Check `sw_major`: actual=None expected=1 comparator=eq -> FAIL

### invalid_channel_set_duty_no_change
- Description: SetDuty với channel invalid không làm thay đổi CCR hiện tại.
- GDB exit code: 0
- Check `before`: actual=None expected=249 comparator=eq -> FAIL
- Check `after`: actual=None expected=249 comparator=eq -> FAIL

### invalid_channel_get_output_state
- Description: GetOutputState với channel invalid trả PWM_LOW.
- GDB exit code: 0
- Check `rv`: actual=None expected=1 comparator=eq -> FAIL

### power_set_without_prepare
- Description: SetPowerState trước PreparePowerState phải lỗi sequence.
- GDB exit code: 0
- Check `ret`: actual=None expected=1 comparator=eq -> FAIL
- Check `result`: actual=None expected=2 comparator=eq -> FAIL

### power_prepare_set_ok
- Description: Prepare + Set power state thành công.
- GDB exit code: 0
- Check `r0`: actual=536870912 expected=0 comparator=eq -> FAIL
- Check `r1`: actual=1073876992 expected=0 comparator=eq -> FAIL
- Check `r2`: actual=536870912 expected=0 comparator=eq -> FAIL
- Check `state`: actual=None expected=1 comparator=eq -> FAIL
- Check `result`: actual=None expected=0 comparator=eq -> FAIL

### power_get_target
- Description: GetTargetPowerState trả đúng mode đã prepare.
- GDB exit code: 0
- Check `r0`: actual=536870912 expected=0 comparator=eq -> FAIL
- Check `r1`: actual=1073876992 expected=0 comparator=eq -> FAIL
- Check `state`: actual=None expected=2 comparator=eq -> FAIL

### power_prepare_uninit
- Description: PreparePowerState khi chưa init phải trả PWM_NOT_INIT.
- GDB exit code: 0
- Check `ret`: actual=None expected=1 comparator=eq -> FAIL
- Check `result`: actual=None expected=1 comparator=eq -> FAIL

### power_get_current_uninit
- Description: GetCurrentPowerState khi chưa init phải trả PWM_NOT_INIT.
- GDB exit code: 0
- Check `ret`: actual=None expected=1 comparator=eq -> FAIL
- Check `result`: actual=None expected=1 comparator=eq -> FAIL

### power_get_target_uninit
- Description: GetTargetPowerState khi chưa init phải trả PWM_NOT_INIT.
- GDB exit code: 0
- Check `ret`: actual=None expected=1 comparator=eq -> FAIL
- Check `result`: actual=None expected=1 comparator=eq -> FAIL

