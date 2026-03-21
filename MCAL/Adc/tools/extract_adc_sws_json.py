#!/usr/bin/env python3
"""Extract key AUTOSAR ADC SWS data from PDF into planning-friendly JSON.

Usage:
  python Adc/tools/extract_adc_sws_json.py \
    --pdf AUTOSAR_CP_SWS_ADCDriver.pdf \
    --out Adc/tools/adc_sws_r25_11_extracted.json
"""

from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

try:
    from pypdf import PdfReader
except Exception as exc:  # pragma: no cover - import guard
    raise SystemExit(
        "Missing dependency 'pypdf'. Install with: python -m pip install pypdf"
    ) from exc


NORMALIZATION_REPLACEMENTS = {
    "Adc_EnableHardwareT rigger": "Adc_EnableHardwareTrigger",
    "Adc_DisableHardwareT rigger": "Adc_DisableHardwareTrigger",
    "Adc_GetStreamLastPointer ": "Adc_GetStreamLastPointer ",
    "Adc_GetT argetPowerState": "Adc_GetTargetPowerState",
    "Adc_Main_PowerT ransitionManager": "Adc_Main_PowerTransitionManager",
    "Adc_ChannelT ype": "Adc_ChannelType",
    "Adc_GroupT ype": "Adc_GroupType",
    "Adc_StreamNumSampleT ype": "Adc_StreamNumSampleType",
    "Adc_ConfigT ype": "Adc_ConfigType",
    "Adc_PowerStateRequestResultT ype": "Adc_PowerStateRequestResultType",
    "Adc_PowerStateT ype": "Adc_PowerStateType",
    "Std_ReturnT ype": "Std_ReturnType",
    "Std_VersionInfoT ype": "Std_VersionInfoType",
    "SRS_Ad c_": "SRS_Adc_",
    "SRS_SP AL_": "SRS_SPAL_",
    "T argetPowerState": "TargetPowerState",
    "AdcPowerStateAsynchT ransitionMode": "AdcPowerStateAsynchTransitionMode",
    "ADC_E_P ARAM_GROUP": "ADC_E_PARAM_GROUP",
    "ADC_E_P ARAM_POINTER": "ADC_E_PARAM_POINTER",
    "ADC_E_NOTIF_CAP ABILITY": "ADC_E_NOTIF_CAPABILITY",
    "ADC_E_PERIPHERAL_NOT_PREP ARED": "ADC_E_PERIPHERAL_NOT_PREPARED",
    "ADE_E_BUFFER_UNINIT": "ADC_E_BUFFER_UNINIT",
    "ADE_E_POWER_ST A TE_NOT_SUPPORTED": "ADC_E_POWER_STATE_NOT_SUPPORTED",
    "ADC_E_POWER_ST A TE_NOT_SUPPORTED": "ADC_E_POWER_STATE_NOT_SUPPORTED",
    "ADC_E_TRANSITION_ NOT_POSSIBLE": "ADC_E_TRANSITION_NOT_POSSIBLE",
}


IMPORTANT_REQUIREMENTS: dict[str, str] = {}


@dataclass
class Section:
    idx: int
    section_id: str
    title: str


def normalize_text(text: str) -> str:
    out = text.replace("\r", "\n")
    out = re.sub(r"(\w)-\n(\w)", r"\1\2", out)
    for src, dst in NORMALIZATION_REPLACEMENTS.items():
        out = out.replace(src, dst)
    out = re.sub(r"\u00a0", " ", out)
    return out


def squash_ws(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


def trim_at_marker(value: str | None, marker: str) -> str | None:
    if value is None:
        return None
    if marker in value:
        value = value.split(marker, 1)[0]
    return squash_ws(value)


def normalize_reentrancy(value: str | None) -> str | None:
    if value is None:
        return None
    value = squash_ws(value)
    if "Reentrant for different channel numbers" in value:
        return "Reentrant for different channel numbers"
    return value


def extract_pdf_text(pdf_path: Path) -> tuple[list[str], str]:
    reader = PdfReader(str(pdf_path))
    pages = [(page.extract_text() or "") for page in reader.pages]
    full_text = "\n".join(pages)
    return pages, normalize_text(full_text)


def parse_document_info(text: str, pages: list[str], source_pdf: Path) -> dict[str, Any]:
    title_match = re.search(r"Document Title\s+(.+?)\s+Document Owner", text, re.S)
    release_match = re.search(r"AUTOSAR CP\s+(R\d{2}-\d{2})", text)
    doc_id_match = re.search(r"Document Identification No\s+(\d+)", text)

    title = squash_ws(title_match.group(1)) if title_match else source_pdf.stem
    release = release_match.group(1) if release_match else None
    doc_id = doc_id_match.group(1) if doc_id_match else None

    return {
        "source_pdf": str(source_pdf),
        "title": title,
        "release": release,
        "document_id": doc_id,
        "page_count": len(pages),
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
    }


def section_text(text: str, start_pat: str, end_pat: str | None = None) -> str:
    starts = list(re.finditer(start_pat, text))
    if not starts:
        return ""
    # Prefer the body section instead of table-of-contents occurrence.
    start_idx = starts[-1].start()
    if end_pat:
        end = re.search(end_pat, text[start_idx:])
        if end:
            return text[start_idx : start_idx + end.start()]
    return text[start_idx:]


def parse_dependencies(text: str) -> list[dict[str, str]]:
    sec = section_text(text, r"5 Dependencies to other modules", r"5\.1 File structure")
    lines = [ln.strip() for ln in sec.splitlines() if ln.strip()]

    deps: list[dict[str, str]] = []
    dep_markers = {
        "PORT Driver": "Set port pin functionality.",
        "MCU Driver": "Set prescaler, system clock, PLL.",
        "DET": "Report development/runtime errors.",
    }
    for marker, fallback_desc in dep_markers.items():
        for ln in lines:
            if marker in ln:
                m = re.search(rf"{re.escape(marker)}\s*\[[0-9]+\]:\s*(.*)", ln)
                desc = squash_ws(m.group(1)) if m else fallback_desc
                deps.append({"module": marker, "description": desc})
                break
    return deps


def find_service_sections(lines: list[str]) -> list[Section]:
    sections: list[Section] = []
    pat = re.compile(r"^(8\.(?:3|5)\.\d+)\s+(Adc_[A-Za-z0-9_]+)$")
    for idx, line in enumerate(lines):
        m = pat.match(line.strip())
        if not m:
            continue
        # Ignore table-of-contents rows with dotted leaders.
        if ". ." in line:
            continue
        sections.append(Section(idx=idx, section_id=m.group(1), title=m.group(2)))
    # Keep only the real body sections (later in text).
    return [s for s in sections if s.idx > 500]


def extract_field_from_lines(lines: list[str], label: str, stop_labels: list[str]) -> str | None:
    start_idx = next((i for i, ln in enumerate(lines) if ln.startswith(label)), None)
    if start_idx is None:
        return None

    first = lines[start_idx][len(label) :].strip()
    parts: list[str] = [first] if first else []
    for i in range(start_idx + 1, len(lines)):
        ln = lines[i].strip()
        if not ln:
            continue
        if any(ln.startswith(stop) for stop in stop_labels):
            break
        if re.match(r"^\d+ of \d+", ln):
            continue
        if ln in {"▽", "△"}:
            continue
        if ln.startswith("Specification of ADC Driver") or ln.startswith("AUTOSAR CP "):
            continue
        parts.append(ln)

    if not parts:
        return None
    return squash_ws(" ".join(parts))


def parse_api_section(block: str, section: Section) -> dict[str, Any]:
    block_lines = [ln.strip() for ln in block.splitlines() if ln.strip()]
    service_start = next((i for i, ln in enumerate(block_lines) if ln.startswith("Service Name ")), 0)
    service_end = next(
        (i for i in range(service_start, len(block_lines)) if block_lines[i].startswith("Available via")),
        None,
    )
    if service_end is not None:
        service_def_lines = block_lines[service_start : service_end + 1]
    else:
        service_def_lines = block_lines

    definition_id = None
    m = re.search(r"\[(SWS_Adc_[A-Za-z0-9_]+)\]\s+Definition of (?:API function|scheduled function)", block)
    if m:
        definition_id = m.group(1)

    service_name = None
    m = re.search(r"Service Name\s+([A-Za-z0-9_<>#]+)", block)
    if m:
        service_name = m.group(1)

    syntax = extract_field_from_lines(
        service_def_lines,
        "Syntax",
        ["Service ID [hex]", "Sync/Async", "Reentrancy"],
    )
    service_id = None
    m = re.search(r"Service ID \[hex\]\s+(0x[0-9A-Fa-f]+)", block)
    if m:
        service_id = m.group(1)

    sync = extract_field_from_lines(
        service_def_lines,
        "Sync/Async",
        ["Reentrancy", "Parameters (in)", "Parameters (inout)", "Parameters (out)", "Return value", "Description"],
    )
    reentrancy = extract_field_from_lines(
        service_def_lines,
        "Reentrancy",
        ["Parameters (in)", "Parameters (inout)", "Parameters (out)", "Return value", "Description"],
    )
    reentrancy = trim_at_marker(reentrancy, "Parameters (in)")
    reentrancy = normalize_reentrancy(reentrancy)
    return_value = extract_field_from_lines(service_def_lines, "Return value", ["Description", "Available via"])
    description = extract_field_from_lines(
        service_def_lines,
        "Description",
        ["Available via", "[SWS_Adc_", "8."],
    )
    available_via = None
    avail_line = next((ln for ln in service_def_lines if ln.startswith("Available via")), None)
    if avail_line:
        value = avail_line.replace("Available via", "", 1).strip()
        available_via = value.split()[0] if value else None

    req_ids = sorted(set(re.findall(r"\[(SWS_Adc_[A-Za-z0-9_]+)\]", block)))
    upstream_ids = sorted(set(re.findall(r"\b(SRS_[A-Za-z0-9_]+)\b", block)))

    return {
        "section": section.section_id,
        "title": section.title,
        "name": service_name or section.title,
        "definition_requirement": definition_id,
        "service_id_hex": service_id,
        "syntax": syntax,
        "sync_async": sync,
        "reentrancy": reentrancy,
        "return_value": return_value,
        "description": description,
        "available_via": available_via,
        "requirement_ids": req_ids,
        "upstream_requirements": upstream_ids,
    }


def parse_apis(lines: list[str]) -> list[dict[str, Any]]:
    sections = find_service_sections(lines)
    if not sections:
        return []

    major_boundary_indices = [
        idx
        for idx, ln in enumerate(lines)
        if re.match(r"^8\.(6|7|8|9|10)\b", ln.strip())
    ]

    apis: list[dict[str, Any]] = []
    for idx, section in enumerate(sections):
        next_api_idx = sections[idx + 1].idx if idx + 1 < len(sections) else len(lines)
        next_major_idx = next((i for i in major_boundary_indices if i > section.idx), len(lines))
        end_idx = min(next_api_idx, next_major_idx)
        block = "\n".join(lines[section.idx : end_idx])
        apis.append(parse_api_section(block, section))
    return apis


def parse_configurable_interfaces(text: str) -> list[dict[str, Any]]:
    sec = section_text(text, r"8\.6\.[23] Configurable interfaces", r"8\.7 Service Interfaces")
    lines = [ln.strip() for ln in sec.splitlines() if ln.strip()]
    starts = [i for i, line in enumerate(lines) if line.startswith("Service Name ")]

    interfaces: list[dict[str, Any]] = []
    for pos, start_idx in enumerate(starts):
        end_idx = starts[pos + 1] if pos + 1 < len(starts) else len(lines)
        block = "\n".join(lines[start_idx:end_idx])

        name = lines[start_idx].replace("Service Name", "", 1).strip()
        block_lines = [ln.strip() for ln in block.splitlines() if ln.strip()]
        syntax = extract_field_from_lines(block_lines, "Syntax", ["Sync/Async", "Reentrancy", "Parameters (in)"])
        sync = extract_field_from_lines(block_lines, "Sync/Async", ["Reentrancy", "Parameters (in)"])
        reent = extract_field_from_lines(block_lines, "Reentrancy", ["Parameters (in)", "Parameters (out)"])
        desc = extract_field_from_lines(block_lines, "Description", ["Available via", "[SWS_Adc_", "8."])
        header = None
        avail_line = next((ln for ln in block_lines if ln.startswith("Available via")), None)
        if avail_line:
            value = avail_line.replace("Available via", "", 1).strip()
            header = value.split()[0] if value else None
        req_ids = sorted(set(re.findall(r"\[(SWS_Adc_[A-Za-z0-9_]+)\]", block)))

        interfaces.append(
            {
                "name": name,
                "syntax": syntax,
                "sync_async": sync,
                "reentrancy": reent,
                "description": desc,
                "available_via": header,
                "requirement_ids": req_ids,
            }
        )
    return interfaces


def parse_error_section(lines: list[str], start_marker: str, end_marker: str | None = None) -> str:
    start_candidates = [i for i, ln in enumerate(lines) if ln.strip().startswith(start_marker)]
    start_idx = start_candidates[-1] if start_candidates else None
    if start_idx is None:
        return ""
    if end_marker is None:
        end_idx = len(lines)
    else:
        end_idx = next(
            (i for i, ln in enumerate(lines[start_idx + 1 :], start_idx + 1) if ln.strip().startswith(end_marker)),
            len(lines),
        )
    return "\n".join(lines[start_idx:end_idx])


def normalize_error_name(name: str) -> str:
    return name.replace(" ", "").replace("\n", "")


def parse_errors(lines: list[str]) -> dict[str, Any]:
    dev_text = parse_error_section(lines, "7.5.1 Development Errors", "7.5.2 Runtime Errors")
    run_text = parse_error_section(lines, "7.5.2 Runtime Errors", "7.5.3 Production Errors")
    if not dev_text and not run_text:
        # Fallback cho các phiên bản tài liệu có đánh số mục cũ hơn/mới hơn.
        dev_text = parse_error_section(lines, "7.6.1 Development Errors", "7.6.2 Runtime Errors")
        run_text = parse_error_section(lines, "7.6.2 Runtime Errors", "7.6.3 Production Errors")

    def _extract(text: str) -> list[dict[str, str]]:
        found: list[dict[str, str]] = []
        seen: set[tuple[str, str]] = set()
        for name, code in re.findall(r"((?:ADC|ADE)_E_[A-Z_ ]+?)\s+(0x[0-9A-Fa-f]+)", text):
            clean_name = normalize_error_name(name).replace("ADE_E_", "ADC_E_")
            key = (clean_name, code)
            if key in seen:
                continue
            seen.add(key)
            found.append({"name": clean_name, "value": code})
        return found

    dev_errors = _extract(dev_text)
    runtime_errors = _extract(run_text)

    return {
        "development": dev_errors,
        "runtime": runtime_errors,
        "production": [],
        "extended_production": [],
        "development_requirement_ids": sorted(set(re.findall(r"\[(SWS_Adc_[A-Za-z0-9_]+)\]", dev_text))),
        "runtime_requirement_ids": sorted(set(re.findall(r"\[(SWS_Adc_[A-Za-z0-9_]+)\]", run_text))),
    }


def _extract_multiline_value(body: str, label: str) -> str | None:
    stop_labels = [
        "Container Name",
        "Parameter Name",
        "Parent Container",
        "Description",
        "Multiplicity",
        "Type",
        "Range",
        "Default value",
        "Post-Build Variant",
        "Pre-compile time",
        "Link time",
        "Value Configuration Class",
        "Multiplicity Configuration Class",
        "Configuration Parameters",
        "Included Parameters",
        "Included Containers",
        "No Included Containers",
        "Dependency",
        "Regular Expression",
    ]
    pat = rf"{re.escape(label)}\s+(.+?)(?:\n(?:{'|'.join(re.escape(s) for s in stop_labels)})\b|$)"
    m = re.search(pat, body, re.S)
    if not m:
        return None
    return squash_ws(m.group(1))


def parse_ecuc_definitions(text: str) -> dict[str, Any]:
    pat = re.compile(
        r"\[(ECUC_Adc_[0-9]+)\]\s+Definition of\s+(.+?)\s+⌈(.*?)⌋",
        re.S,
    )
    containers: list[dict[str, Any]] = []
    parameters: list[dict[str, Any]] = []

    for m in pat.finditer(text):
        ecuc_id, definition_phrase_raw, body = m.groups()
        definition_phrase = squash_ws(definition_phrase_raw)
        split_idx = definition_phrase.find(" ")
        if split_idx == -1:
            def_type = definition_phrase
            def_name = definition_phrase
        else:
            def_type = definition_phrase[:split_idx]
            def_name = definition_phrase[split_idx + 1 :]

        item: dict[str, Any] = {
            "id": ecuc_id,
            "definition_type": def_type,
            "definition_name": def_name,
            "container_name": _extract_multiline_value(body, "Container Name"),
            "parameter_name": _extract_multiline_value(body, "Parameter Name"),
            "parent_container": _extract_multiline_value(body, "Parent Container"),
            "multiplicity": _extract_multiline_value(body, "Multiplicity"),
            "type": _extract_multiline_value(body, "Type"),
            "description": _extract_multiline_value(body, "Description"),
        }

        if "ContainerDef" in def_type or "ModuleDef" in def_type:
            containers.append(item)
        else:
            parameters.append(item)

    optional_api_switches = sorted(
        p["parameter_name"]
        for p in parameters
        if p.get("parent_container") == "AdcConfigurationOfOptApiServices" and p.get("parameter_name")
    )
    general_switches = sorted(
        p["parameter_name"]
        for p in parameters
        if p.get("parent_container") == "AdcGeneral" and p.get("parameter_name")
    )
    channel_params = sorted(
        p["parameter_name"]
        for p in parameters
        if p.get("parent_container") == "AdcChannel" and p.get("parameter_name")
    )

    return {
        "containers": containers,
        "parameters": parameters,
        "derived": {
            "optional_api_switches": optional_api_switches,
            "general_switches": general_switches,
            "channel_parameters": channel_params,
        },
    }


def important_requirements_status(text: str) -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for req_id, summary in IMPORTANT_REQUIREMENTS.items():
        out.append(
            {
                "id": req_id,
                "present_in_document": req_id in text,
                "summary": summary,
            }
        )
    return out


def build_stm32f103_hint_block(apis: list[dict[str, Any]], config: dict[str, Any]) -> dict[str, Any]:
    api_names = [api["name"] for api in apis if api.get("name")]
    return {
        "target": "stm32f103_spl",
        "hardware_backends": ["ADC1", "ADC2"],
        "api_coverage_from_sws": api_names,
        "recommended_compile_switches": config["derived"].get("optional_api_switches", []),
        "implementation_notes": [
            "Map logical Adc_GroupType tới regular conversion sequence của ADC1/ADC2.",
            "Nếu dùng hardware trigger, map trigger source theo tài nguyên TIM/EXTI của STM32F103.",
            "Tôn trọng cơ chế setup result buffer trước khi read stream/group.",
            "Map DET development errors and runtime errors to the corresponding service IDs.",
            "Cân nhắc DMA cho streaming mode và đồng bộ trạng thái Adc_GroupStatus.",
            "Provide ISR hook để dispatch Adc_Group<n> notification callback khi conversion complete.",
            "If low-power support is not used on STM32F103, set AdcLowPowerStatesSupport=false and gate power APIs.",
        ],
    }


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Extract AUTOSAR ADC SWS info to JSON")
    parser.add_argument("--pdf", required=True, type=Path, help="Path to AUTOSAR ADC SWS PDF")
    parser.add_argument("--out", required=True, type=Path, help="Output JSON path")
    parser.add_argument("--pretty", action="store_true", help="Pretty-print JSON")
    return parser.parse_args()


def main() -> None:
    args = parse_arguments()
    if not args.pdf.exists():
        raise SystemExit(f"Input PDF not found: {args.pdf}")

    pages, text = extract_pdf_text(args.pdf)
    lines = [ln.strip() for ln in text.splitlines() if ln.strip()]

    apis = parse_apis(lines)
    config = parse_ecuc_definitions(text)

    output: dict[str, Any] = {
        "document": parse_document_info(text, pages, args.pdf.resolve()),
        "scope": {
            "limitations": [
                "ADC Driver targets Successive Approximation ADC hardware.",
                "Delta Sigma ADC use cases are out of scope in this SWS release.",
            ],
            "dependencies": parse_dependencies(text),
            "conversion_features": {
                "group_conversion_modes": ["one-shot", "continuous"],
                "trigger_modes": ["software", "hardware"],
                "result_access_modes": ["single", "streaming"],
            },
            "notes": [
                "Each ADC channel group is configured with exactly one trigger source.",
                "Result buffer setup is mandatory before reading conversion results.",
            ],
        },
        "apis": apis,
        "configurable_interfaces": parse_configurable_interfaces(text),
        "errors": parse_errors(lines),
        "configuration": config,
        "important_requirements": important_requirements_status(text),
        "stm32f103_spl_hints": build_stm32f103_hint_block(apis, config),
        "extraction_stats": {
            "api_count": len(apis),
            "configurable_interface_count": len(parse_configurable_interfaces(text)),
            "ecuc_container_count": len(config["containers"]),
            "ecuc_parameter_count": len(config["parameters"]),
            "requirement_id_count": len(set(re.findall(r"\bSWS_Adc_[A-Za-z0-9_]+\b", text))),
        },
    }

    args.out.parent.mkdir(parents=True, exist_ok=True)
    with args.out.open("w", encoding="utf-8") as f:
        if args.pretty:
            json.dump(output, f, ensure_ascii=False, indent=2)
        else:
            json.dump(output, f, ensure_ascii=False, separators=(",", ":"))

    print(f"Wrote JSON: {args.out}")
    print(f"APIs: {len(apis)} | ECUC params: {len(config['parameters'])} | ECUC containers: {len(config['containers'])}")


if __name__ == "__main__":
    main()
