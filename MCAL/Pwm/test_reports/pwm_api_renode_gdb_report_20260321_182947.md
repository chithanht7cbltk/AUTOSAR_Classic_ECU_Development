# PWM API Test Report (Renode + GDB)

- Generated at: `2026-03-21T18:29:47.467923`
- ELF: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/Pwm/tests/renode/build/pwm_api_test.elf`
- Repl: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/stm32f103_full.repl`
- Renode: `/usr/local/bin/renode`
- GDB: `/Users/phamvanvu/Projects/Embedded/ARM-Toolchain/arm-toolchain/bin/arm-none-eabi-gdb`

## Summary: 16/19 scenarios passed

| Scenario | APIs | Result |
|---|---|---|
| init_default_values | Pwm_Init | PASS |
| set_duty_ch0_half | Pwm_Init, Pwm_SetDutyCycle | PASS |
| set_duty_ch1_full | Pwm_Init, Pwm_SetDutyCycle | PASS |
| set_period_and_duty | Pwm_Init, Pwm_SetPeriodAndDuty | PASS |
| set_output_to_idle | Pwm_Init, Pwm_SetDutyCycle, Pwm_SetOutputToIdle | PASS |
| get_output_state_high_low | Pwm_Init, Pwm_GetOutputState | PASS |
| enable_notification | Pwm_Init, Pwm_EnableNotification | PASS |
| disable_notification | Pwm_Init, Pwm_EnableNotification, Pwm_DisableNotification | PASS |
| deinit_disables_counter | Pwm_Init, Pwm_DeInit | PASS |
| version_info | Pwm_GetVersionInfo | PASS |
| invalid_channel_set_duty_no_change | Pwm_Init, Pwm_SetDutyCycle | PASS |
| invalid_channel_get_output_state | Pwm_Init, Pwm_GetOutputState | PASS |
| power_set_without_prepare | Pwm_Init, Pwm_SetPowerState | PASS |
| power_prepare_set_ok | Pwm_Init, Pwm_PreparePowerState, Pwm_SetPowerState, Pwm_GetCurrentPowerState | PASS |
| power_get_target | Pwm_Init, Pwm_PreparePowerState, Pwm_GetTargetPowerState | PASS |
| power_prepare_uninit | Pwm_PreparePowerState | FAIL |
| power_get_current_uninit | Pwm_GetCurrentPowerState | FAIL |
| power_get_target_uninit | Pwm_GetTargetPowerState | FAIL |
| main_power_transition_uninit_safe | Pwm_Main_PowerTransitionManager | PASS |

## Failed Details

### power_prepare_uninit
- Description: PreparePowerState khi chưa init phải trả PWM_NOT_INIT.
- GDB exit code: 0
- Check `ret`: actual=0 expected=1 comparator=eq -> FAIL
- Check `result`: actual=0 expected=1 comparator=eq -> FAIL

### power_get_current_uninit
- Description: GetCurrentPowerState khi chưa init phải trả PWM_NOT_INIT.
- GDB exit code: 0
- Check `ret`: actual=0 expected=1 comparator=eq -> FAIL
- Check `result`: actual=0 expected=1 comparator=eq -> FAIL

### power_get_target_uninit
- Description: GetTargetPowerState khi chưa init phải trả PWM_NOT_INIT.
- GDB exit code: 0
- Check `ret`: actual=0 expected=1 comparator=eq -> FAIL
- Check `result`: actual=0 expected=1 comparator=eq -> FAIL

