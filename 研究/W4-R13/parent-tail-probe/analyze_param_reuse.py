#!/usr/bin/env python3
"""Evaluate the parameter-reuse diagnosis using the saved FP64 reference."""
import json
from pathlib import Path

from analyze_outputs import analyze


def main():
    probe = Path(__file__).resolve().parent
    evidence = probe / "evidence" / "param-reuse-01"
    previous = json.loads((probe / "evidence" / "reference-analysis.json").read_text())
    previous_cases = {item["width"]: item for item in previous["results"]}
    results = []
    for width in (8192, 9216, 10240):
        diagnosis = analyze(evidence / f"diag-1x{width}", 1, width)
        item = {"shape": f"1x{width}", "dtype": "fp32",
                "previous_parent": previous_cases[width], "diagnosis": diagnosis}
        if width > 8192:
            item["same_device_parent"] = analyze(evidence / f"parent-current-1x{width}", 1, width)
        else:
            old_output = probe / "evidence" / "parent-1x8192-run02-output.bin"
            new_output = evidence / "diag-1x8192-output.bin"
            item["control_bit_equal_to_saved_parent"] = old_output.read_bytes() == new_output.read_bytes()
        results.append(item)
    report = {
        "route": "W4-R13", "variant": "DIAG-PARAM-REUSE-01",
        "classification": "RESEARCH_EXECUTED_VARIANT",
        "parent": "R31B-V011", "device": 1,
        "change": "SyncVToMTE2 before gamma/bias Load when tile > 0",
        "reference": "independent FP64; existing FP32-add reference also retained",
        "atol": 1.0e-4, "rtol": 1.0e-4, "results": results,
        "diagnosis_pass": all(item["diagnosis"]["pass"] for item in results),
        "local_score": None, "local_delta": None,
        "timing_use": "single-launch diagnosis only",
        "new_performance_revisions": 0, "valid_local_results": 0,
        "official": "NONE", "online": "PAUSED",
    }
    print(json.dumps(report, ensure_ascii=False, indent=2))
    return 0 if report["diagnosis_pass"] else 3


if __name__ == "__main__":
    raise SystemExit(main())
