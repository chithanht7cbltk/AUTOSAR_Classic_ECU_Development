# ADC API Test Report (Renode + GDB)

- Generated at: `2026-03-21T19:27:36.766880`
- ELF: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/Adc/tests/renode/build/adc_api_test.elf`
- Repl: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/stm32f103_full.repl`
- Renode: `/usr/local/bin/renode`
- GDB: `/Users/phamvanvu/Projects/Embedded/ARM-Toolchain/arm-toolchain/bin/arm-none-eabi-gdb`
- Traceability source: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/Adc/tools/adc_sws_r25_11_extracted.json`

## Summary: 19/19 scenarios passed
- Requirement summary: 158/158 PASS

| Scenario | APIs | Req IDs | Result |
|---|---|---|---|
| init_enables_adc_unit | Adc_Init | 11 | PASS |
| setup_result_buffer_group0 | Adc_Init, Adc_SetupResultBuffer | 19 | PASS |
| start_group_conversion_sw | Adc_Init, Adc_SetupResultBuffer, Adc_StartGroupConversion, Adc_GetGroupStatus | 46 | PASS |
| read_group_after_start | Adc_Init, Adc_SetupResultBuffer, Adc_StartGroupConversion, Adc_ReadGroup | 46 | PASS |
| stop_group_conversion | Adc_Init, Adc_SetupResultBuffer, Adc_StartGroupConversion, Adc_StopGroupConversion, Adc_GetGroupStatus | 59 | PASS |
| enable_hardware_trigger_group1 | Adc_Init, Adc_SetupResultBuffer, Adc_EnableHardwareTrigger | 34 | PASS |
| disable_hardware_trigger_group1 | Adc_Init, Adc_SetupResultBuffer, Adc_EnableHardwareTrigger, Adc_DisableHardwareTrigger | 47 | PASS |
| enable_group_notification | Adc_Init, Adc_EnableGroupNotification | 17 | PASS |
| disable_group_notification | Adc_Init, Adc_EnableGroupNotification, Adc_DisableGroupNotification | 23 | PASS |
| stream_last_pointer_group1 | Adc_Init, Adc_SetupResultBuffer, Adc_EnableHardwareTrigger, Adc_StartGroupConversion, Adc_ReadGroup, Adc_GetStreamLastPointer | 73 | PASS |
| version_info | Adc_GetVersionInfo | 2 | PASS |
| deinit_disables_adon | Adc_Init, Adc_DeInit | 18 | PASS |
| adc_isr_handler_callback | Adc_Init, Adc_SetupResultBuffer, Adc_EnableGroupNotification, Adc_StartGroupConversion | 41 | PASS |
| adc_dma_isr_handler_callbacks | Adc_Init, Adc_SetupResultBuffer, Adc_EnableHardwareTrigger, Adc_StartGroupConversion | 50 | PASS |
| power_set_without_prepare | Adc_Init, Adc_SetPowerState | 22 | PASS |
| power_prepare_set_get_current | Adc_Init, Adc_PreparePowerState, Adc_SetPowerState, Adc_GetCurrentPowerState | 30 | PASS |
| power_get_target | Adc_Init, Adc_PreparePowerState, Adc_GetTargetPowerState | 20 | PASS |
| main_power_transition_manager_safe | Adc_Main_PowerTransitionManager | 5 | PASS |
| invalid_group_negative | Adc_Init, Adc_SetupResultBuffer, Adc_StartGroupConversion, Adc_GetGroupStatus | 46 | PASS |
