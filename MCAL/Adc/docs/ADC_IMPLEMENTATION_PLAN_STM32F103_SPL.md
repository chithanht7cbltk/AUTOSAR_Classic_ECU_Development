# Detailed Implementation Plan - AUTOSAR ADC for STM32F103 (SPL)

- Generated on: `2026-03-21`
- Based on SWS release: `R25-11`
- Target MCU: `STM32F103` | Library: `SPL`

## Current Baseline

- Existing files: `Adc.h`, `Adc.c`, `Adc_Types.h`, `Adc_Cfg.h`, `Adc_Cfg.c`.
- APIs not yet available in current header/source baseline: `none`
- Immediate action: align function signatures with SWS and standardize return/error handling.

## Phase Plan

### Phase 0 - Baseline Hardening (1-2 days)
- Clean compile warnings and enforce consistent enum/typedef use.
- Verify ISR/DMA handler hooks and callback execution path.
- Exit criteria: stable init/start/read/stop behavior in current examples.

### Phase 1 - Interface & Type Compliance (2-3 days)
- Align API signatures and service IDs with SWS section 8.3.
- Complete power-state API signatures and return/result types.
- Exit criteria: API table in traceability matrix marked declared+defined.

### Phase 2 - Configuration Model (2-4 days)
- Align `AdcGeneral`, `AdcGroup`, `AdcChannel`, `AdcHwUnit` with ECUC definitions.
- Add/gate optional API switches (ReadGroup, StartStopGroup, HwTrigger, VersionInfo, DeInit).
- Exit criteria: build variants for ON/OFF switch matrix pass.

### Phase 3 - Group Conversion Core (4-6 days)
- Implement strict state machine for one-shot/continuous/software/hardware trigger.
- Ensure `Adc_SetupResultBuffer`, `Adc_ReadGroup`, `Adc_GetStreamLastPointer` follow SWS.
- Exit criteria: group status transitions verified in SIL/HIL tests.

### Phase 4 - Notifications, IRQ, DMA Streaming (3-5 days)
- Finalize EOC/JEOC/DMATC interrupt routing and callback dispatch.
- Validate circular DMA/streaming access and buffer ownership constraints.
- Exit criteria: notification and streaming checklist items PASS.

### Phase 5 - Error Handling & DET/Runtime (2-3 days)
- Map all ADC_E_* development/runtime errors to service calls.
- Enforce skip-on-error behavior and no side-effects on invalid calls.
- Exit criteria: negative tests PASS with clear error evidence.

### Phase 6 - Power State (Optional) (2-4 days)
- Implement prepare/set/get current/get target + async transition manager.
- Gate by `AdcLowPowerStatesSupport` and async mode switch.
- Exit criteria: power transition scenarios PASS.

### Phase 7 - Compliance Closure (3-5 days)
- Execute full checklist and freeze traceability with evidence links.
- Collect Renode logs + board logs as objective verification artifacts.
- Exit criteria: mandatory requirements closed or justified N/A.

## Deliverables

- Production files: `Adc.h`, `Adc.c`, `Adc_Irq.c`, `Adc_Dma.c`, `Adc_Types.h`, `Adc_Cfg.h`, `Adc_Cfg.c`.
- Validation files: `ADC_TRACEABILITY_MATRIX.md`, `ADC_TEST_CHECKLIST.md`, test logs.
- Optional: Renode+GDB automated test runner for ADC APIs.
