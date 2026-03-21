# Detailed Implementation Plan - AUTOSAR PWM for STM32F103 (SPL)

- Generated on: `2026-03-21`
- Based on SWS release: `R25-11`
- Target MCU: `STM32F103` | Library: `SPL`

## Current Baseline

- Existing files: `Pwm.h`, `Pwm.c`, `Pwm_Lcfg.h`, `Pwm_Lcfg.c`.
- APIs not yet available in current header/source baseline: `Pwm_SetPowerState, Pwm_GetCurrentPowerState, Pwm_GetTargetPowerState, Pwm_PreparePowerState, Pwm_Main_PowerTransitionManager`
- Immediate corrective action: fix `Pwm_SetDutyCycle` channelConfig scope/logic bug before extending features.

## Phase Plan

### Phase 0 - Stabilize Baseline (1-2 days)
- Fix compile/runtime defects in existing `Pwm.c`.
- Normalize channel mapping model: logical channel ID vs TIM channel index.
- Add minimal smoke test app for TIM2/TIM3 outputs.
- Exit criteria: stable waveform with Init/SetDuty/DeInit.

### Phase 1 - AUTOSAR Interface Completion (2-3 days)
- Add missing prototypes/types/macros in `Pwm.h`.
- Define service IDs and DET/runtime error codes.
- Add optional API compile-switch macros in `Pwm_Cfg.h` equivalent.
- Exit criteria: all SWS APIs visible at interface level.

### Phase 2 - Configuration Model (2-3 days)
- Implement `PwmGeneral`/`PwmChannel`/optional API switches in generated config structures.
- Ensure channel class, default period/duty, polarity, idle state, notification callback are configurable.
- Exit criteria: one config set can express fixed and variable period channels.

### Phase 3 - Core API Compliance (4-6 days)
- Implement/adjust: `Pwm_Init`, `Pwm_DeInit`, `Pwm_SetDutyCycle`, `Pwm_SetPeriodAndDuty`, `Pwm_SetOutputToIdle`, `Pwm_GetOutputState`.
- Enforce duty scaling (0x0000..0x8000), channel-class rules, and end-of-period update modes.
- Suppress spikes via preload/update-event strategy.
- Exit criteria: all Phase-3 checklist tests pass on board.

### Phase 4 - Notifications + ISR Layer (2-4 days)
- Implement `Pwm_EnableNotification`/`Pwm_DisableNotification` gating by config.
- Add `Pwm_Irq.c` to clear interrupt flags and dispatch `Pwm_Notification_<#Channel>`.
- Exit criteria: callback edge tests pass and no stale interrupt pending.

### Phase 5 - DET/Runtime Error Handling (2-3 days)
- Integrate `Det_ReportError`/`Det_ReportRuntimeError` wrappers.
- Apply checks: UNINIT, invalid channel, period unchangeable, null pointers, already initialized.
- Exit criteria: negative tests confirm skip-on-error behavior.

### Phase 6 - Power State APIs (Optional) (3-5 days)
- Implement: `Pwm_PreparePowerState`, `Pwm_SetPowerState`, `Pwm_GetCurrentPowerState`, `Pwm_GetTargetPowerState`.
- Implement `Pwm_Main_PowerTransitionManager` for async transition mode.
- Gate full feature by `PwmLowPowerStatesSupport`.
- Exit criteria: sequence + runtime disengage tests pass.

### Phase 7 - Verification & Compliance Closure (3-5 days)
- Execute full checklist in `PWM_TEST_CHECKLIST.md`.
- Capture logic analyzer evidence for duty/period/idle/notification timing.
- Freeze traceability matrix with final status and evidence links.
- Exit criteria: all mandatory requirements mapped to passing tests or justified N/A.

## Deliverables

- Production files: `Pwm.h`, `Pwm.c`, `Pwm_Irq.c`, `Pwm_Cfg.h`, `Pwm_Cfg.c`, `Pwm_Lcfg.h`, `Pwm_Lcfg.c`.
- Validation files: `PWM_TRACEABILITY_MATRIX.md`, `PWM_TEST_CHECKLIST.md`, oscilloscope captures.
- Optional: host-based unit-test harness and CI build matrix for config-switch combinations.
