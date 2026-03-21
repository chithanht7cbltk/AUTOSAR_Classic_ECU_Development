# PWM API Test Report (Renode + GDB)

- Generated at: `2026-03-21T18:36:42.586687`
- ELF: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/Pwm/tests/renode/build/pwm_api_test.elf`
- Repl: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/stm32f103_full.repl`
- Renode: `/usr/local/bin/renode`
- GDB: `/Users/phamvanvu/Projects/Embedded/ARM-Toolchain/arm-toolchain/bin/arm-none-eabi-gdb`
- Traceability source: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/Pwm/tools/pwm_sws_r25_11_extracted.json`

## Summary: 19/19 scenarios passed
- Requirement summary: 90/90 PASS

| Scenario | APIs | Req IDs | Result |
|---|---|---|---|
| init_default_values | Pwm_Init | 13 | PASS |
| set_duty_ch0_half | Pwm_Init, Pwm_SetDutyCycle | 25 | PASS |
| set_duty_ch1_full | Pwm_Init, Pwm_SetDutyCycle | 25 | PASS |
| set_period_and_duty | Pwm_Init, Pwm_SetPeriodAndDuty | 26 | PASS |
| set_output_to_idle | Pwm_Init, Pwm_SetDutyCycle, Pwm_SetOutputToIdle | 32 | PASS |
| get_output_state_high_low | Pwm_Init, Pwm_GetOutputState | 20 | PASS |
| enable_notification | Pwm_Init, Pwm_EnableNotification | 20 | PASS |
| disable_notification | Pwm_Init, Pwm_EnableNotification, Pwm_DisableNotification | 24 | PASS |
| deinit_disables_counter | Pwm_Init, Pwm_DeInit | 20 | PASS |
| version_info | Pwm_GetVersionInfo | 1 | PASS |
| invalid_channel_set_duty_no_change | Pwm_Init, Pwm_SetDutyCycle | 25 | PASS |
| invalid_channel_get_output_state | Pwm_Init, Pwm_GetOutputState | 20 | PASS |
| power_set_without_prepare | Pwm_Init, Pwm_SetPowerState | 24 | PASS |
| power_prepare_set_ok | Pwm_Init, Pwm_PreparePowerState, Pwm_SetPowerState, Pwm_GetCurrentPowerState | 33 | PASS |
| power_get_target | Pwm_Init, Pwm_PreparePowerState, Pwm_GetTargetPowerState | 22 | PASS |
| power_prepare_uninit | Pwm_DeInit, Pwm_PreparePowerState | 15 | PASS |
| power_get_current_uninit | Pwm_DeInit, Pwm_GetCurrentPowerState | 12 | PASS |
| power_get_target_uninit | Pwm_DeInit, Pwm_GetTargetPowerState | 12 | PASS |
| main_power_transition_uninit_safe | Pwm_Main_PowerTransitionManager | 5 | PASS |
