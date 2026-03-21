#!/usr/bin/env python3
"""Build toàn bộ exmaple PWM cho STM32F103 (SPL).

Mục tiêu:
1) Dò tất cả case `exmaple/*/case_*/main.c`.
2) Build từng case thành ELF độc lập.
3) Xuất report JSON để tổng hợp PASS/FAIL.
"""

from __future__ import annotations

import json
import shlex
import subprocess
from dataclasses import asdict, dataclass
from datetime import datetime
from pathlib import Path


@dataclass
class BuildCaseResult:
    group: str
    case: str
    main_file: str
    elf: str
    passed: bool
    gcc: str
    compile_stdout: str
    compile_stderr: str
    link_stdout: str
    link_stderr: str


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


def collect_case_main_files(exmaple_dir: Path) -> list[Path]:
    return sorted(exmaple_dir.glob("*/case_*/main.c"))


def build_one_case(
    root: Path,
    gcc: str,
    main_file: Path,
    output_root: Path,
) -> BuildCaseResult:
    group = main_file.parent.parent.name
    case = main_file.parent.name
    case_build_dir = output_root / group / case
    obj_dir = case_build_dir / "obj"
    obj_dir.mkdir(parents=True, exist_ok=True)

    elf = case_build_dir / f"{case}.elf"
    map_file = case_build_dir / f"{case}.map"

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
        "-IPwm/exmaple/common",
    ]

    sources = [
        main_file,
        root / "Pwm/exmaple/common/pwm_example_port.c",
        root / "Pwm/Pwm.c",
        root / "Pwm/Pwm_Lcfg.c",
        root / "platform/spl/src/system_stm32f10x.c",
        root / "platform/spl/src/stm32f10x_rcc.c",
        root / "platform/spl/src/stm32f10x_gpio.c",
        root / "platform/spl/src/stm32f10x_tim.c",
        root / "platform/bsp/startup_stm32f10x_md.s",
    ]

    compile_out_all: list[str] = []
    compile_err_all: list[str] = []
    objs: list[Path] = []

    for src in sources:
        obj = obj_dir / f"{src.stem}.o"
        objs.append(obj)
        cmd = [gcc, *cflags, "-c", str(src), "-o", str(obj)]
        rc = run_command(cmd, cwd=root)
        compile_out_all.append(rc.stdout)
        compile_err_all.append(rc.stderr)
        if rc.returncode != 0:
            return BuildCaseResult(
                group=group,
                case=case,
                main_file=str(main_file),
                elf=str(elf),
                passed=False,
                gcc=gcc,
                compile_stdout="".join(compile_out_all),
                compile_stderr="".join(compile_err_all),
                link_stdout="",
                link_stderr="",
            )

    ldflags = [
        "-T",
        str(root / "platform/bsp/linker/stm32f103.ld"),
        "-nostartfiles",
        "-nostdlib",
        "-Wl,-Map=" + str(map_file),
    ]
    link_cmd = [gcc, *cflags, *(str(x) for x in objs), *ldflags, "-o", str(elf)]
    rc_link = run_command(link_cmd, cwd=root)

    return BuildCaseResult(
        group=group,
        case=case,
        main_file=str(main_file),
        elf=str(elf),
        passed=(rc_link.returncode == 0),
        gcc=gcc,
        compile_stdout="".join(compile_out_all),
        compile_stderr="".join(compile_err_all),
        link_stdout=rc_link.stdout,
        link_stderr=rc_link.stderr,
    )


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    exmaple_dir = root / "Pwm/exmaple"
    output_root = exmaple_dir / "build"
    output_root.mkdir(parents=True, exist_ok=True)

    gcc = find_binary(None, ["arm-none-eabi-gcc"])
    main_files = collect_case_main_files(exmaple_dir)
    if not main_files:
        raise SystemExit(f"Không tìm thấy case trong: {exmaple_dir}")

    results: list[BuildCaseResult] = []
    for main_file in main_files:
        results.append(build_one_case(root, gcc, main_file, output_root))

    passed = sum(1 for x in results if x.passed)
    total = len(results)
    failed = total - passed

    report = {
        "generated_at": datetime.now().isoformat(),
        "tool": "Pwm/exmaple/build_examples.py",
        "gcc": gcc,
        "summary": {
            "total": total,
            "passed": passed,
            "failed": failed,
        },
        "cases": [asdict(x) for x in results],
    }

    report_path = output_root / "build_report.json"
    report_path.write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding="utf-8")

    print(f"GCC: {gcc}")
    print(f"Case passed: {passed}/{total}")
    for r in results:
        status = "PASS" if r.passed else "FAIL"
        print(f"- [{status}] {r.group}/{r.case} -> {r.elf}")
    print(f"Build report: {report_path}")

    if failed > 0:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
