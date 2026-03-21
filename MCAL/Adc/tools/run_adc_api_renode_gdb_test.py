#!/usr/bin/env python3
"""Runner test ADC API bằng Renode + GDB.

Mục tiêu:
1) Build firmware test harness sử dụng SPL trong platform.
2) Khởi chạy Renode với file .repl STM32F103.
3) Dùng GDB chạy toàn bộ kịch bản API ADC.
4) Xuất báo cáo markdown/json và traceability.
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
from dataclasses import asdict, dataclass
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

    elf = build_dir / "adc_api_test.elf"
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
        "-IAdc",
    ]

    sources = [
        root / "Adc/tests/renode/adc_api_test_main.c",
        root / "Adc/Adc.c",
        root / "Adc/Adc_Cfg.c",
        root / "platform/spl/src/system_stm32f10x.c",
        root / "platform/spl/src/stm32f10x_rcc.c",
        root / "platform/spl/src/stm32f10x_adc.c",
        root / "platform/spl/src/stm32f10x_dma.c",
        root / "platform/spl/src/misc.c",
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
        "-Wl,-Map=" + str(build_dir / "adc_api_test.map"),
    ]

    link_cmd = [gcc, *cflags, *(str(x) for x in object_files), *ldflags, "-o", str(elf)]
    rc_link = run_command(link_cmd, cwd=root)
    stdout_all.append(rc_link.stdout)
    stderr_all.append(rc_link.stderr)

    return rc_link.returncode == 0, "".join(stdout_all), "".join(stderr_all), elf


def write_resc(resc_path: Path, repl: Path, elf: Path, gdb_port: int) -> None:
    content = f"""
mach create "adc_api_test"
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
    adc1_sr = "(*(unsigned int*)0x40012400)"
    adc1_cr1 = "(*(unsigned int*)0x40012404)"
    adc1_cr2 = "(*(unsigned int*)0x40012408)"
    adc1_dr = "(*(unsigned int*)0x4001244C)"

    dma1_isr = "(*(unsigned int*)0x40020000)"
    dma1_cmar1 = "(*(unsigned int*)0x40020014)"
    dma1_cndtr1 = "(*(unsigned int*)0x4002000C)"

    rcc_apb2enr = "(*(unsigned int*)0x40021018)"

    return [
        Scenario(
            name="init_enables_adc_unit",
            description="Adc_Init phải bật clock ADC1 và ADON.",
            apis=["Adc_Init"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "set $s0 = (unsigned int)Adc_GetGroupStatus(0)",
            ],
            checks=[
                Check("adc1_clock", f"(({rcc_apb2enr} & (1u<<9)) != 0)", 1),
                Check("adc1_adon", f"(({adc1_cr2} & 0x1u) != 0)", 1),
                Check("group0_idle", "$s0", 0),
            ],
        ),
        Scenario(
            name="setup_result_buffer_group0",
            description="Adc_SetupResultBuffer group 0 trả E_OK.",
            apis=["Adc_Init", "Adc_SetupResultBuffer"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "set $ret = (unsigned int)Adc_SetupResultBuffer(0, (Adc_ValueGroupType*)g_group0_result_buffer)",
            ],
            checks=[
                Check("ret", "$ret", 0),
            ],
        ),
        Scenario(
            name="start_group_conversion_sw",
            description="Adc_StartGroupConversion group 0 đưa trạng thái sang BUSY.",
            apis=["Adc_Init", "Adc_SetupResultBuffer", "Adc_StartGroupConversion", "Adc_GetGroupStatus"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_SetupResultBuffer(0, (Adc_ValueGroupType*)g_group0_result_buffer)",
                "call Adc_StartGroupConversion(0)",
                "set $st = (unsigned int)Adc_GetGroupStatus(0)",
            ],
            checks=[
                Check("status", "$st", 1),
            ],
        ),
        Scenario(
            name="read_group_after_start",
            description="Adc_ReadGroup đọc giá trị từ DR vào buffer và trả E_OK.",
            apis=["Adc_Init", "Adc_SetupResultBuffer", "Adc_StartGroupConversion", "Adc_ReadGroup"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_SetupResultBuffer(0, (Adc_ValueGroupType*)g_group0_result_buffer)",
                "call Adc_StartGroupConversion(0)",
                f"set {adc1_dr} = 1977",
                "set $ret = (unsigned int)Adc_ReadGroup(0, (Adc_ValueGroupType*)g_readback_buffer)",
                "set $st = (unsigned int)Adc_GetGroupStatus(0)",
            ],
            checks=[
                Check("ret", "$ret", 0),
                Check("value0", "g_readback_buffer[0]", 1977),
                Check("status", "$st", 2),
            ],
        ),
        Scenario(
            name="stop_group_conversion",
            description="Adc_StopGroupConversion đưa trạng thái group về IDLE.",
            apis=["Adc_Init", "Adc_SetupResultBuffer", "Adc_StartGroupConversion", "Adc_StopGroupConversion", "Adc_GetGroupStatus"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_SetupResultBuffer(0, (Adc_ValueGroupType*)g_group0_result_buffer)",
                "call Adc_StartGroupConversion(0)",
                "call Adc_StopGroupConversion(0)",
                "set $st = (unsigned int)Adc_GetGroupStatus(0)",
            ],
            checks=[
                Check("status", "$st", 0),
            ],
        ),
        Scenario(
            name="enable_hardware_trigger_group1",
            description="Adc_EnableHardwareTrigger phải bật EXTTRIG cho group 1.",
            apis=["Adc_Init", "Adc_SetupResultBuffer", "Adc_EnableHardwareTrigger"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_SetupResultBuffer(1, (Adc_ValueGroupType*)g_group1_stream_buffer)",
                "call Adc_EnableHardwareTrigger(1)",
            ],
            checks=[
                Check("exttrig", f"(({adc1_cr2} & (1u<<20)) != 0)", 1),
                Check("extsel", f"(({adc1_cr2} & 0x000E0000u) == 0x00060000u)", 1),
            ],
        ),
        Scenario(
            name="disable_hardware_trigger_group1",
            description="Adc_DisableHardwareTrigger phải tắt EXTTRIG.",
            apis=["Adc_Init", "Adc_SetupResultBuffer", "Adc_EnableHardwareTrigger", "Adc_DisableHardwareTrigger"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_SetupResultBuffer(1, (Adc_ValueGroupType*)g_group1_stream_buffer)",
                "call Adc_EnableHardwareTrigger(1)",
                "call Adc_DisableHardwareTrigger(1)",
            ],
            checks=[
                Check("exttrig", f"(({adc1_cr2} & (1u<<20)) != 0)", 0),
            ],
        ),
        Scenario(
            name="enable_group_notification",
            description="Adc_EnableGroupNotification phải bật bit EOCIE trong CR1.",
            apis=["Adc_Init", "Adc_EnableGroupNotification"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_EnableGroupNotification(0)",
            ],
            checks=[
                Check("eocie", f"(({adc1_cr1} & 0x20u) != 0)", 1),
            ],
        ),
        Scenario(
            name="disable_group_notification",
            description="Adc_DisableGroupNotification phải clear bit EOCIE.",
            apis=["Adc_Init", "Adc_EnableGroupNotification", "Adc_DisableGroupNotification"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_EnableGroupNotification(0)",
                "call Adc_DisableGroupNotification(0)",
            ],
            checks=[
                Check("eocie", f"(({adc1_cr1} & 0x20u) != 0)", 0),
            ],
        ),
        Scenario(
            name="stream_last_pointer_group1",
            description="Adc_GetStreamLastPointer trả pointer hợp lệ và số sample hợp lệ sau ReadGroup.",
            apis=["Adc_Init", "Adc_SetupResultBuffer", "Adc_EnableHardwareTrigger", "Adc_StartGroupConversion", "Adc_ReadGroup", "Adc_GetStreamLastPointer"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_SetupResultBuffer(1, (Adc_ValueGroupType*)g_group1_stream_buffer)",
                "set g_group1_stream_buffer[0] = 111",
                "set g_group1_stream_buffer[1] = 222",
                "call Adc_EnableHardwareTrigger(1)",
                "call Adc_StartGroupConversion(1)",
                "set $ret = (unsigned int)Adc_ReadGroup(1, (Adc_ValueGroupType*)g_readback_buffer)",
                "set $cnt = (unsigned int)Adc_GetStreamLastPointer(1, (Adc_ValueGroupType**)&g_stream_ptr)",
            ],
            checks=[
                Check("ret", "$ret", 0),
                Check("cnt", "$cnt", 2),
                Check("ptr", "(unsigned int)g_stream_ptr", 1, comparator="nonzero"),
                Check("v0", "g_readback_buffer[0]", 111),
                Check("v1", "g_readback_buffer[1]", 222),
                Check("dma_cmar", dma1_cmar1, 1, comparator="nonzero"),
                Check("dma_cndtr", dma1_cndtr1, 4),
            ],
        ),
        Scenario(
            name="version_info",
            description="Adc_GetVersionInfo trả đúng thông tin phiên bản module.",
            apis=["Adc_GetVersionInfo"],
            commands=[
                "call Adc_GetVersionInfo((Std_VersionInfoType*)&g_version_info)",
            ],
            checks=[
                Check("vendor", "g_version_info.vendorID", 0x1234),
                Check("module", "g_version_info.moduleID", 0x007B),
                Check("major", "g_version_info.sw_major_version", 1),
                Check("minor", "g_version_info.sw_minor_version", 1),
            ],
        ),
        Scenario(
            name="deinit_disables_adon",
            description="Adc_DeInit phải đưa ADON về 0.",
            apis=["Adc_Init", "Adc_DeInit"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_DeInit()",
            ],
            checks=[
                Check("adon", f"(({adc1_cr2} & 0x1u) != 0)", 0),
            ],
        ),
        Scenario(
            name="adc_isr_handler_callback",
            description="Adc_IsrHandler phải gọi callback Group0 khi EOC interrupt bật.",
            apis=["Adc_Init", "Adc_SetupResultBuffer", "Adc_EnableGroupNotification", "Adc_StartGroupConversion"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_SetupResultBuffer(0, (Adc_ValueGroupType*)g_group0_result_buffer)",
                "call Adc_EnableGroupNotification(0)",
                "call Adc_StartGroupConversion(0)",
                f"set {adc1_dr} = 1234",
                f"set {adc1_sr} = ({adc1_sr} | 0x2u)",
                "call Adc_IsrHandler((ADC_TypeDef*)0x40012400)",
            ],
            checks=[
                Check("irq_count", "g_eoc_irq_count", 1, comparator="nonzero"),
                Check("buf0", "g_group0_result_buffer[0]", 1234),
            ],
        ),
        Scenario(
            name="adc_dma_isr_handler_callbacks",
            description="Adc_DmaIsrHandler phải xử lý cờ HT/TC và gọi callback DMA.",
            apis=["Adc_Init", "Adc_SetupResultBuffer", "Adc_EnableHardwareTrigger", "Adc_StartGroupConversion"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "call Adc_SetupResultBuffer(1, (Adc_ValueGroupType*)g_group1_stream_buffer)",
                "set g_group1_stream_buffer[0] = 555",
                "set g_group1_stream_buffer[1] = 777",
                "call Adc_EnableHardwareTrigger(1)",
                "call Adc_StartGroupConversion(1)",
                f"set {dma1_isr} = ({dma1_isr} | 0x6u)",
                "call Adc_DmaIsrHandler((DMA_Channel_TypeDef*)0x40020008)",
                "set $st = (unsigned int)Adc_GetGroupStatus(1)",
            ],
            checks=[
                Check("dma_ht", "g_dma_ht_count", 1, comparator="nonzero"),
                Check("dma_tc", "g_dma_tc_count", 1, comparator="nonzero"),
                Check("status", "$st", 3),
            ],
        ),
        Scenario(
            name="power_set_without_prepare",
            description="Adc_SetPowerState trước Prepare phải báo lỗi sequence.",
            apis=["Adc_Init", "Adc_SetPowerState"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "set $ret = (unsigned int)Adc_SetPowerState((Adc_PowerStateRequestResultType*)&g_power_result)",
            ],
            checks=[
                Check("ret", "$ret", 1),
                Check("result", "g_power_result", 2),
            ],
        ),
        Scenario(
            name="power_prepare_set_get_current",
            description="Prepare + Set + GetCurrentPowerState phải thành công.",
            apis=["Adc_Init", "Adc_PreparePowerState", "Adc_SetPowerState", "Adc_GetCurrentPowerState"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "set $r0 = (unsigned int)Adc_PreparePowerState((Adc_PowerStateType)1, (Adc_PowerStateRequestResultType*)&g_power_result)",
                "set $r1 = (unsigned int)Adc_SetPowerState((Adc_PowerStateRequestResultType*)&g_power_result)",
                "set $r2 = (unsigned int)Adc_GetCurrentPowerState((Adc_PowerStateType*)&g_power_state, (Adc_PowerStateRequestResultType*)&g_power_result)",
            ],
            checks=[
                Check("r0", "$r0", 0),
                Check("r1", "$r1", 0),
                Check("r2", "$r2", 0),
                Check("state", "g_power_state", 1),
                Check("result", "g_power_result", 0),
                Check("adon", f"(({adc1_cr2} & 0x1u) != 0)", 0),
            ],
        ),
        Scenario(
            name="power_get_target",
            description="Adc_GetTargetPowerState trả đúng trạng thái đã Prepare.",
            apis=["Adc_Init", "Adc_PreparePowerState", "Adc_GetTargetPowerState"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "set $r0 = (unsigned int)Adc_PreparePowerState((Adc_PowerStateType)1, (Adc_PowerStateRequestResultType*)&g_power_result)",
                "set $r1 = (unsigned int)Adc_GetTargetPowerState((Adc_PowerStateType*)&g_power_state, (Adc_PowerStateRequestResultType*)&g_power_result)",
            ],
            checks=[
                Check("r0", "$r0", 0),
                Check("r1", "$r1", 0),
                Check("state", "g_power_state", 1),
            ],
        ),
        Scenario(
            name="main_power_transition_manager_safe",
            description="Adc_Main_PowerTransitionManager chạy an toàn.",
            apis=["Adc_Main_PowerTransitionManager"],
            commands=[
                "call Adc_Main_PowerTransitionManager()",
                "set $ok = 1",
            ],
            checks=[
                Check("ok", "$ok", 1),
            ],
        ),
        Scenario(
            name="invalid_group_negative",
            description="API với group invalid không làm hỏng trạng thái hệ thống.",
            apis=["Adc_Init", "Adc_SetupResultBuffer", "Adc_StartGroupConversion", "Adc_GetGroupStatus"],
            commands=[
                "call Adc_Init(&AdcDriverConfig)",
                "set $ret = (unsigned int)Adc_SetupResultBuffer(99, (Adc_ValueGroupType*)g_group0_result_buffer)",
                "call Adc_StartGroupConversion(99)",
                "set $st = (unsigned int)Adc_GetGroupStatus(99)",
            ],
            checks=[
                Check("ret", "$ret", 1),
                Check("status", "$st", 0),
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
    trace_json_path = report_dir / f"adc_api_traceability_{ts}.json"
    trace_csv_path = report_dir / f"adc_api_traceability_{ts}.csv"
    trace_md_path = report_dir / f"adc_api_traceability_{ts}.md"

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
    md_lines.append("# ADC Traceability Report")
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
    report_dir.mkdir(parents=True, exist_ok=True)
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")

    legacy_json_path = report_dir / f"adc_api_renode_gdb_report_{ts}.json"
    legacy_md_path = report_dir / f"adc_api_renode_gdb_report_{ts}.md"
    details_json_path = report_dir / f"adc_api_renode_gdb_details_{ts}.json"
    summary_json_path = report_dir / f"adc_api_renode_gdb_summary_{ts}.json"

    failed_scenarios = [x["name"] for x in details_data.get("scenarios", []) if not x.get("passed", False)]
    summary_data = {
        "generated_at": details_data.get("generated_at"),
        "status": "PASS"
        if (
            details_data.get("summary", {}).get("failed", 1) == 0
            and details_data.get("build", {}).get("success", False)
        )
        else "FAIL",
        "exit_code": exit_code,
        "scenario_summary": details_data.get("summary", {}),
        "traceability_summary": traceability_data.get("summary", {}),
        "failed_scenarios": failed_scenarios,
        "build_success": details_data.get("build", {}).get("success", False),
    }

    details_json_path.write_text(json.dumps(details_data, indent=2, ensure_ascii=False), encoding="utf-8")
    summary_json_path.write_text(json.dumps(summary_data, indent=2, ensure_ascii=False), encoding="utf-8")
    legacy_json_path.write_text(json.dumps(details_data, indent=2, ensure_ascii=False), encoding="utf-8")

    lines: list[str] = []
    lines.append("# ADC API Test Report (Renode + GDB)")
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
    parser = argparse.ArgumentParser(description="Run ADC API scenarios via Renode + GDB")
    parser.add_argument("--root", type=Path, default=root, help="Project root")
    parser.add_argument("--repl", type=Path, default=root / "stm32f103_full.repl", help="Path to .repl file")
    parser.add_argument(
        "--sws-json",
        type=Path,
        default=root / "Adc/tools/adc_sws_r25_11_extracted.json",
        help="Path file JSON đã trích xuất từ AUTOSAR ADC SWS",
    )
    parser.add_argument("--gdb-port", type=int, default=3333, help="GDB server port")
    parser.add_argument("--monitor-port", type=int, default=12345, help="Renode monitor TCP port")
    parser.add_argument("--renode", default=os.environ.get("RENODE_PATH"), help="Path Renode executable")
    parser.add_argument("--gdb", default=os.environ.get("GDB_PATH"), help="Path arm-none-eabi-gdb")
    parser.add_argument("--gcc", default=os.environ.get("GCC_PATH"), help="Path arm-none-eabi-gcc")
    parser.add_argument("--skip-build", action="store_true", help="Skip build step")
    parser.add_argument("--report-dir", type=Path, default=root / "Adc/test_reports", help="Directory output reports")
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

    build_dir = root / "Adc/tests/renode/build"
    build_ok = True
    build_stdout = ""
    build_stderr = ""
    elf = build_dir / "adc_api_test.elf"
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
                },
                "build": {
                    "success": False,
                    "elf": str(elf),
                    "stdout": build_stdout,
                    "stderr": build_stderr,
                },
                "summary": {
                    "total": len(scenarios),
                    "passed": 0,
                    "failed": len(scenarios),
                },
                "scenarios": [],
            }
            traceability_data = build_traceability_data([], api_requirement_map)
            outputs = write_reports(args.report_dir, details_data, traceability_data, exit_code=2)
            print(f"Build FAIL. Reports: {outputs}")
            return 2

    if not elf.exists():
        raise SystemExit(f"ELF không tồn tại: {elf}")

    resc_path = build_dir / "adc_api_test.resc"
    renode_log_path = build_dir / "renode.log"
    write_resc(resc_path, repl, elf, args.gdb_port)

    renode_proc: subprocess.Popen | None = None
    scenario_results: list[ScenarioResult] = []

    try:
        renode_proc = start_renode(renode, resc_path, renode_log_path, monitor_port)
        if not wait_for_tcp_port("127.0.0.1", args.gdb_port, args.port_timeout):
            raise RuntimeError(f"Timeout chờ GDB server cổng {args.gdb_port}")

        for sc in scenarios:
            result = run_gdb_scenario(gdb, elf, args.gdb_port, sc)
            scenario_results.append(result)
            status = "PASS" if result.passed else "FAIL"
            print(f"[{status}] {sc.name}")

    finally:
        stop_renode(renode_proc)

    passed = sum(1 for x in scenario_results if x.passed)
    failed = len(scenario_results) - passed

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
        },
        "build": {
            "success": build_ok,
            "elf": str(elf),
            "stdout": build_stdout,
            "stderr": build_stderr,
        },
        "summary": {
            "total": len(scenario_results),
            "passed": passed,
            "failed": failed,
        },
        "scenarios": [
            {
                **asdict(x),
                "checks": [asdict(c) for c in x.checks],
            }
            for x in scenario_results
        ],
    }

    traceability_data = build_traceability_data(scenario_results, api_requirement_map)
    exit_code = 0 if (failed == 0 and build_ok) else 1
    outputs = write_reports(args.report_dir, details_data, traceability_data, exit_code=exit_code)

    print(f"Scenarios: {passed}/{len(scenario_results)} PASS")
    print(f"Requirement PASS: {traceability_data['summary']['passed_requirements']}/{traceability_data['summary']['total_requirements']}")
    print("Reports:")
    for name, path in outputs.items():
        print(f"- {name}: {path}")

    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
