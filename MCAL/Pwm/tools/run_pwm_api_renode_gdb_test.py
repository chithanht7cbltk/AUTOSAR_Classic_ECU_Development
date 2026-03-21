#!/usr/bin/env python3
"""Runner test PWM API bằng Renode + GDB.

Mục tiêu:
1) Build firmware test harness sử dụng SPL trong platform.
2) Khởi chạy Renode với file .repl STM32F103.
3) Dùng GDB chạy toàn bộ kịch bản API PWM.
4) Xuất bộ báo cáo cho người đọc và CI:
   - report markdown/json tổng hợp
   - summary.json + details.json
   - traceability requirement (json/csv/md)
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import re
import shlex
import signal
import socket
import subprocess
import time
from dataclasses import dataclass, asdict
from datetime import datetime
from pathlib import Path
from typing import Any


@dataclass
class Check:
    label: str
    expr: str
    expected: int
    comparator: str = "eq"  # eq | ne | nonzero


@dataclass
class Scenario:
    name: str
    description: str
    apis: list[str]
    commands: list[str]
    checks: list[Check]
    requirement_ids: list[str] | None = None


@dataclass
class CheckResult:
    label: str
    expr: str
    expected: int
    actual: int | None
    comparator: str
    passed: bool


@dataclass
class ScenarioResult:
    name: str
    description: str
    apis: list[str]
    requirement_ids: list[str]
    passed: bool
    checks: list[CheckResult]
    gdb_exit_code: int
    gdb_stdout: str
    gdb_stderr: str


def run_command(cmd: list[str], cwd: Path | None = None) -> subprocess.CompletedProcess:
    return subprocess.run(
        cmd,
        cwd=cwd,
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
    )


def find_binary(preferred: str | None, fallback_names: list[str]) -> str:
    if preferred:
        p = Path(preferred)
        if p.exists():
            return str(p)

    for name in fallback_names:
        rc = run_command(["/bin/zsh", "-lc", f"which {shlex.quote(name)}"])
        if rc.returncode == 0:
            candidate = rc.stdout.strip().splitlines()[-1].strip()
            if candidate:
                return candidate

    raise FileNotFoundError(f"Không tìm thấy binary: {fallback_names}")


def wait_for_tcp_port(host: str, port: int, timeout_s: float) -> bool:
    start = time.time()
    while (time.time() - start) < timeout_s:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(0.5)
            try:
                sock.connect((host, port))
                return True
            except OSError:
                time.sleep(0.2)
    return False


def build_firmware(root: Path, gcc: str, build_dir: Path) -> tuple[bool, str, str, Path]:
    build_dir.mkdir(parents=True, exist_ok=True)

    elf = build_dir / "pwm_api_test.elf"
    objs_dir = build_dir / "obj"
    objs_dir.mkdir(parents=True, exist_ok=True)

    cflags = [
        "-mcpu=cortex-m3",
        "-mthumb",
        "-O0",
        "-g3",
        "-Wall",
        "-Wextra",
        "-DSTM32F10X_MD",
        "-DUSE_STDPERIPH_DRIVER",
        "-I.",
        "-Iplatform/include",
        "-Iplatform/spl/inc",
        "-Iplatform/bsp/cmsis",
        "-IPwm",
    ]

    sources = [
        root / "Pwm/tests/renode/pwm_api_test_main.c",
        root / "Pwm/Pwm.c",
        root / "Pwm/Pwm_Lcfg.c",
        root / "platform/spl/src/system_stm32f10x.c",
        root / "platform/spl/src/stm32f10x_rcc.c",
        root / "platform/spl/src/stm32f10x_gpio.c",
        root / "platform/spl/src/stm32f10x_tim.c",
        root / "platform/bsp/startup_stm32f10x_md.s",
    ]

    stdout_all: list[str] = []
    stderr_all: list[str] = []
    object_files: list[Path] = []

    for src in sources:
        obj = objs_dir / (src.stem + ".o")
        object_files.append(obj)
        cmd = [gcc, *cflags, "-c", str(src), "-o", str(obj)]
        rc = run_command(cmd, cwd=root)
        stdout_all.append(rc.stdout)
        stderr_all.append(rc.stderr)
        if rc.returncode != 0:
            return False, "".join(stdout_all), "".join(stderr_all), elf

    ldflags = [
        "-T",
        str(root / "platform/bsp/linker/stm32f103.ld"),
        "-nostartfiles",
        "-nostdlib",
        "-Wl,-Map=" + str(build_dir / "pwm_api_test.map"),
    ]

    link_cmd = [gcc, *cflags, *(str(x) for x in object_files), *ldflags, "-o", str(elf)]
    rc_link = run_command(link_cmd, cwd=root)
    stdout_all.append(rc_link.stdout)
    stderr_all.append(rc_link.stderr)

    return rc_link.returncode == 0, "".join(stdout_all), "".join(stderr_all), elf


def write_resc(resc_path: Path, repl: Path, elf: Path, gdb_port: int) -> None:
    content = f"""
mach create "pwm_api_test"
machine LoadPlatformDescription @{repl}
sysbus LoadELF @{elf}
machine StartGdbServer {gdb_port}
start
""".strip()
    resc_path.write_text(content + "\n", encoding="utf-8")


def parse_checks_from_gdb_stdout(stdout: str) -> dict[str, int]:
    out: dict[str, int] = {}
    for line in stdout.splitlines():
        m = re.match(r"__CHK__([A-Za-z0-9_]+)=(-?\d+)", line.strip())
        if m:
            out[m.group(1)] = int(m.group(2))
    return out


def evaluate_check(actual: int | None, expected: int, comparator: str) -> bool:
    if actual is None:
        return False
    if comparator == "eq":
        return actual == expected
    if comparator == "ne":
        return actual != expected
    if comparator == "nonzero":
        return actual != 0
    return False


def load_api_requirement_map(sws_json_path: Path) -> dict[str, list[str]]:
    """Đọc mapping API -> requirement IDs từ file JSON đã extract SWS."""
    if not sws_json_path.exists():
        return {}

    try:
        data = json.loads(sws_json_path.read_text(encoding="utf-8"))
    except Exception:
        return {}

    out: dict[str, list[str]] = {}
    apis = data.get("apis", [])
    if not isinstance(apis, list):
        return out

    for api in apis:
        if not isinstance(api, dict):
            continue
        api_name = api.get("name")
        req_ids = api.get("requirement_ids", [])
        if not isinstance(api_name, str):
            continue
        if not isinstance(req_ids, list):
            req_ids = []
        req_ids_clean = [x for x in req_ids if isinstance(x, str)]
        out[api_name] = sorted(set(req_ids_clean))
    return out


def enrich_scenarios_requirements(
    scenarios: list[Scenario],
    api_requirement_map: dict[str, list[str]],
) -> list[Scenario]:
    """Bổ sung requirement IDs cho từng scenario dựa trên danh sách API scenario đó dùng."""
    for sc in scenarios:
        req_set = set(sc.requirement_ids or [])
        for api in sc.apis:
            req_set.update(api_requirement_map.get(api, []))
        sc.requirement_ids = sorted(req_set)
    return scenarios


def build_traceability_data(
    scenario_results: list[ScenarioResult],
    api_requirement_map: dict[str, list[str]],
) -> dict[str, Any]:
    """Tạo dữ liệu traceability Requirement -> API -> Scenario -> Result."""
    all_requirements = sorted({req for reqs in api_requirement_map.values() for req in reqs})

    req_summary: dict[str, dict[str, Any]] = {}
    scenario_requirement_rows: list[dict[str, Any]] = []

    for sc in scenario_results:
        req_ids = sc.requirement_ids or []
        for req_id in req_ids:
            apis_for_req = [api for api in sc.apis if req_id in api_requirement_map.get(api, [])]
            if not apis_for_req:
                apis_for_req = list(sc.apis)

            row = {
                "requirement_id": req_id,
                "apis": apis_for_req,
                "scenario": sc.name,
                "scenario_passed": sc.passed,
            }
            scenario_requirement_rows.append(row)

            if req_id not in req_summary:
                req_summary[req_id] = {
                    "requirement_id": req_id,
                    "apis": set(),
                    "covered_by_scenarios": [],
                    "passed_scenarios": 0,
                    "failed_scenarios": 0,
                }
            req_summary[req_id]["apis"].update(apis_for_req)
            req_summary[req_id]["covered_by_scenarios"].append(sc.name)
            if sc.passed:
                req_summary[req_id]["passed_scenarios"] += 1
            else:
                req_summary[req_id]["failed_scenarios"] += 1

    requirements: list[dict[str, Any]] = []
    for req_id in all_requirements:
        info = req_summary.get(req_id)
        if info is None:
            requirements.append(
                {
                    "requirement_id": req_id,
                    "apis": [],
                    "covered_by_scenarios": [],
                    "passed_scenarios": 0,
                    "failed_scenarios": 0,
                    "status": "NOT_COVERED",
                }
            )
            continue

        failed_count = int(info["failed_scenarios"])
        status = "PASS" if failed_count == 0 else "FAIL"
        requirements.append(
            {
                "requirement_id": req_id,
                "apis": sorted(info["apis"]),
                "covered_by_scenarios": sorted(set(info["covered_by_scenarios"])),
                "passed_scenarios": int(info["passed_scenarios"]),
                "failed_scenarios": failed_count,
                "status": status,
            }
        )

    passed_req = sum(1 for x in requirements if x["status"] == "PASS")
    failed_req = sum(1 for x in requirements if x["status"] == "FAIL")
    not_covered_req = sum(1 for x in requirements if x["status"] == "NOT_COVERED")

    return {
        "summary": {
            "total_requirements": len(requirements),
            "passed_requirements": passed_req,
            "failed_requirements": failed_req,
            "not_covered_requirements": not_covered_req,
        },
        "requirements": requirements,
        "scenario_requirement_rows": scenario_requirement_rows,
    }


def run_gdb_scenario(gdb: str, elf: Path, port: int, scenario: Scenario) -> ScenarioResult:
    gdb_cmds: list[str] = [
        "set pagination off",
        "set confirm off",
        f"target remote :{port}",
        "monitor reset halt",
    ]

    gdb_cmds.extend(scenario.commands)

    for check in scenario.checks:
        gdb_cmds.append(f'printf "__CHK__{check.label}=%u\\n", (unsigned int)({check.expr})')

    gdb_cmds.extend([
        "detach",
        "quit",
    ])

    cmd = [gdb, "-q", "--batch", str(elf)]
    for c in gdb_cmds:
        cmd.extend(["-ex", c])

    rc = run_command(cmd)
    parsed = parse_checks_from_gdb_stdout(rc.stdout)

    check_results: list[CheckResult] = []
    for check in scenario.checks:
        actual = parsed.get(check.label)
        passed = evaluate_check(actual, check.expected, check.comparator)
        check_results.append(
            CheckResult(
                label=check.label,
                expr=check.expr,
                expected=check.expected,
                actual=actual,
                comparator=check.comparator,
                passed=passed,
            )
        )

    scenario_passed = (rc.returncode == 0) and all(c.passed for c in check_results)

    return ScenarioResult(
        name=scenario.name,
        description=scenario.description,
        apis=scenario.apis,
        requirement_ids=list(scenario.requirement_ids or []),
        passed=scenario_passed,
        checks=check_results,
        gdb_exit_code=rc.returncode,
        gdb_stdout=rc.stdout,
        gdb_stderr=rc.stderr,
    )


def scenario_suite() -> list[Scenario]:
    tim2_cr1 = "(*(unsigned int*)0x40000000)"
    tim2_dier = "(*(unsigned int*)0x4000000C)"
    tim2_arr = "(*(unsigned int*)0x4000002C)"
    tim2_ccr1 = "(*(unsigned int*)0x40000034)"

    tim3_cr1 = "(*(unsigned int*)0x40000400)"
    tim3_arr = "(*(unsigned int*)0x4000042C)"
    tim3_ccr2 = "(*(unsigned int*)0x40000438)"

    return [
        Scenario(
            name="init_default_values",
            description="Pwm_Init phải set ARR/CCR theo cấu hình mặc định cho 2 kênh.",
            apis=["Pwm_Init"],
            commands=["call Pwm_Init(&PwmDriverConfig)"],
            checks=[
                Check("tim2_arr", tim2_arr, 999),
                Check("tim2_ccr1", tim2_ccr1, 0),
                Check("tim3_arr", tim3_arr, 999),
                Check("tim3_ccr2", tim3_ccr2, 0),
            ],
        ),
        Scenario(
            name="set_duty_ch0_half",
            description="Pwm_SetDutyCycle channel 0 với 50%.",
            apis=["Pwm_Init", "Pwm_SetDutyCycle"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "call Pwm_SetDutyCycle(0, 0x4000)",
            ],
            checks=[
                Check("tim2_ccr1", tim2_ccr1, 499),
            ],
        ),
        Scenario(
            name="set_duty_ch1_full",
            description="Pwm_SetDutyCycle channel 1 với 100%.",
            apis=["Pwm_Init", "Pwm_SetDutyCycle"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "call Pwm_SetDutyCycle(1, 0x8000)",
            ],
            checks=[
                Check("tim3_ccr2", tim3_ccr2, 999),
            ],
        ),
        Scenario(
            name="set_period_and_duty",
            description="Pwm_SetPeriodAndDuty phải cập nhật ARR/CCR kênh variable period.",
            apis=["Pwm_Init", "Pwm_SetPeriodAndDuty"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "call Pwm_SetPeriodAndDuty(0, 2000, 0x4000)",
            ],
            checks=[
                Check("tim2_arr", tim2_arr, 2000),
                Check("tim2_ccr1", tim2_ccr1, 1000),
            ],
        ),
        Scenario(
            name="set_output_to_idle",
            description="Pwm_SetOutputToIdle phải đưa duty về 0.",
            apis=["Pwm_Init", "Pwm_SetDutyCycle", "Pwm_SetOutputToIdle"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "call Pwm_SetDutyCycle(0, 0x2000)",
                "call Pwm_SetOutputToIdle(0)",
            ],
            checks=[
                Check("tim2_ccr1", tim2_ccr1, 0),
            ],
        ),
        Scenario(
            name="get_output_state_high_low",
            description="Pwm_GetOutputState trả HIGH/LOW theo CCER bit enable.",
            apis=["Pwm_Init", "Pwm_GetOutputState"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "set {unsigned int}0x40000020 = 1",
                "set $s1 = (unsigned int)Pwm_GetOutputState(0)",
                "set {unsigned int}0x40000020 = 0",
                "set $s2 = (unsigned int)Pwm_GetOutputState(0)",
            ],
            checks=[
                Check("state_high", "$s1", 0),
                Check("state_low", "$s2", 1),
            ],
        ),
        Scenario(
            name="enable_notification",
            description="Pwm_EnableNotification bật bit DIER CC1IE.",
            apis=["Pwm_Init", "Pwm_EnableNotification"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "call Pwm_EnableNotification(0, PWM_BOTH_EDGES)",
            ],
            checks=[
                Check("dier_cc1", f"(({tim2_dier} & 0x0002u) != 0)", 1),
            ],
        ),
        Scenario(
            name="disable_notification",
            description="Pwm_DisableNotification xóa bit DIER CC1IE.",
            apis=["Pwm_Init", "Pwm_EnableNotification", "Pwm_DisableNotification"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "call Pwm_EnableNotification(0, PWM_BOTH_EDGES)",
                "call Pwm_DisableNotification(0)",
            ],
            checks=[
                Check("dier_cc1", f"(({tim2_dier} & 0x0002u) != 0)", 0),
            ],
        ),
        Scenario(
            name="deinit_disables_counter",
            description="Pwm_DeInit phải tắt counter timer.",
            apis=["Pwm_Init", "Pwm_DeInit"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "call Pwm_DeInit()",
            ],
            checks=[
                Check("tim2_cen", f"(({tim2_cr1} & 0x0001u) != 0)", 0),
                Check("tim3_cen", f"(({tim3_cr1} & 0x0001u) != 0)", 0),
            ],
        ),
        Scenario(
            name="version_info",
            description="Pwm_GetVersionInfo trả đúng phiên bản cấu hình cứng trong driver.",
            apis=["Pwm_GetVersionInfo"],
            commands=[
                "call Pwm_GetVersionInfo((Std_VersionInfoType*)&g_version_info)",
            ],
            checks=[
                Check("vendor", "g_version_info.vendorID", 0x1234),
                Check("module", "g_version_info.moduleID", 0xABCD),
                Check("sw_major", "g_version_info.sw_major_version", 1),
            ],
        ),
        Scenario(
            name="invalid_channel_set_duty_no_change",
            description="SetDuty với channel invalid không làm thay đổi CCR hiện tại.",
            apis=["Pwm_Init", "Pwm_SetDutyCycle"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "call Pwm_SetDutyCycle(0, 0x2000)",
                f"set $before = (unsigned int){tim2_ccr1}",
                "call Pwm_SetDutyCycle(99, 0x8000)",
                f"set $after = (unsigned int){tim2_ccr1}",
            ],
            checks=[
                Check("before", "$before", 249),
                Check("after", "$after", 249),
            ],
        ),
        Scenario(
            name="invalid_channel_get_output_state",
            description="GetOutputState với channel invalid trả PWM_LOW.",
            apis=["Pwm_Init", "Pwm_GetOutputState"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "set $rv = (unsigned int)Pwm_GetOutputState(99)",
            ],
            checks=[
                Check("rv", "$rv", 1),
            ],
        ),
        Scenario(
            name="power_set_without_prepare",
            description="SetPowerState trước PreparePowerState phải lỗi sequence.",
            apis=["Pwm_Init", "Pwm_SetPowerState"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "set $ret = (unsigned int)Pwm_SetPowerState((Pwm_PowerStateRequestResultType*)&g_power_result)",
            ],
            checks=[
                Check("ret", "$ret", 1),
                Check("result", "g_power_result", 2),
            ],
        ),
        Scenario(
            name="power_prepare_set_ok",
            description="Prepare + Set power state thành công.",
            apis=["Pwm_Init", "Pwm_PreparePowerState", "Pwm_SetPowerState", "Pwm_GetCurrentPowerState"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "set $r0 = (unsigned int)Pwm_PreparePowerState((Pwm_PowerStateType)1, (Pwm_PowerStateRequestResultType*)&g_power_result)",
                "set $r1 = (unsigned int)Pwm_SetPowerState((Pwm_PowerStateRequestResultType*)&g_power_result)",
                "set $r2 = (unsigned int)Pwm_GetCurrentPowerState((Pwm_PowerStateType*)&g_power_state, (Pwm_PowerStateRequestResultType*)&g_power_result)",
            ],
            checks=[
                Check("r0", "$r0", 0),
                Check("r1", "$r1", 0),
                Check("r2", "$r2", 0),
                Check("state", "g_power_state", 1),
                Check("result", "g_power_result", 0),
            ],
        ),
        Scenario(
            name="power_get_target",
            description="GetTargetPowerState trả đúng mode đã prepare.",
            apis=["Pwm_Init", "Pwm_PreparePowerState", "Pwm_GetTargetPowerState"],
            commands=[
                "call Pwm_Init(&PwmDriverConfig)",
                "set $r0 = (unsigned int)Pwm_PreparePowerState((Pwm_PowerStateType)2, (Pwm_PowerStateRequestResultType*)&g_power_result)",
                "set $r1 = (unsigned int)Pwm_GetTargetPowerState((Pwm_PowerStateType*)&g_power_state, (Pwm_PowerStateRequestResultType*)&g_power_result)",
            ],
            checks=[
                Check("r0", "$r0", 0),
                Check("r1", "$r1", 0),
                Check("state", "g_power_state", 2),
            ],
        ),
        Scenario(
            name="power_prepare_uninit",
            description="PreparePowerState khi chưa init phải trả PWM_NOT_INIT.",
            apis=["Pwm_DeInit", "Pwm_PreparePowerState"],
            commands=[
                "call Pwm_DeInit()",
                "set $ret = (unsigned int)Pwm_PreparePowerState((Pwm_PowerStateType)1, (Pwm_PowerStateRequestResultType*)&g_power_result)",
            ],
            checks=[
                Check("ret", "$ret", 1),
                Check("result", "g_power_result", 1),
            ],
        ),
        Scenario(
            name="power_get_current_uninit",
            description="GetCurrentPowerState khi chưa init phải trả PWM_NOT_INIT.",
            apis=["Pwm_DeInit", "Pwm_GetCurrentPowerState"],
            commands=[
                "call Pwm_DeInit()",
                "set $ret = (unsigned int)Pwm_GetCurrentPowerState((Pwm_PowerStateType*)&g_power_state, (Pwm_PowerStateRequestResultType*)&g_power_result)",
            ],
            checks=[
                Check("ret", "$ret", 1),
                Check("result", "g_power_result", 1),
            ],
        ),
        Scenario(
            name="power_get_target_uninit",
            description="GetTargetPowerState khi chưa init phải trả PWM_NOT_INIT.",
            apis=["Pwm_DeInit", "Pwm_GetTargetPowerState"],
            commands=[
                "call Pwm_DeInit()",
                "set $ret = (unsigned int)Pwm_GetTargetPowerState((Pwm_PowerStateType*)&g_power_state, (Pwm_PowerStateRequestResultType*)&g_power_result)",
            ],
            checks=[
                Check("ret", "$ret", 1),
                Check("result", "g_power_result", 1),
            ],
        ),
        Scenario(
            name="main_power_transition_uninit_safe",
            description="Main_PowerTransitionManager khi chưa init phải return an toàn.",
            apis=["Pwm_Main_PowerTransitionManager"],
            commands=[
                "call Pwm_Main_PowerTransitionManager()",
                "set $ok = 1",
            ],
            checks=[
                Check("ok", "$ok", 1),
            ],
        ),
    ]


def start_renode(
    renode_path: str,
    resc_path: Path,
    log_path: Path,
    monitor_port: int,
) -> subprocess.Popen:
    log_file = open(log_path, "w", encoding="utf-8")
    # Dùng monitor TCP để Renode không thoát khi stdin console bị đóng.
    cmd = [
        renode_path,
        "--plain",
        "--disable-gui",
        "--port",
        str(monitor_port),
        "-e",
        f"i @{resc_path}",
    ]
    proc = subprocess.Popen(
        cmd,
        stdout=log_file,
        stderr=subprocess.STDOUT,
        preexec_fn=os.setsid,
    )
    return proc


def stop_renode(proc: subprocess.Popen | None) -> None:
    if proc is None:
        return
    try:
        os.killpg(os.getpgid(proc.pid), signal.SIGTERM)
    except Exception:
        pass


def write_traceability_reports(
    report_dir: Path,
    ts: str,
    traceability_data: dict[str, Any],
) -> tuple[Path, Path, Path]:
    """Ghi bộ artifact traceability: JSON + CSV + Markdown."""
    trace_json_path = report_dir / f"pwm_api_traceability_{ts}.json"
    trace_csv_path = report_dir / f"pwm_api_traceability_{ts}.csv"
    trace_md_path = report_dir / f"pwm_api_traceability_{ts}.md"

    trace_json_path.write_text(
        json.dumps(traceability_data, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )

    with trace_csv_path.open("w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["requirement_id", "apis", "scenario", "scenario_result"])
        for row in traceability_data.get("scenario_requirement_rows", []):
            writer.writerow(
                [
                    row.get("requirement_id", ""),
                    ", ".join(row.get("apis", [])),
                    row.get("scenario", ""),
                    "PASS" if row.get("scenario_passed", False) else "FAIL",
                ]
            )

    md_lines: list[str] = []
    md_lines.append("# PWM Traceability Report")
    md_lines.append("")
    sm = traceability_data.get("summary", {})
    md_lines.append(f"- Total requirements: `{sm.get('total_requirements', 0)}`")
    md_lines.append(f"- Passed requirements: `{sm.get('passed_requirements', 0)}`")
    md_lines.append(f"- Failed requirements: `{sm.get('failed_requirements', 0)}`")
    md_lines.append(f"- Not covered requirements: `{sm.get('not_covered_requirements', 0)}`")
    md_lines.append("")
    md_lines.append("## Requirement Summary")
    md_lines.append("")
    md_lines.append("| Requirement | APIs | Status | Covered Scenarios |")
    md_lines.append("|---|---|---|---|")

    for req in traceability_data.get("requirements", []):
        md_lines.append(
            f"| {req.get('requirement_id', '')} | {', '.join(req.get('apis', []))} | {req.get('status', '')} | {len(req.get('covered_by_scenarios', []))} |"
        )

    trace_md_path.write_text("\n".join(md_lines) + "\n", encoding="utf-8")
    return trace_json_path, trace_csv_path, trace_md_path


def write_reports(
    report_dir: Path,
    details_data: dict[str, Any],
    traceability_data: dict[str, Any],
    exit_code: int,
) -> dict[str, Path]:
    """Ghi toàn bộ report: legacy report + summary/details cho CI + traceability."""
    report_dir.mkdir(parents=True, exist_ok=True)
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")

    legacy_json_path = report_dir / f"pwm_api_renode_gdb_report_{ts}.json"
    legacy_md_path = report_dir / f"pwm_api_renode_gdb_report_{ts}.md"
    details_json_path = report_dir / f"pwm_api_renode_gdb_details_{ts}.json"
    summary_json_path = report_dir / f"pwm_api_renode_gdb_summary_{ts}.json"

    failed_scenarios = [x["name"] for x in details_data.get("scenarios", []) if not x.get("passed", False)]
    summary_data = {
        "generated_at": details_data.get("generated_at"),
        "status": "PASS" if (details_data.get("summary", {}).get("failed", 1) == 0 and details_data.get("build", {}).get("success", False)) else "FAIL",
        "exit_code": exit_code,
        "scenario_summary": details_data.get("summary", {}),
        "traceability_summary": traceability_data.get("summary", {}),
        "failed_scenarios": failed_scenarios,
        "build_success": details_data.get("build", {}).get("success", False),
    }

    details_json_path.write_text(json.dumps(details_data, indent=2, ensure_ascii=False), encoding="utf-8")
    summary_json_path.write_text(json.dumps(summary_data, indent=2, ensure_ascii=False), encoding="utf-8")

    # Giữ report JSON cũ để tương thích với luồng đã dùng trước đây.
    legacy_json_path.write_text(json.dumps(details_data, indent=2, ensure_ascii=False), encoding="utf-8")

    lines: list[str] = []
    lines.append("# PWM API Test Report (Renode + GDB)")
    lines.append("")
    lines.append(f"- Generated at: `{details_data['generated_at']}`")
    lines.append(f"- ELF: `{details_data['build']['elf']}`")
    lines.append(f"- Repl: `{details_data['environment']['repl']}`")
    lines.append(f"- Renode: `{details_data['environment']['renode']}`")
    lines.append(f"- GDB: `{details_data['environment']['gdb']}`")
    lines.append(f"- Traceability source: `{details_data.get('requirements_source', '')}`")
    lines.append("")
    lines.append(
        f"## Summary: {details_data['summary']['passed']}/{details_data['summary']['total']} scenarios passed"
    )
    lines.append(
        f"- Requirement summary: {traceability_data['summary']['passed_requirements']}/{traceability_data['summary']['total_requirements']} PASS"
    )
    lines.append("")
    lines.append("| Scenario | APIs | Req IDs | Result |")
    lines.append("|---|---|---|---|")

    for sc in details_data["scenarios"]:
        result = "PASS" if sc["passed"] else "FAIL"
        req_count = len(sc.get("requirement_ids", []))
        lines.append(f"| {sc['name']} | {', '.join(sc['apis'])} | {req_count} | {result} |")

    failed = [x for x in details_data["scenarios"] if not x["passed"]]
    if failed:
        lines.append("")
        lines.append("## Failed Details")
        lines.append("")
        for sc in failed:
            lines.append(f"### {sc['name']}")
            lines.append(f"- Description: {sc['description']}")
            lines.append(f"- Requirement IDs: {', '.join(sc.get('requirement_ids', []))}")
            lines.append(f"- GDB exit code: {sc['gdb_exit_code']}")
            for chk in sc["checks"]:
                lines.append(
                    f"- Check `{chk['label']}`: actual={chk['actual']} expected={chk['expected']} comparator={chk['comparator']} -> {'PASS' if chk['passed'] else 'FAIL'}"
                )
            lines.append("")

    legacy_md_path.write_text("\n".join(lines) + "\n", encoding="utf-8")

    trace_json_path, trace_csv_path, trace_md_path = write_traceability_reports(
        report_dir=report_dir,
        ts=ts,
        traceability_data=traceability_data,
    )

    return {
        "legacy_json": legacy_json_path,
        "legacy_md": legacy_md_path,
        "details_json": details_json_path,
        "summary_json": summary_json_path,
        "traceability_json": trace_json_path,
        "traceability_csv": trace_csv_path,
        "traceability_md": trace_md_path,
    }


def parse_args() -> argparse.Namespace:
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description="Run PWM API scenarios via Renode + GDB")
    parser.add_argument("--root", type=Path, default=root, help="Project root")
    parser.add_argument("--repl", type=Path, default=root / "stm32f103_full.repl", help="Path to .repl file")
    parser.add_argument(
        "--sws-json",
        type=Path,
        default=root / "Pwm/tools/pwm_sws_r25_11_extracted.json",
        help="Path file JSON đã trích xuất từ AUTOSAR PWM SWS",
    )
    parser.add_argument("--gdb-port", type=int, default=3333, help="GDB server port")
    parser.add_argument("--monitor-port", type=int, default=12345, help="Renode monitor TCP port")
    parser.add_argument("--renode", default=os.environ.get("RENODE_PATH"), help="Path Renode executable")
    parser.add_argument("--gdb", default=os.environ.get("GDB_PATH"), help="Path arm-none-eabi-gdb")
    parser.add_argument("--gcc", default=os.environ.get("GCC_PATH"), help="Path arm-none-eabi-gcc")
    parser.add_argument("--skip-build", action="store_true", help="Skip build step")
    parser.add_argument("--report-dir", type=Path, default=root / "Pwm/test_reports", help="Directory output reports")
    parser.add_argument("--port-timeout", type=float, default=25.0, help="Timeout wait GDB port")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    root = args.root.resolve()
    sws_json = args.sws_json.resolve()

    repl = args.repl.resolve()
    if not repl.exists():
        fallback = root / "platform/debug/stm32f103_full.repl"
        if fallback.exists():
            repl = fallback
        else:
            raise SystemExit(f"Không tìm thấy file .repl: {args.repl}")

    renode = find_binary(args.renode, ["renode", "/Applications/Renode.app/Contents/MacOS/renode"])
    gdb = find_binary(args.gdb, ["arm-none-eabi-gdb"])
    gcc = find_binary(args.gcc, ["arm-none-eabi-gcc"])

    api_requirement_map = load_api_requirement_map(sws_json)
    scenarios = enrich_scenarios_requirements(scenario_suite(), api_requirement_map)

    build_dir = root / "Pwm/tests/renode/build"
    build_ok = True
    build_stdout = ""
    build_stderr = ""
    elf = build_dir / "pwm_api_test.elf"
    monitor_port = args.monitor_port
    if monitor_port == args.gdb_port:
        monitor_port = args.gdb_port + 1000

    if not args.skip_build:
        build_ok, build_stdout, build_stderr, elf = build_firmware(root, gcc, build_dir)
        if not build_ok:
            details_data = {
                "generated_at": datetime.now().isoformat(),
                "requirements_source": str(sws_json),
                "api_requirement_map": api_requirement_map,
                "environment": {
                    "repl": str(repl),
                    "renode": renode,
                    "gdb": gdb,
                    "gcc": gcc,
                    "gdb_port": args.gdb_port,
                    "monitor_port": monitor_port,
                    "renode_log": str(build_dir / "renode.log"),
                },
                "build": {
                    "success": False,
                    "elf": str(elf),
                    "stdout": build_stdout,
                    "stderr": build_stderr,
                },
                "summary": {
                    "total": len(scenarios),
                    "executed": 0,
                    "passed": 0,
                    "failed": 0,
                    "not_executed": len(scenarios),
                },
                "scenarios": [],
            }

            traceability_data = build_traceability_data([], api_requirement_map)
            report_paths = write_reports(
                report_dir=args.report_dir,
                details_data=details_data,
                traceability_data=traceability_data,
                exit_code=2,
            )
            print("Build failed. Artifacts:")
            for key, path in report_paths.items():
                print(f"- {key}: {path}")
            return 2

    if not elf.exists():
        raise SystemExit(f"Không tìm thấy ELF: {elf}")

    resc_path = build_dir / "pwm_api_test.resc"
    renode_log = build_dir / "renode.log"
    write_resc(resc_path, repl, elf, args.gdb_port)

    renode_proc: subprocess.Popen | None = None
    try:
        renode_proc = start_renode(renode, resc_path, renode_log, monitor_port)
        if not wait_for_tcp_port("127.0.0.1", args.gdb_port, args.port_timeout):
            raise RuntimeError(
                f"Renode chưa mở GDB server ở cổng {args.gdb_port}. Kiểm tra log: {renode_log}"
            )

        results: list[ScenarioResult] = []
        for sc in scenarios:
            results.append(run_gdb_scenario(gdb, elf, args.gdb_port, sc))

        passed = sum(1 for x in results if x.passed)
        total = len(results)
        failed = total - passed

        details_data = {
            "generated_at": datetime.now().isoformat(),
            "requirements_source": str(sws_json),
            "api_requirement_map": api_requirement_map,
            "environment": {
                "repl": str(repl),
                "renode": renode,
                "gdb": gdb,
                "gcc": gcc,
                "gdb_port": args.gdb_port,
                "monitor_port": monitor_port,
                "renode_log": str(renode_log),
            },
            "build": {
                "success": True,
                "elf": str(elf),
                "stdout": build_stdout,
                "stderr": build_stderr,
            },
            "summary": {
                "total": total,
                "executed": total,
                "passed": passed,
                "failed": failed,
                "not_executed": 0,
            },
            "scenarios": [asdict(r) for r in results],
        }

        traceability_data = build_traceability_data(results, api_requirement_map)
        exit_code = 1 if failed > 0 else 0

        report_paths = write_reports(
            report_dir=args.report_dir,
            details_data=details_data,
            traceability_data=traceability_data,
            exit_code=exit_code,
        )
        print("Artifacts:")
        for key, path in report_paths.items():
            print(f"- {key}: {path}")

        if failed > 0:
            return 1
        return 0

    finally:
        stop_renode(renode_proc)


if __name__ == "__main__":
    raise SystemExit(main())
