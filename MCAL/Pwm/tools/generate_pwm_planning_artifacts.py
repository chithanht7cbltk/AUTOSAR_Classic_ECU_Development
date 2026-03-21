#!/usr/bin/env python3
"""Generate PWM planning artifacts from extracted AUTOSAR SWS JSON.

Outputs:
- PWM_TRACEABILITY_MATRIX.md
- PWM_TRACEABILITY_MATRIX.csv
- PWM_TEST_CHECKLIST.md
- PWM_IMPLEMENTATION_PLAN_STM32F103_SPL.md
"""

from __future__ import annotations

import argparse
import csv
import json
import re
from dataclasses import dataclass
from datetime import date
from pathlib import Path
from typing import Any


CONFIG_SWITCH_BY_API = {
    "Pwm_DeInit": "PwmDeInitApi",
    "Pwm_SetDutyCycle": "PwmSetDutyCycle",
    "Pwm_SetPeriodAndDuty": "PwmSetPeriodAndDuty",
    "Pwm_SetOutputToIdle": "PwmSetOutputToIdle",
    "Pwm_GetOutputState": "PwmGetOutputState",
    "Pwm_GetVersionInfo": "PwmVersionInfoApi",
    "Pwm_EnableNotification": "PwmNotificationSupported",
    "Pwm_DisableNotification": "PwmNotificationSupported",
    "Pwm_SetPowerState": "PwmLowPowerStatesSupport",
    "Pwm_GetCurrentPowerState": "PwmLowPowerStatesSupport",
    "Pwm_GetTargetPowerState": "PwmLowPowerStatesSupport",
    "Pwm_PreparePowerState": "PwmLowPowerStatesSupport",
    "Pwm_Main_PowerTransitionManager": "PwmLowPowerStatesSupport + PwmPowerStateAsynchTransitionMode",
}


CHANNEL_APIS = {
    "Pwm_SetDutyCycle",
    "Pwm_SetPeriodAndDuty",
    "Pwm_SetOutputToIdle",
    "Pwm_GetOutputState",
    "Pwm_EnableNotification",
    "Pwm_DisableNotification",
}


def squash_ws(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


def escape_md(text: str) -> str:
    return text.replace("|", "\\|")


def read_text_if_exists(path: Path) -> str:
    if not path.exists():
        return ""
    return path.read_text(encoding="utf-8", errors="ignore")


def has_declaration(header_text: str, func_name: str) -> bool:
    pat = re.compile(rf"\b{re.escape(func_name)}\s*\([^;]*\)\s*;", re.S)
    return bool(pat.search(header_text))


def has_definition(source_text: str, func_name: str) -> bool:
    pat = re.compile(
        rf"^\s*[A-Za-z_][A-Za-z0-9_\s\*]*\b{re.escape(func_name)}\s*\([^;]*\)\s*(?:\n\s*)?\{{",
        re.M,
    )
    return bool(pat.search(source_text))


def has_error_symbol(code_text: str, symbol: str) -> bool:
    return bool(re.search(rf"\b{re.escape(symbol)}\b", code_text))


@dataclass
class ArtifactContext:
    data: dict[str, Any]
    project_root: Path
    pwm_root: Path
    header_text: str
    source_text: str
    code_text: str


@dataclass
class TestCase:
    tc_id: str
    title: str
    requirement_ids: list[str]
    level: str
    precondition: str
    steps: str
    expected: str


def load_context(json_path: Path, header_path: Path, source_glob: str) -> ArtifactContext:
    data = json.loads(json_path.read_text(encoding="utf-8"))
    pwm_root = header_path.parent
    project_root = pwm_root.parent

    header_text = read_text_if_exists(header_path)
    source_paths = sorted(pwm_root.glob(source_glob))
    source_text = "\n\n".join(read_text_if_exists(p) for p in source_paths)
    code_text = "\n\n".join([header_text, source_text])

    return ArtifactContext(
        data=data,
        project_root=project_root,
        pwm_root=pwm_root,
        header_text=header_text,
        source_text=source_text,
        code_text=code_text,
    )


def matrix_rows(ctx: ArtifactContext) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    apis = ctx.data.get("apis", [])

    for api in apis:
        name = api.get("name", "")
        sid = api.get("service_id_hex", "") or "-"
        req_ids = api.get("requirement_ids", [])
        req_short = ", ".join(req_ids[:8])
        if len(req_ids) > 8:
            req_short += ", ..."

        header_ok = has_declaration(ctx.header_text, name)
        source_ok = has_definition(ctx.source_text, name)
        switch = CONFIG_SWITCH_BY_API.get(name, "mandatory")

        if header_ok and source_ok:
            status = "implemented"
            note = ""
        elif header_ok and not source_ok:
            status = "declared_only"
            note = "Missing source definition"
        elif not header_ok and source_ok:
            status = "defined_not_declared"
            note = "Missing header declaration"
        else:
            status = "missing"
            note = "Not present in current module"

        rows.append(
            {
                "api": name,
                "service_id": sid,
                "section": api.get("section", ""),
                "config_switch": switch,
                "req_ids": req_short,
                "header_declared": "YES" if header_ok else "NO",
                "source_defined": "YES" if source_ok else "NO",
                "status": status,
                "notes": note,
            }
        )

    return rows


def error_rows(ctx: ArtifactContext) -> list[dict[str, str]]:
    out: list[dict[str, str]] = []
    errors = ctx.data.get("errors", {})

    for category in ("development", "runtime"):
        for item in errors.get(category, []):
            name = item.get("name", "")
            out.append(
                {
                    "category": category,
                    "error": name,
                    "value": item.get("value", ""),
                    "symbol_in_code": "YES" if has_error_symbol(ctx.code_text, name) else "NO",
                }
            )
    return out


def write_traceability_md(ctx: ArtifactContext, out_path: Path) -> None:
    rows = matrix_rows(ctx)
    errors = error_rows(ctx)

    total = len(rows)
    implemented = sum(1 for r in rows if r["status"] == "implemented")
    missing = sum(1 for r in rows if r["status"] == "missing")

    lines: list[str] = []
    lines.append("# PWM SWS Traceability Matrix")
    lines.append("")
    lines.append(f"- Source: `{ctx.data['document'].get('source_pdf', '')}`")
    lines.append(f"- AUTOSAR release: `{ctx.data['document'].get('release', 'N/A')}`")
    lines.append(f"- Generated on: `{date.today().isoformat()}`")
    lines.append("")
    lines.append("## API Coverage Summary")
    lines.append("")
    lines.append(f"- Total APIs in extracted SWS scope: **{total}**")
    lines.append(f"- Fully implemented (declared+defined): **{implemented}**")
    lines.append(f"- Missing in current module: **{missing}**")
    lines.append("")
    lines.append("## API Matrix")
    lines.append("")
    lines.append("| API | SID | Section | Config Switch | Key SWS IDs | Header | Source | Status | Notes |")
    lines.append("|---|---|---|---|---|---|---|---|---|")
    for r in rows:
        lines.append(
            "| "
            + " | ".join(
                [
                    escape_md(r["api"]),
                    escape_md(r["service_id"]),
                    escape_md(r["section"]),
                    escape_md(r["config_switch"]),
                    escape_md(r["req_ids"]),
                    r["header_declared"],
                    r["source_defined"],
                    r["status"],
                    escape_md(r["notes"]),
                ]
            )
            + " |"
        )

    lines.append("")
    lines.append("## Error Symbol Coverage")
    lines.append("")
    lines.append("| Category | Error Symbol | Value | Symbol in Current Code |")
    lines.append("|---|---|---|---|")
    for e in errors:
        lines.append(
            f"| {e['category']} | `{escape_md(e['error'])}` | `{escape_md(e['value'])}` | {e['symbol_in_code']} |"
        )

    lines.append("")
    lines.append("## Notes")
    lines.append("")
    lines.append("- This matrix is generated from extracted SWS metadata + static scan of current `Pwm.h` and `Pwm.c` files.")
    lines.append("- For formal compliance, keep this matrix synchronized with implementation and test evidence.")

    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_traceability_csv(ctx: ArtifactContext, out_path: Path) -> None:
    rows = matrix_rows(ctx)
    with out_path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "api",
                "service_id",
                "section",
                "config_switch",
                "req_ids",
                "header_declared",
                "source_defined",
                "status",
                "notes",
            ],
        )
        writer.writeheader()
        writer.writerows(rows)


def build_test_cases() -> list[TestCase]:
    t: list[TestCase] = []
    add = t.append

    add(TestCase("PWM_TC_001", "Pwm_Init initializes configured channels only", ["SWS_Pwm_00007", "SWS_Pwm_00062"], "unit+HIL", "Driver uninitialized", "Call Pwm_Init with valid config containing subset of channels", "Only configured TIM/CH resources are touched"))
    add(TestCase("PWM_TC_002", "Pwm_Init starts channels with default period/duty/polarity", ["SWS_Pwm_10009", "SWS_Pwm_20009", "SWS_Pwm_30009"], "HIL", "Clock+Port configured", "Call Pwm_Init then observe waveform", "Default waveform matches configuration"))
    add(TestCase("PWM_TC_003", "Pwm_Init disables all notifications", ["SWS_Pwm_00052"], "unit+HIL", "Notification callbacks configured", "Call Pwm_Init then trigger compare events", "No callback is fired before EnableNotification"))
    add(TestCase("PWM_TC_004", "Pwm_Init twice reports already initialized", ["SWS_Pwm_00118", "SWS_Pwm_50002"], "unit", "DET enabled, driver initialized", "Call Pwm_Init again", "DET gets PWM_E_ALREADY_INITIALIZED and state remains unchanged"))
    add(TestCase("PWM_TC_005", "Pwm_DeInit sets output to idle and disables notifications", ["SWS_Pwm_00010", "SWS_Pwm_00011", "SWS_Pwm_00012"], "HIL", "Driver initialized and running PWM", "Call Pwm_DeInit", "Outputs go idle, interrupts/callbacks disabled"))
    add(TestCase("PWM_TC_006", "API call before init raises UNINIT", ["SWS_Pwm_00117", "SWS_Pwm_20002"], "unit", "DET enabled, driver uninitialized", "Call each API except Pwm_Init", "DET logs PWM_E_UNINIT and action skipped"))

    add(TestCase("PWM_TC_007", "SetDutyCycle with 0% follows polarity inversion rule", ["SWS_Pwm_00014"], "HIL", "Driver initialized", "Call Pwm_SetDutyCycle(ch,0x0000)", "Output steady state is inverse of configured polarity"))
    add(TestCase("PWM_TC_008", "SetDutyCycle with 100% follows polarity rule", ["SWS_Pwm_00014"], "HIL", "Driver initialized", "Call Pwm_SetDutyCycle(ch,0x8000)", "Output steady state equals configured polarity"))
    add(TestCase("PWM_TC_009", "SetDutyCycle in (0,100%) modulates correctly", ["SWS_Pwm_00016"], "HIL", "Driver initialized", "Set duty to mid value and sample waveform", "Measured duty equals configured ratio"))
    add(TestCase("PWM_TC_010", "Duty scaling formula uses 0x8000 as full scale", ["SWS_Pwm_00058", "SWS_Pwm_00059"], "unit", "Driver initialized", "Set multiple duty values and inspect CCR", "CCR = (Period * Duty16)>>15"))
    add(TestCase("PWM_TC_011", "Duty update timing follows PwmDutycycleUpdatedEndperiod", ["SWS_Pwm_00017"], "HIL", "Two builds: endperiod ON/OFF", "Update duty during active period", "Update occurs at end-of-period when ON, immediate when OFF"))

    add(TestCase("PWM_TC_012", "SetPeriodAndDuty changes both period and duty", ["SWS_Pwm_00019"], "HIL", "Variable-period channel", "Call Pwm_SetPeriodAndDuty", "ARR/CCR updated consistently"))
    add(TestCase("PWM_TC_013", "SetPeriodAndDuty only allowed on VARIABLE_PERIOD", ["SWS_Pwm_00041", "SWS_Pwm_00045", "SWS_Pwm_40002"], "unit", "DET enabled, fixed-period channel", "Call Pwm_SetPeriodAndDuty", "PWM_E_PERIOD_UNCHANGEABLE is reported and no register update"))
    add(TestCase("PWM_TC_014", "Period update timing follows PwmPeriodUpdatedEndperiod", ["SWS_Pwm_00076"], "HIL", "Two builds: endperiod ON/OFF", "Update period while running", "Update timing matches configuration"))
    add(TestCase("PWM_TC_015", "SetPeriodAndDuty with Period=0 results in 0% output", ["SWS_Pwm_00150"], "HIL", "Variable-period channel", "Call Pwm_SetPeriodAndDuty(ch,0,duty)", "Output remains low/0%"))

    add(TestCase("PWM_TC_016", "SetOutputToIdle acts immediately", ["SWS_Pwm_00021"], "HIL", "Channel running PWM", "Call Pwm_SetOutputToIdle", "Output transitions immediately to configured idle state"))
    add(TestCase("PWM_TC_017", "Reactivation path after SetOutputToIdle for variable period", ["SWS_Pwm_10086"], "HIL", "Variable-period channel idle", "Reactivate using Pwm_SetPeriodAndDuty", "Waveform resumes with new period"))
    add(TestCase("PWM_TC_018", "Reactivation path after SetOutputToIdle for fixed period", ["SWS_Pwm_00119", "SWS_Pwm_20086"], "HIL", "Fixed-period channel idle", "Reactivate using Pwm_SetDutyCycle", "Waveform resumes with previous period"))

    add(TestCase("PWM_TC_019", "GetOutputState returns current internal output state", ["SWS_Pwm_00022"], "unit+HIL", "Driver initialized", "Call Pwm_GetOutputState in different states", "Return reflects internal high/low state"))
    add(TestCase("PWM_TC_020", "GetOutputState before init or invalid channel returns PWM_LOW", ["SWS_Pwm_30051"], "unit", "Driver uninitialized or bad channel", "Call Pwm_GetOutputState", "Function returns PWM_LOW"))

    add(TestCase("PWM_TC_021", "EnableNotification enables requested edge mode", ["SWS_Pwm_00024"], "HIL", "Notification configured", "Enable rising/falling/both modes and trigger edges", "Callback behavior matches requested mode"))
    add(TestCase("PWM_TC_022", "DisableNotification stops callbacks", ["SWS_Pwm_00023"], "HIL", "Notification enabled", "Call Pwm_DisableNotification then trigger edges", "No callback occurs"))
    add(TestCase("PWM_TC_023", "EnableNotification clears pending interrupts", ["SWS_Pwm_00081"], "unit+HIL", "Interrupt pending before enable", "Call Pwm_EnableNotification", "No stale interrupt callback after enabling"))
    add(TestCase("PWM_TC_024", "ISR clears interrupt flag before callback", ["SWS_Pwm_00026"], "unit+HIL", "Notification enabled", "Force compare interrupt", "Flag cleared and callback invoked exactly once"))

    add(TestCase("PWM_TC_025", "Invalid ChannelNumber raises PARAM_CHANNEL", ["SWS_Pwm_00047", "SWS_Pwm_30002"], "unit", "DET enabled", "Call channel APIs with invalid channel", "PWM_E_PARAM_CHANNEL reported"))
    add(TestCase("PWM_TC_026", "Development error handling skips functional action", ["SWS_Pwm_20051"], "unit", "Inject any DET-triggering condition", "Call target API", "No HW state/register corruption due to rejected call"))
    add(TestCase("PWM_TC_027", "VersionInfo NULL pointer raises PARAM_POINTER", ["SWS_Pwm_00201"], "unit", "DET enabled", "Call Pwm_GetVersionInfo(NULL)", "PWM_E_PARAM_POINTER reported"))

    add(TestCase("PWM_TC_028", "Power API availability controlled by PwmLowPowerStatesSupport", ["SWS_Pwm_00154", "SWS_Pwm_00155"], "build", "Generate config with support ON/OFF", "Build both variants", "Power APIs generated only when support is enabled"))
    add(TestCase("PWM_TC_029", "PreparePowerState then SetPowerState valid sequence", ["SWS_Pwm_00157", "SWS_Pwm_00158", "SWS_Pwm_00159"], "unit+HIL", "Low-power support enabled", "Call Prepare then Set", "Transition accepted; no sequence error"))
    add(TestCase("PWM_TC_030", "SetPowerState before PreparePowerState rejected", ["SWS_Pwm_00159", "SWS_Pwm_00176"], "unit", "Low-power support enabled", "Call SetPowerState directly", "PWM_E_PERIPHERAL_NOT_PREPARED (or sequence error result)"))
    add(TestCase("PWM_TC_031", "SetPowerState rejects unsupported state", ["SWS_Pwm_00174", "SWS_Pwm_00194"], "unit", "Low-power support enabled", "Request unsupported power state", "PWM_E_POWER_STATE_NOT_SUPPORTED reported"))
    add(TestCase("PWM_TC_032", "SetPowerState rejects non-disengaged HW", ["SWS_Pwm_00200", "SWS_Pwm_00173"], "HIL", "At least one channel active/non-idle", "Call Pwm_SetPowerState", "Runtime error PWM_E_NOT_DISENGAGED"))
    add(TestCase("PWM_TC_033", "Current/Target power state getters report UNINIT when needed", ["SWS_Pwm_00179", "SWS_Pwm_00182"], "unit", "Driver uninitialized", "Call getter APIs", "Development error PWM_E_UNINIT"))
    add(TestCase("PWM_TC_034", "Main_PowerTransitionManager in uninitialized state returns silently", ["SWS_Pwm_00193"], "unit", "Driver uninitialized", "Call Pwm_Main_PowerTransitionManager", "Function returns without DET"))

    add(TestCase("PWM_TC_035", "Reentrancy for different channels", ["SWS_Pwm_00088"], "stress", "Two channels configured", "Call APIs concurrently on different channels", "No cross-channel corruption"))
    add(TestCase("PWM_TC_036", "Same-channel concurrency integrity handled by upper layer", ["SWS_Pwm_00089"], "integration", "Concurrent tasks/ISR target same channel", "Run contention scenario", "Behavior matches integrator protection strategy"))

    add(TestCase("PWM_TC_037", "Optional API switches compile ON/OFF", ["SWS_Pwm_10080", "SWS_Pwm_10082", "SWS_Pwm_10083", "SWS_Pwm_10084", "SWS_Pwm_10085"], "build", "Prepare build variants", "Toggle optional API switches", "Generated code exports only enabled APIs"))
    add(TestCase("PWM_TC_038", "Notification support switch gating", ["SWS_Pwm_10112", "SWS_Pwm_10113", "SWS_Pwm_10115", "SWS_Pwm_20115"], "build+unit", "Build with notification support OFF", "Attempt to use notification APIs", "APIs disabled or return safe behavior per config"))

    return t


def write_test_checklist_md(out_path: Path) -> None:
    test_cases = build_test_cases()

    lines: list[str] = []
    lines.append("# PWM Test Checklist (AUTOSAR SWS Based)")
    lines.append("")
    lines.append(f"- Generated on: `{date.today().isoformat()}`")
    lines.append("- Status values: `TODO`, `IN_PROGRESS`, `PASS`, `FAIL`, `N/A`")
    lines.append("")
    lines.append("| TC ID | Title | SWS IDs | Level | Precondition | Steps | Expected | Status | Evidence |")
    lines.append("|---|---|---|---|---|---|---|---|---|")

    for tc in test_cases:
        lines.append(
            "| "
            + " | ".join(
                [
                    tc.tc_id,
                    escape_md(tc.title),
                    escape_md(", ".join(tc.requirement_ids)),
                    tc.level,
                    escape_md(tc.precondition),
                    escape_md(tc.steps),
                    escape_md(tc.expected),
                    "TODO",
                    "",
                ]
            )
            + " |"
        )

    lines.append("")
    lines.append("## Execution Notes")
    lines.append("")
    lines.append("- Unit tests: run on host with register/HAL stubs and DET spies.")
    lines.append("- HIL tests: run on STM32F103 board with oscilloscope/logic-analyzer capture.")
    lines.append("- Build tests: compile matrix with different config switches.")

    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_implementation_plan_md(ctx: ArtifactContext, out_path: Path) -> None:
    rows = matrix_rows(ctx)
    missing = [r["api"] for r in rows if r["status"] in {"missing", "declared_only"}]
    today = date.today().isoformat()

    lines: list[str] = []
    lines.append("# Detailed Implementation Plan - AUTOSAR PWM for STM32F103 (SPL)")
    lines.append("")
    lines.append(f"- Generated on: `{today}`")
    lines.append(f"- Based on SWS release: `{ctx.data['document'].get('release', 'N/A')}`")
    lines.append("- Target MCU: `STM32F103` | Library: `SPL`")
    lines.append("")
    lines.append("## Current Baseline")
    lines.append("")
    lines.append("- Existing files: `Pwm.h`, `Pwm.c`, `Pwm_Lcfg.h`, `Pwm_Lcfg.c`.")
    lines.append(f"- APIs not yet available in current header/source baseline: `{', '.join(missing) if missing else 'none'}`")
    lines.append("- Immediate corrective action: fix `Pwm_SetDutyCycle` channelConfig scope/logic bug before extending features.")
    lines.append("")

    lines.append("## Phase Plan")
    lines.append("")
    lines.append("### Phase 0 - Stabilize Baseline (1-2 days)")
    lines.append("- Fix compile/runtime defects in existing `Pwm.c`.")
    lines.append("- Normalize channel mapping model: logical channel ID vs TIM channel index.")
    lines.append("- Add minimal smoke test app for TIM2/TIM3 outputs.")
    lines.append("- Exit criteria: stable waveform with Init/SetDuty/DeInit.")
    lines.append("")

    lines.append("### Phase 1 - AUTOSAR Interface Completion (2-3 days)")
    lines.append("- Add missing prototypes/types/macros in `Pwm.h`.")
    lines.append("- Define service IDs and DET/runtime error codes.")
    lines.append("- Add optional API compile-switch macros in `Pwm_Cfg.h` equivalent.")
    lines.append("- Exit criteria: all SWS APIs visible at interface level.")
    lines.append("")

    lines.append("### Phase 2 - Configuration Model (2-3 days)")
    lines.append("- Implement `PwmGeneral`/`PwmChannel`/optional API switches in generated config structures.")
    lines.append("- Ensure channel class, default period/duty, polarity, idle state, notification callback are configurable.")
    lines.append("- Exit criteria: one config set can express fixed and variable period channels.")
    lines.append("")

    lines.append("### Phase 3 - Core API Compliance (4-6 days)")
    lines.append("- Implement/adjust: `Pwm_Init`, `Pwm_DeInit`, `Pwm_SetDutyCycle`, `Pwm_SetPeriodAndDuty`, `Pwm_SetOutputToIdle`, `Pwm_GetOutputState`.")
    lines.append("- Enforce duty scaling (0x0000..0x8000), channel-class rules, and end-of-period update modes.")
    lines.append("- Suppress spikes via preload/update-event strategy.")
    lines.append("- Exit criteria: all Phase-3 checklist tests pass on board.")
    lines.append("")

    lines.append("### Phase 4 - Notifications + ISR Layer (2-4 days)")
    lines.append("- Implement `Pwm_EnableNotification`/`Pwm_DisableNotification` gating by config.")
    lines.append("- Add `Pwm_Irq.c` to clear interrupt flags and dispatch `Pwm_Notification_<#Channel>`.")
    lines.append("- Exit criteria: callback edge tests pass and no stale interrupt pending.")
    lines.append("")

    lines.append("### Phase 5 - DET/Runtime Error Handling (2-3 days)")
    lines.append("- Integrate `Det_ReportError`/`Det_ReportRuntimeError` wrappers.")
    lines.append("- Apply checks: UNINIT, invalid channel, period unchangeable, null pointers, already initialized.")
    lines.append("- Exit criteria: negative tests confirm skip-on-error behavior.")
    lines.append("")

    lines.append("### Phase 6 - Power State APIs (Optional) (3-5 days)")
    lines.append("- Implement: `Pwm_PreparePowerState`, `Pwm_SetPowerState`, `Pwm_GetCurrentPowerState`, `Pwm_GetTargetPowerState`.")
    lines.append("- Implement `Pwm_Main_PowerTransitionManager` for async transition mode.")
    lines.append("- Gate full feature by `PwmLowPowerStatesSupport`.")
    lines.append("- Exit criteria: sequence + runtime disengage tests pass.")
    lines.append("")

    lines.append("### Phase 7 - Verification & Compliance Closure (3-5 days)")
    lines.append("- Execute full checklist in `PWM_TEST_CHECKLIST.md`.")
    lines.append("- Capture logic analyzer evidence for duty/period/idle/notification timing.")
    lines.append("- Freeze traceability matrix with final status and evidence links.")
    lines.append("- Exit criteria: all mandatory requirements mapped to passing tests or justified N/A.")
    lines.append("")

    lines.append("## Deliverables")
    lines.append("")
    lines.append("- Production files: `Pwm.h`, `Pwm.c`, `Pwm_Irq.c`, `Pwm_Cfg.h`, `Pwm_Cfg.c`, `Pwm_Lcfg.h`, `Pwm_Lcfg.c`.")
    lines.append("- Validation files: `PWM_TRACEABILITY_MATRIX.md`, `PWM_TEST_CHECKLIST.md`, oscilloscope captures.")
    lines.append("- Optional: host-based unit-test harness and CI build matrix for config-switch combinations.")

    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Generate PWM planning artifacts from extracted JSON")
    p.add_argument("--input", type=Path, default=Path("Pwm/tools/pwm_sws_r25_11_extracted.json"), help="Input extracted JSON")
    p.add_argument("--header", type=Path, default=Path("Pwm/Pwm.h"), help="Path to Pwm header")
    p.add_argument("--source-glob", default="*.c", help="Glob for source files under Pwm directory")
    p.add_argument("--out-dir", type=Path, default=Path("Pwm/docs"), help="Output documentation directory")
    return p.parse_args()


def main() -> None:
    args = parse_args()
    if not args.input.exists():
        raise SystemExit(f"Input JSON not found: {args.input}")
    if not args.header.exists():
        raise SystemExit(f"Header not found: {args.header}")

    ctx = load_context(args.input, args.header, args.source_glob)
    args.out_dir.mkdir(parents=True, exist_ok=True)

    trace_md = args.out_dir / "PWM_TRACEABILITY_MATRIX.md"
    trace_csv = args.out_dir / "PWM_TRACEABILITY_MATRIX.csv"
    checklist_md = args.out_dir / "PWM_TEST_CHECKLIST.md"
    plan_md = args.out_dir / "PWM_IMPLEMENTATION_PLAN_STM32F103_SPL.md"

    write_traceability_md(ctx, trace_md)
    write_traceability_csv(ctx, trace_csv)
    write_test_checklist_md(checklist_md)
    write_implementation_plan_md(ctx, plan_md)

    print(f"Generated: {trace_md}")
    print(f"Generated: {trace_csv}")
    print(f"Generated: {checklist_md}")
    print(f"Generated: {plan_md}")


if __name__ == "__main__":
    main()
