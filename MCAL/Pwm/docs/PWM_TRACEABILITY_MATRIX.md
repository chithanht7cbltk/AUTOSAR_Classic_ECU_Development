# PWM SWS Traceability Matrix

- Source: `/Users/phamvanvu/HALAAcademy/01_Courses/AUTOSAR_Classic/MCAL/AUTOSAR_CP_SWS_PWMDriver.pdf`
- AUTOSAR release: `R25-11`
- Generated on: `2026-03-21`

## API Coverage Summary

- Total APIs in extracted SWS scope: **14**
- Fully implemented (declared+defined): **9**
- Missing in current module: **5**

## API Matrix

| API | SID | Section | Config Switch | Key SWS IDs | Header | Source | Status | Notes |
|---|---|---|---|---|---|---|---|---|
| Pwm_Init | 0x00 | 8.3.1 | mandatory | SWS_Pwm_00007, SWS_Pwm_00052, SWS_Pwm_00062, SWS_Pwm_00093, SWS_Pwm_00095, SWS_Pwm_00116, SWS_Pwm_00118, SWS_Pwm_00121, ... | YES | YES | implemented |  |
| Pwm_DeInit | 0x01 | 8.3.2 | PwmDeInitApi | SWS_Pwm_00010, SWS_Pwm_00011, SWS_Pwm_00012, SWS_Pwm_00096, SWS_Pwm_00117, SWS_Pwm_10051, SWS_Pwm_10080, SWS_Pwm_20051, ... | YES | YES | implemented |  |
| Pwm_SetDutyCycle | 0x02 | 8.3.3 | PwmSetDutyCycle | SWS_Pwm_00013, SWS_Pwm_00014, SWS_Pwm_00016, SWS_Pwm_00017, SWS_Pwm_00018, SWS_Pwm_00047, SWS_Pwm_00058, SWS_Pwm_00059, ... | YES | YES | implemented |  |
| Pwm_SetPeriodAndDuty | 0x03 | 8.3.4 | PwmSetPeriodAndDuty | SWS_Pwm_00019, SWS_Pwm_00020, SWS_Pwm_00041, SWS_Pwm_00045, SWS_Pwm_00047, SWS_Pwm_00058, SWS_Pwm_00059, SWS_Pwm_00076, ... | YES | YES | implemented |  |
| Pwm_SetOutputToIdle | 0x04 | 8.3.5 | PwmSetOutputToIdle | SWS_Pwm_00021, SWS_Pwm_00047, SWS_Pwm_00117, SWS_Pwm_00119, SWS_Pwm_10051, SWS_Pwm_10084, SWS_Pwm_10086, SWS_Pwm_20051, ... | YES | YES | implemented |  |
| Pwm_GetOutputState | 0x05 | 8.3.6 | PwmGetOutputState | SWS_Pwm_00022, SWS_Pwm_00047, SWS_Pwm_00100, SWS_Pwm_00117, SWS_Pwm_10051, SWS_Pwm_10085, SWS_Pwm_20051, SWS_Pwm_20085, ... | YES | YES | implemented |  |
| Pwm_DisableNotification | 0x06 | 8.3.7 | PwmNotificationSupported | SWS_Pwm_00023, SWS_Pwm_00047, SWS_Pwm_00117, SWS_Pwm_10051, SWS_Pwm_10112, SWS_Pwm_20051, SWS_Pwm_20112, SWS_Pwm_91003 | YES | YES | implemented |  |
| Pwm_EnableNotification | 0x07 | 8.3.8 | PwmNotificationSupported | SWS_Pwm_00024, SWS_Pwm_00047, SWS_Pwm_00081, SWS_Pwm_00117, SWS_Pwm_10051, SWS_Pwm_10113, SWS_Pwm_20051, SWS_Pwm_20113, ... | YES | YES | implemented |  |
| Pwm_SetPowerState | 0x09 | 8.3.9 | PwmLowPowerStatesSupport | SWS_Pwm_00166, SWS_Pwm_00167, SWS_Pwm_00168, SWS_Pwm_00169, SWS_Pwm_00170, SWS_Pwm_00171, SWS_Pwm_00172, SWS_Pwm_00173, ... | NO | NO | missing | Not present in current module |
| Pwm_GetCurrentPowerState | 0x0a | 8.3.10 | PwmLowPowerStatesSupport | SWS_Pwm_00177, SWS_Pwm_00178, SWS_Pwm_00179 | NO | NO | missing | Not present in current module |
| Pwm_GetTargetPowerState | 0x0b | 8.3.11 | PwmLowPowerStatesSupport | SWS_Pwm_00180, SWS_Pwm_00181, SWS_Pwm_00182 | NO | NO | missing | Not present in current module |
| Pwm_PreparePowerState | 0x0c | 8.3.12 | PwmLowPowerStatesSupport | SWS_Pwm_00183, SWS_Pwm_00184, SWS_Pwm_00185, SWS_Pwm_00186, SWS_Pwm_00187, SWS_Pwm_00188 | NO | NO | missing | Not present in current module |
| Pwm_GetVersionInfo | 0x08 | 8.3.13 | PwmVersionInfoApi | SWS_Pwm_00103 | YES | YES | implemented |  |
| Pwm_Main_PowerTransitionManager | 0x0d | 8.5.1 | PwmLowPowerStatesSupport + PwmPowerStateAsynchTransitionMode | SWS_Pwm_00189, SWS_Pwm_00190, SWS_Pwm_00191, SWS_Pwm_00192, SWS_Pwm_00193 | NO | NO | missing | Not present in current module |

## Error Symbol Coverage

| Category | Error Symbol | Value | Symbol in Current Code |
|---|---|---|---|
| development | `PWM_E_INIT_FAILED` | `0x10` | NO |
| development | `PWM_E_UNINIT` | `0x11` | NO |
| development | `PWM_E_PARAM_CHANNEL` | `0x12` | NO |
| development | `PWM_E_PERIOD_UNCHANGEABLE` | `0x13` | NO |
| development | `PWM_E_ALREADY_INITIALIZED` | `0x14` | NO |
| development | `PWM_E_PARAM_POINTER` | `0x15` | NO |
| development | `PWM_E_POWER_STATE_NOT_SUPPORTED` | `0x17` | NO |
| development | `PWM_E_TRANSITION_NOT_POSSIBLE` | `0x18` | NO |
| development | `PWM_E_PERIPHERAL_NOT_PREPARED` | `0x19` | NO |
| runtime | `PWM_E_NOT_DISENGAGED` | `0x16` | NO |

## Notes

- This matrix is generated from extracted SWS metadata + static scan of current `Pwm.h` and `Pwm.c` files.
- For formal compliance, keep this matrix synchronized with implementation and test evidence.
