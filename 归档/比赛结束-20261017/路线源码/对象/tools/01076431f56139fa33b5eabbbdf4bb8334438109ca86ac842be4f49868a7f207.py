#!/usr/bin/env python3
"""Summarize the completed R01 requalification without running NPU work."""

import csv
from datetime import datetime
import json
from pathlib import Path
import re
import statistics

from recompute_local_result import mad_over_median, read_block


def stats(values):
    middle = statistics.median(values)
    mean = statistics.mean(values)
    return {
        "count": len(values), "median_us": middle, "mean_us": mean,
        "stdev_us": statistics.stdev(values), "cv": statistics.stdev(values) / mean,
        "mad_over_median": mad_over_median(values), "min_us": min(values), "max_us": max(values),
    }


def context(path):
    text = path.read_text()
    capacity = int(re.search(r"HBM Capacity\(MB\)\s*:\s*(\d+)", text)[1])
    used = int(re.search(r"HBM Usage Rate\(%\)\s*:\s*(\d+)", text)[1])
    return {
        "file": path.name, "utc": text.splitlines()[0],
        "free_hbm_mb": capacity * (100 - used) // 100,
        "aicore_percent": int(re.search(r"Aicore Usage Rate\(%\)\s*:\s*(\d+)", text)[1]),
        "aivector_percent": int(re.search(r"Aivector Usage Rate\(%\)\s*:\s*(\d+)", text)[1]),
        "loadavg": next(line for line in text.splitlines() if re.match(r"^\d+\.\d+ ", line)),
    }


def summarize(root, prefix, second_label):
    groups = []
    pooled_a, pooled_b, pooled_delta = [], [], []
    group_medians = []
    for number in (1, 2, 3):
        path = root / f"{prefix}-b{number}.tsv"
        a, b, delta = read_block(path)
        with path.open() as handle:
            rows = list(csv.DictReader((line for line in handle if not line.startswith("#")), delimiter="\t"))
        pooled_a.extend(a)
        pooled_b.extend(b)
        pooled_delta.extend(delta)
        am, bm = statistics.median(a), statistics.median(b)
        group_medians.extend([am, bm])
        groups.append({
            "block": number, "raw_file": path.name,
            "first_label": "Parent", "second_label": second_label,
            "first": stats(a), "second": stats(b),
            "side_median_delta_percent": (bm / am - 1) * 100,
            "median_paired_delta_us": statistics.median(delta),
            "pc_count": sum(row["order"] == "PC" for row in rows),
            "cp_count": sum(row["order"] == "CP" for row in rows),
            "first_wall": stats([float(row["parent_wall_us"]) for row in rows]),
            "second_wall": stats([float(row["candidate_wall_us"]) for row in rows]),
            "pre": context(path.with_suffix(".pre.txt")),
            "post": context(path.with_suffix(".post.txt")),
        })
    center = statistics.median(pooled_a + pooled_b)
    drift = max(abs(x - center) for x in group_medians) / center
    stable_groups = sum(
        group["first"]["mad_over_median"] <= 0.10 and group["second"]["mad_over_median"] <= 0.10
        for group in groups
    )
    return {
        "groups": groups, "pooled_first": stats(pooled_a), "pooled_second": stats(pooled_b),
        "pooled_side_median_delta_percent": (statistics.median(pooled_b) / statistics.median(pooled_a) - 1) * 100,
        "pooled_median_paired_delta_us": statistics.median(pooled_delta),
        "block_drift_over_joint_median": drift,
        "block_span_over_joint_median": (max(group_medians) - min(group_medians)) / center,
        "mad_over_median_limit": 0.10, "block_drift_limit": 0.10,
        "blocks_with_both_sides_within_mad_limit": stable_groups,
        "within_spread_limits": stable_groups == 3 and drift <= 0.10,
    }


def reference_results(path):
    references = []
    paired = None
    for line in path.read_text().splitlines():
        if line.startswith("REFERENCE ") or line.startswith("CORRECTNESS "):
            fields = dict(word.split("=", 1) for word in line.split()[1:])
            if line.startswith("REFERENCE "):
                references.append({
                    "label": fields["label"], "result": fields["result"],
                    "strict_all_elements": fields["strict_all_elements"],
                    "element_count": int(fields["elements"]),
                    "tolerance_failures": int(fields["tolerance_failures"]),
                    "nonfinite": int(fields["nonfinite"]),
                    "max_abs_error": float(fields["max_abs"]),
                    "mismatch_fraction": float(fields["mismatch_fraction"]),
                })
            else:
                paired = {"result": fields["result"], "bit_differences": int(fields["bit_differences"])}
    return {"evidence": path.name, "reference_results": references, "parent_vs_candidate": paired}


def main():
    root = Path(__file__).resolve().parents[1] / "requal-device2-20261008"
    same_function = summarize(root, "samefunction-d2-r16-d16384", "same Parent function")
    paired = {
        "target_guard_on": {"shape": [16, 16384], "guard": "ON", **summarize(root, "paired-d2-r16-d16384", "Candidate")},
        "control_guard_off": {"shape": [16, 32768], "guard": "OFF", **summarize(root, "paired-d2-r16-d32768", "Candidate")},
        "boundary_guard_on": {"shape": [16, 16352], "guard": "ON", **summarize(root, "paired-d2-r16-d16352", "Candidate")},
    }
    refs = [reference_results(root / f"correctness-template-r{rows}-d{width}.log")
            for rows, width in [(16, 16384), (8, 16384), (16, 32768), (16, 16352)]]
    target = paired["target_guard_on"]
    parse_time = lambda text: datetime.strptime(text, "%Y-%m-%dT%H:%M:%SZ")
    pp_end = parse_time(same_function["groups"][-1]["post"]["utc"])
    pc_start = parse_time(target["groups"][0]["pre"]["utc"])
    result = {
        "route": "W4-R01", "revision": "V001", "new_performance_revision": False,
        "source_commit": "c8a1577ddcb53dd3f67048d09117a865480af359",
        "rule_source_commit": "9f91895506023d917637f707bb3f61cd9d9f8765",
        "worktree": str(Path(__file__).resolve().parents[4]),
        "direct_parent": "R31B/V011", "branch": "w4/r01-selective-param-pipeline-x",
        "status": "MEASUREMENT_BLOCKED", "strict_reference_status": "FAIL",
        "candidate_source_changed": False, "parent_source_changed": False,
        "compile": {
            "existing_kernel_libraries": "REUSED_PASS",
            "host_runner": "PASS",
            "cmake_attempt": "FAIL: unintended ASC object rebuild; vector header unavailable",
            "logs": ["compile-runner.log", "compile-runner-host.log", "compile-parent-diagnostic.log", "compile-template-reference.log"],
        },
        "correctness": {
            "dtype": "fp16", "blocks": 8,
            "parent_vs_candidate": "PASS_BITWISE_IDENTICAL",
            "parent_vs_reference": "PASS_TEMPLATE", "candidate_vs_reference": "PASS_TEMPLATE",
            "reference": "CPU FP64 problem formula, final FP16; independent NumPy FP32 confirms the failed element",
            "atol": 0.001, "rtol": 0.001, "allowed_mismatch_fraction": 0.001,
            "criterion_source": "归档/历史工作区/WIDE-X/scripts/verify_result.py:case_output_specs",
            "initial_supplementary_criterion": "required_match_fraction=1.0; failed and initially stopped P/C timing",
            "criterion_alignment": "The existing template permits 0.1% unmatched elements. The extra full-agreement criterion was removed from admission, with strict counts and its original failure log retained. Absolute and relative tolerances stayed at 1e-3.",
            "strict_counts_per_side": {"16x16384": 1, "8x16384": 0, "16x32768": 3, "16x16352": 0},
            "not_an_official_verdict": True,
            "initial_evidence": ["correctness-r16-d16384.log", "reference-probe.json"],
            "shapes": refs,
        },
        "local_score": target["pooled_second"]["median_us"], "local_score_unit": "us",
        "local_delta_percent": target["pooled_side_median_delta_percent"],
        "local_accepted": False,
        "parent_pc_median_us": target["pooled_first"]["median_us"],
        "candidate_pc_median_us": target["pooled_second"]["median_us"],
        "candidate_performance_sample_count": sum(item["pooled_second"]["count"] for item in paired.values()),
        "target_candidate_sample_count": target["pooled_second"]["count"],
        "paired_results": paired,
        "current_local_best": "R31B/V011 (carried forward; V001 not promoted)",
        "same_function_diagnostic": {
            "candidate_dispatched": False, "not_a_candidate_score": True,
            "shape": [16, 16384], "dtype": "fp16", "device": 2, "blocks": 8,
            "epsilon": 1e-5, "warmups": 45, "pairs_per_block": 42, "block_count": 3,
            "launches_per_event": 1, "order": "alternating PC/CP, 21 each per block",
            "comparison": "same loaded Parent function, separate output buffers",
            **same_function,
            "qualified": same_function["within_spread_limits"],
        },
        "pp_to_pc_gap_seconds": (pc_start - pp_end).total_seconds(),
        "timing_window_note": "P/P was retained before P/C. Host-only reference protocol alignment separated the phases; they were not back-to-back. No statistical gain attribution is claimed.",
        "load_note": "Other VLLM processes remained present. Load is recorded as context only; device 2 free HBM was sufficient throughout.",
        "old_device0_evidence": "../support/requal-20261008/ (recovered; not rerun or pooled)",
        "old_v001_delta_percent": -5.9979, "old_v001_delta_accepted": False,
        "official_score": None, "online_state": "PAUSED", "push": "NO",
        "new_performance_versions": 0, "valid_local_results": 0, "stagnation_3_increment": 0,
        "running_device_operations": "NONE", "process_evidence": "paired-window-end.txt",
        "next_action": "Use an R01 Parent-only task-duration versus device-event diagnostic to identify the timing noise source; reuse the completed four-shape template reference results. No V002 until timing attribution is credible.",
    }
    print(json.dumps(result, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
