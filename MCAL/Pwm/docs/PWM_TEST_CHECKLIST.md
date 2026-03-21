# PWM Test Checklist (AUTOSAR SWS Based)

- Generated on: `2026-03-21`
- Status values: `TODO`, `IN_PROGRESS`, `PASS`, `FAIL`, `N/A`

| TC ID | Title | SWS IDs | Level | Precondition | Steps | Expected | Status | Evidence |
|---|---|---|---|---|---|---|---|---|
| PWM_TC_001 | Pwm_Init initializes configured channels only | SWS_Pwm_00007, SWS_Pwm_00062 | unit+HIL | Driver uninitialized | Call Pwm_Init with valid config containing subset of channels | Only configured TIM/CH resources are touched | TODO |  |
| PWM_TC_002 | Pwm_Init starts channels with default period/duty/polarity | SWS_Pwm_10009, SWS_Pwm_20009, SWS_Pwm_30009 | HIL | Clock+Port configured | Call Pwm_Init then observe waveform | Default waveform matches configuration | TODO |  |
| PWM_TC_003 | Pwm_Init disables all notifications | SWS_Pwm_00052 | unit+HIL | Notification callbacks configured | Call Pwm_Init then trigger compare events | No callback is fired before EnableNotification | TODO |  |
| PWM_TC_004 | Pwm_Init twice reports already initialized | SWS_Pwm_00118, SWS_Pwm_50002 | unit | DET enabled, driver initialized | Call Pwm_Init again | DET gets PWM_E_ALREADY_INITIALIZED and state remains unchanged | TODO |  |
| PWM_TC_005 | Pwm_DeInit sets output to idle and disables notifications | SWS_Pwm_00010, SWS_Pwm_00011, SWS_Pwm_00012 | HIL | Driver initialized and running PWM | Call Pwm_DeInit | Outputs go idle, interrupts/callbacks disabled | TODO |  |
| PWM_TC_006 | API call before init raises UNINIT | SWS_Pwm_00117, SWS_Pwm_20002 | unit | DET enabled, driver uninitialized | Call each API except Pwm_Init | DET logs PWM_E_UNINIT and action skipped | TODO |  |
| PWM_TC_007 | SetDutyCycle with 0% follows polarity inversion rule | SWS_Pwm_00014 | HIL | Driver initialized | Call Pwm_SetDutyCycle(ch,0x0000) | Output steady state is inverse of configured polarity | TODO |  |
| PWM_TC_008 | SetDutyCycle with 100% follows polarity rule | SWS_Pwm_00014 | HIL | Driver initialized | Call Pwm_SetDutyCycle(ch,0x8000) | Output steady state equals configured polarity | TODO |  |
| PWM_TC_009 | SetDutyCycle in (0,100%) modulates correctly | SWS_Pwm_00016 | HIL | Driver initialized | Set duty to mid value and sample waveform | Measured duty equals configured ratio | TODO |  |
| PWM_TC_010 | Duty scaling formula uses 0x8000 as full scale | SWS_Pwm_00058, SWS_Pwm_00059 | unit | Driver initialized | Set multiple duty values and inspect CCR | CCR = (Period * Duty16)>>15 | TODO |  |
| PWM_TC_011 | Duty update timing follows PwmDutycycleUpdatedEndperiod | SWS_Pwm_00017 | HIL | Two builds: endperiod ON/OFF | Update duty during active period | Update occurs at end-of-period when ON, immediate when OFF | TODO |  |
| PWM_TC_012 | SetPeriodAndDuty changes both period and duty | SWS_Pwm_00019 | HIL | Variable-period channel | Call Pwm_SetPeriodAndDuty | ARR/CCR updated consistently | TODO |  |
| PWM_TC_013 | SetPeriodAndDuty only allowed on VARIABLE_PERIOD | SWS_Pwm_00041, SWS_Pwm_00045, SWS_Pwm_40002 | unit | DET enabled, fixed-period channel | Call Pwm_SetPeriodAndDuty | PWM_E_PERIOD_UNCHANGEABLE is reported and no register update | TODO |  |
| PWM_TC_014 | Period update timing follows PwmPeriodUpdatedEndperiod | SWS_Pwm_00076 | HIL | Two builds: endperiod ON/OFF | Update period while running | Update timing matches configuration | TODO |  |
| PWM_TC_015 | SetPeriodAndDuty with Period=0 results in 0% output | SWS_Pwm_00150 | HIL | Variable-period channel | Call Pwm_SetPeriodAndDuty(ch,0,duty) | Output remains low/0% | TODO |  |
| PWM_TC_016 | SetOutputToIdle acts immediately | SWS_Pwm_00021 | HIL | Channel running PWM | Call Pwm_SetOutputToIdle | Output transitions immediately to configured idle state | TODO |  |
| PWM_TC_017 | Reactivation path after SetOutputToIdle for variable period | SWS_Pwm_10086 | HIL | Variable-period channel idle | Reactivate using Pwm_SetPeriodAndDuty | Waveform resumes with new period | TODO |  |
| PWM_TC_018 | Reactivation path after SetOutputToIdle for fixed period | SWS_Pwm_00119, SWS_Pwm_20086 | HIL | Fixed-period channel idle | Reactivate using Pwm_SetDutyCycle | Waveform resumes with previous period | TODO |  |
| PWM_TC_019 | GetOutputState returns current internal output state | SWS_Pwm_00022 | unit+HIL | Driver initialized | Call Pwm_GetOutputState in different states | Return reflects internal high/low state | TODO |  |
| PWM_TC_020 | GetOutputState before init or invalid channel returns PWM_LOW | SWS_Pwm_30051 | unit | Driver uninitialized or bad channel | Call Pwm_GetOutputState | Function returns PWM_LOW | TODO |  |
| PWM_TC_021 | EnableNotification enables requested edge mode | SWS_Pwm_00024 | HIL | Notification configured | Enable rising/falling/both modes and trigger edges | Callback behavior matches requested mode | TODO |  |
| PWM_TC_022 | DisableNotification stops callbacks | SWS_Pwm_00023 | HIL | Notification enabled | Call Pwm_DisableNotification then trigger edges | No callback occurs | TODO |  |
| PWM_TC_023 | EnableNotification clears pending interrupts | SWS_Pwm_00081 | unit+HIL | Interrupt pending before enable | Call Pwm_EnableNotification | No stale interrupt callback after enabling | TODO |  |
| PWM_TC_024 | ISR clears interrupt flag before callback | SWS_Pwm_00026 | unit+HIL | Notification enabled | Force compare interrupt | Flag cleared and callback invoked exactly once | TODO |  |
| PWM_TC_025 | Invalid ChannelNumber raises PARAM_CHANNEL | SWS_Pwm_00047, SWS_Pwm_30002 | unit | DET enabled | Call channel APIs with invalid channel | PWM_E_PARAM_CHANNEL reported | TODO |  |
| PWM_TC_026 | Development error handling skips functional action | SWS_Pwm_20051 | unit | Inject any DET-triggering condition | Call target API | No HW state/register corruption due to rejected call | TODO |  |
| PWM_TC_027 | VersionInfo NULL pointer raises PARAM_POINTER | SWS_Pwm_00201 | unit | DET enabled | Call Pwm_GetVersionInfo(NULL) | PWM_E_PARAM_POINTER reported | TODO |  |
| PWM_TC_028 | Power API availability controlled by PwmLowPowerStatesSupport | SWS_Pwm_00154, SWS_Pwm_00155 | build | Generate config with support ON/OFF | Build both variants | Power APIs generated only when support is enabled | TODO |  |
| PWM_TC_029 | PreparePowerState then SetPowerState valid sequence | SWS_Pwm_00157, SWS_Pwm_00158, SWS_Pwm_00159 | unit+HIL | Low-power support enabled | Call Prepare then Set | Transition accepted; no sequence error | TODO |  |
| PWM_TC_030 | SetPowerState before PreparePowerState rejected | SWS_Pwm_00159, SWS_Pwm_00176 | unit | Low-power support enabled | Call SetPowerState directly | PWM_E_PERIPHERAL_NOT_PREPARED (or sequence error result) | TODO |  |
| PWM_TC_031 | SetPowerState rejects unsupported state | SWS_Pwm_00174, SWS_Pwm_00194 | unit | Low-power support enabled | Request unsupported power state | PWM_E_POWER_STATE_NOT_SUPPORTED reported | TODO |  |
| PWM_TC_032 | SetPowerState rejects non-disengaged HW | SWS_Pwm_00200, SWS_Pwm_00173 | HIL | At least one channel active/non-idle | Call Pwm_SetPowerState | Runtime error PWM_E_NOT_DISENGAGED | TODO |  |
| PWM_TC_033 | Current/Target power state getters report UNINIT when needed | SWS_Pwm_00179, SWS_Pwm_00182 | unit | Driver uninitialized | Call getter APIs | Development error PWM_E_UNINIT | TODO |  |
| PWM_TC_034 | Main_PowerTransitionManager in uninitialized state returns silently | SWS_Pwm_00193 | unit | Driver uninitialized | Call Pwm_Main_PowerTransitionManager | Function returns without DET | TODO |  |
| PWM_TC_035 | Reentrancy for different channels | SWS_Pwm_00088 | stress | Two channels configured | Call APIs concurrently on different channels | No cross-channel corruption | TODO |  |
| PWM_TC_036 | Same-channel concurrency integrity handled by upper layer | SWS_Pwm_00089 | integration | Concurrent tasks/ISR target same channel | Run contention scenario | Behavior matches integrator protection strategy | TODO |  |
| PWM_TC_037 | Optional API switches compile ON/OFF | SWS_Pwm_10080, SWS_Pwm_10082, SWS_Pwm_10083, SWS_Pwm_10084, SWS_Pwm_10085 | build | Prepare build variants | Toggle optional API switches | Generated code exports only enabled APIs | TODO |  |
| PWM_TC_038 | Notification support switch gating | SWS_Pwm_10112, SWS_Pwm_10113, SWS_Pwm_10115, SWS_Pwm_20115 | build+unit | Build with notification support OFF | Attempt to use notification APIs | APIs disabled or return safe behavior per config | TODO |  |

## Execution Notes

- Unit tests: run on host with register/HAL stubs and DET spies.
- HIL tests: run on STM32F103 board with oscilloscope/logic-analyzer capture.
- Build tests: compile matrix with different config switches.
