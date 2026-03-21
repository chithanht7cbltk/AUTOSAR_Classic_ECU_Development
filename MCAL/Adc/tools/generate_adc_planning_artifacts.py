#!/usr/bin/env python3
"""Generate ADC planning artifacts from extracted AUTOSAR SWS JSON.

Outputs:
- ADC_TRACEABILITY_MATRIX.md
- ADC_TRACEABILITY_MATRIX.csv
- ADC_TEST_CHECKLIST.md
- ADC_IMPLEMENTATION_PLAN_STM32F103_SPL.md
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
    "Adc_DeInit": "AdcDeInitApi",
    "Adc_StartGroupConversion": "AdcEnableStartStopGroupApi",
    "Adc_StopGroupConversion": "AdcEnableStartStopGroupApi",
    "Adc_ReadGroup": "AdcReadGroupApi",
    "Adc_EnableHardwareTrigger": "AdcHwTriggerApi",
    "Adc_DisableHardwareTrigger": "AdcHwTriggerApi",
    "Adc_EnableGroupNotification": "AdcGrpNotifCapability",
    "Adc_DisableGroupNotification": "AdcGrpNotifCapability",
    "Adc_GetVersionInfo": "AdcVersionInfoApi",
    "Adc_SetPowerState": "AdcLowPowerStatesSupport",
    "Adc_GetCurrentPowerState": "AdcLowPowerStatesSupport",
    "Adc_GetTargetPowerState": "AdcLowPowerStatesSupport",
    "Adc_PreparePowerState": "AdcLowPowerStatesSupport",
    "Adc_Main_PowerTransitionManager": "AdcLowPowerStatesSupport + AdcPowerStateAsynchTransitionMode",
}


GROUP_APIS = {
    "Adc_SetupResultBuffer",
    "Adc_StartGroupConversion",
    "Adc_StopGroupConversion",
    "Adc_ReadGroup",
    "Adc_EnableHardwareTrigger",
    "Adc_DisableHardwareTrigger",
    "Adc_EnableGroupNotification",
    "Adc_DisableGroupNotification",
    "Adc_GetGroupStatus",
    "Adc_GetStreamLastPointer",
}


def squash_ws(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


def normalize_symbol(text: str) -> str:
    return squash_ws(text).replace(" ", "")


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
    adc_root: Path
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
    adc_root = header_path.parent
    project_root = adc_root.parent

    header_text = read_text_if_exists(header_path)
    source_paths = sorted(adc_root.glob(source_glob))
    source_text = "\n\n".join(read_text_if_exists(p) for p in source_paths)
    code_text = "\n\n".join([header_text, source_text])

    return ArtifactContext(
        data=data,
        project_root=project_root,
        adc_root=adc_root,
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
            name = normalize_symbol(item.get("name", ""))
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
    lines.append("# ADC SWS Traceability Matrix")
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
    lines.append("- This matrix is generated from extracted SWS metadata + static scan of current `Adc.h` and `Adc.c` files.")
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


def build_test_cases(ctx: ArtifactContext) -> list[TestCase]:
    apis = ctx.data.get("apis", [])
    test_cases: list[TestCase] = []

    tc_idx = 1
    for api in apis:
        name = api.get("name", "")
        req_ids = api.get("requirement_ids", [])
        req_slice = req_ids[:4] if req_ids else []

        if name in {"Adc_Init", "Adc_DeInit"}:
            level = "unit+HIL"
            pre = "ADC driver uninitialized"
            steps = f"Call `{name}` with expected config/state."
            expected = "Driver state and HW state transition correctly."
        elif name in GROUP_APIS:
            level = "unit+HIL"
            pre = "Driver initialized, group configured"
            steps = f"Call `{name}` with valid/invalid group IDs."
            expected = "Group state transitions and return values follow SWS."
        elif "PowerState" in name or "PowerTransition" in name:
            level = "unit"
            pre = "Low-power support enabled in config"
            steps = f"Execute power-state sequence containing `{name}`."
            expected = "Return code and state machine behavior follow SWS."
        else:
            level = "unit"
            pre = "Driver initialized"
            steps = f"Call `{name}` and inspect return/output."
            expected = "API behavior follows SWS definition."

        test_cases.append(
            TestCase(
                tc_id=f"ADC_TC_{tc_idx:03d}",
                title=f"{name} functional behavior",
                requirement_ids=req_slice,
                level=level,
                precondition=pre,
                steps=steps,
                expected=expected,
            )
        )
        tc_idx += 1

        # Add one negative test per API for robustness.
        test_cases.append(
            TestCase(
                tc_id=f"ADC_TC_{tc_idx:03d}",
                title=f"{name} negative path (invalid precondition)",
                requirement_ids=req_slice,
                level="unit",
                precondition="Trigger DET/runtime precondition violation",
                steps=f"Call `{name}` with invalid state/argument sequence.",
                expected="Error is reported and no unintended side-effects occur.",
            )
        )
        tc_idx += 1

    return test_cases


def write_test_checklist_md(ctx: ArtifactContext, out_path: Path) -> None:
    test_cases = build_test_cases(ctx)

    lines: list[str] = []
    lines.append("# ADC Test Checklist (AUTOSAR SWS Based)")
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
    lines.append("- Unit tests: run on host with SPL register/HAL stubs and DET spies.")
    lines.append("- SIL tests: run on Renode + GDB automation for group state and register checks.")
    lines.append("- HIL tests: run on STM32F103 board with known analog source and capture logs.")

    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_implementation_plan_md(ctx: ArtifactContext, out_path: Path) -> None:
    rows = matrix_rows(ctx)
    missing = [r["api"] for r in rows if r["status"] in {"missing", "declared_only"}]
    today = date.today().isoformat()

    lines: list[str] = []
    lines.append("# Detailed Implementation Plan - AUTOSAR ADC for STM32F103 (SPL)")
    lines.append("")
    lines.append(f"- Generated on: `{today}`")
    lines.append(f"- Based on SWS release: `{ctx.data['document'].get('release', 'N/A')}`")
    lines.append("- Target MCU: `STM32F103` | Library: `SPL`")
    lines.append("")
    lines.append("## Current Baseline")
    lines.append("")
    lines.append("- Existing files: `Adc.h`, `Adc.c`, `Adc_Types.h`, `Adc_Cfg.h`, `Adc_Cfg.c`.")
    lines.append(f"- APIs not yet available in current header/source baseline: `{', '.join(missing) if missing else 'none'}`")
    lines.append("- Immediate action: align function signatures with SWS and standardize return/error handling.")
    lines.append("")
    lines.append("## Phase Plan")
    lines.append("")
    lines.append("### Phase 0 - Baseline Hardening (1-2 days)")
    lines.append("- Clean compile warnings and enforce consistent enum/typedef use.")
    lines.append("- Verify ISR/DMA handler hooks and callback execution path.")
    lines.append("- Exit criteria: stable init/start/read/stop behavior in current examples.")
    lines.append("")
    lines.append("### Phase 1 - Interface & Type Compliance (2-3 days)")
    lines.append("- Align API signatures and service IDs with SWS section 8.3.")
    lines.append("- Complete power-state API signatures and return/result types.")
    lines.append("- Exit criteria: API table in traceability matrix marked declared+defined.")
    lines.append("")
    lines.append("### Phase 2 - Configuration Model (2-4 days)")
    lines.append("- Align `AdcGeneral`, `AdcGroup`, `AdcChannel`, `AdcHwUnit` with ECUC definitions.")
    lines.append("- Add/gate optional API switches (ReadGroup, StartStopGroup, HwTrigger, VersionInfo, DeInit).")
    lines.append("- Exit criteria: build variants for ON/OFF switch matrix pass.")
    lines.append("")
    lines.append("### Phase 3 - Group Conversion Core (4-6 days)")
    lines.append("- Implement strict state machine for one-shot/continuous/software/hardware trigger.")
    lines.append("- Ensure `Adc_SetupResultBuffer`, `Adc_ReadGroup`, `Adc_GetStreamLastPointer` follow SWS.")
    lines.append("- Exit criteria: group status transitions verified in SIL/HIL tests.")
    lines.append("")
    lines.append("### Phase 4 - Notifications, IRQ, DMA Streaming (3-5 days)")
    lines.append("- Finalize EOC/JEOC/DMATC interrupt routing and callback dispatch.")
    lines.append("- Validate circular DMA/streaming access and buffer ownership constraints.")
    lines.append("- Exit criteria: notification and streaming checklist items PASS.")
    lines.append("")
    lines.append("### Phase 5 - Error Handling & DET/Runtime (2-3 days)")
    lines.append("- Map all ADC_E_* development/runtime errors to service calls.")
    lines.append("- Enforce skip-on-error behavior and no side-effects on invalid calls.")
    lines.append("- Exit criteria: negative tests PASS with clear error evidence.")
    lines.append("")
    lines.append("### Phase 6 - Power State (Optional) (2-4 days)")
    lines.append("- Implement prepare/set/get current/get target + async transition manager.")
    lines.append("- Gate by `AdcLowPowerStatesSupport` and async mode switch.")
    lines.append("- Exit criteria: power transition scenarios PASS.")
    lines.append("")
    lines.append("### Phase 7 - Compliance Closure (3-5 days)")
    lines.append("- Execute full checklist and freeze traceability with evidence links.")
    lines.append("- Collect Renode logs + board logs as objective verification artifacts.")
    lines.append("- Exit criteria: mandatory requirements closed or justified N/A.")
    lines.append("")
    lines.append("## Deliverables")
    lines.append("")
    lines.append("- Production files: `Adc.h`, `Adc.c`, `Adc_Irq.c`, `Adc_Dma.c`, `Adc_Types.h`, `Adc_Cfg.h`, `Adc_Cfg.c`.")
    lines.append("- Validation files: `ADC_TRACEABILITY_MATRIX.md`, `ADC_TEST_CHECKLIST.md`, test logs.")
    lines.append("- Optional: Renode+GDB automated test runner for ADC APIs.")

    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Generate ADC planning artifacts from extracted JSON")
    p.add_argument("--input", type=Path, default=Path("Adc/tools/adc_sws_r25_11_extracted.json"), help="Input extracted JSON")
    p.add_argument("--header", type=Path, default=Path("Adc/Adc.h"), help="Path to Adc header")
    p.add_argument("--source-glob", default="*.c", help="Glob for source files under Adc directory")
    p.add_argument("--out-dir", type=Path, default=Path("Adc/docs"), help="Output documentation directory")
    return p.parse_args()


def main() -> None:
    args = parse_args()
    if not args.input.exists():
        raise SystemExit(f"Input JSON not found: {args.input}")
    if not args.header.exists():
        raise SystemExit(f"Header not found: {args.header}")

    ctx = load_context(args.input, args.header, args.source_glob)
    args.out_dir.mkdir(parents=True, exist_ok=True)

    trace_md = args.out_dir / "ADC_TRACEABILITY_MATRIX.md"
    trace_csv = args.out_dir / "ADC_TRACEABILITY_MATRIX.csv"
    checklist_md = args.out_dir / "ADC_TEST_CHECKLIST.md"
    plan_md = args.out_dir / "ADC_IMPLEMENTATION_PLAN_STM32F103_SPL.md"

    write_traceability_md(ctx, trace_md)
    write_traceability_csv(ctx, trace_csv)
    write_test_checklist_md(ctx, checklist_md)
    write_implementation_plan_md(ctx, plan_md)

    print(f"Generated: {trace_md}")
    print(f"Generated: {trace_csv}")
    print(f"Generated: {checklist_md}")
    print(f"Generated: {plan_md}")


if __name__ == "__main__":
    main()
