#!/usr/bin/env python3
"""Summarize the saved R02 samples without dropping any observations."""

import csv
import json
import re
import statistics
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent


def read_rows(path):
    return list(csv.DictReader(
        (line for line in path.read_text().splitlines() if not line.startswith("#")),
        delimiter="\t"))


def percentile(values, fraction):
    ordered = sorted(values)
    index = (len(ordered) - 1) * fraction
    lo = int(index)
    hi = min(lo + 1, len(ordered) - 1)
    return ordered[lo] + (ordered[hi] - ordered[lo]) * (index - lo)


def stats(values):
    median = statistics.median(values)
    mean = statistics.mean(values)
    stdev = statistics.stdev(values)
    mad = statistics.median(abs(value - median) for value in values)
    return {
        "n": len(values), "median_us": median, "mean_us": mean,
        "stdev_us": stdev, "cv": stdev / abs(mean) if mean else None,
        "mad_us": mad, "mad_over_median": mad / abs(median) if median else None,
        "min_us": min(values), "max_us": max(values),
        "p10_us": percentile(values, 0.1), "p90_us": percentile(values, 0.9),
        "p95_us": percentile(values, 0.95),
    }


def side_stats(rows):
    device = stats([float(row["device_event_us"]) for row in rows])
    blocks = [statistics.median(float(row["device_event_us"]) for row in rows
                                if int(row["block"]) == block)
              for block in range(1, 5)]
    device["block_medians_us"] = blocks
    device["block_drift_ratio"] = (max(blocks) - min(blocks)) / device["median_us"]
    return {"device": device, "wall": stats([float(row["wall_us"]) for row in rows])}


def shape_result(width):
    same_path = ROOT / "local" / f"same-parent-{width}.tsv"
    paired_path = ROOT / "local" / f"paired-{width}.tsv"
    same_rows = read_rows(same_path)
    paired_rows = read_rows(paired_path)
    parent_rows = [row for row in paired_rows if row["side"] == "parent"]
    candidate_rows = [row for row in paired_rows if row["side"] == "candidate"]
    assert len(same_rows) == len(parent_rows) == len(candidate_rows) == 84
    same = side_stats(same_rows)
    parent = side_stats(parent_rows)
    candidate = side_stats(candidate_rows)
    pairs = {}
    for row in paired_rows:
        pair = pairs.setdefault((int(row["block"]), int(row["sample"])), {})
        pair[row["side"]] = float(row["device_event_us"])
        pair["order"] = row["order"]
    assert len(pairs) == 84 and all("parent" in p and "candidate" in p for p in pairs.values())
    deltas = [pair["candidate"] - pair["parent"] for pair in pairs.values()]
    parent_median = parent["device"]["median_us"]
    candidate_median = candidate["device"]["median_us"]
    quality_reasons = []
    for label, result in [("same_parent", same), ("paired_parent", parent), ("paired_candidate", candidate)]:
        for metric in ["mad_over_median", "block_drift_ratio"]:
            if result["device"][metric] > 0.10:
                quality_reasons.append(f"{label}.{metric}={result['device'][metric]:.9f}>0.10")
    return {
        "shape": [128, width], "dtype": "fp16", "available_cores": 40,
        "block_count": 40, "tile_width": 4096, "tile_count": (width + 4095) // 4096,
        "batch_limit": 2, "changed_two_row_batches": width in (18432, 20480),
        "same_parent": same, "parent": parent, "candidate": candidate,
        "local_score_us": candidate_median,
        "local_delta_percent": 100 * (candidate_median / parent_median - 1),
        "paired_delta_us": stats(deltas),
        "paired_delta_percent_of_parent_median": 100 * statistics.median(deltas) / parent_median,
        "median_per_pair_delta_percent": statistics.median(
            100 * (p["candidate"] / p["parent"] - 1) for p in pairs.values()),
        "negative_pairs": sum(delta < 0 for delta in deltas),
        "zero_pairs": sum(delta == 0 for delta in deltas),
        "positive_pairs": sum(delta > 0 for delta in deltas),
        "paired_block_medians_us": [
            statistics.median(pair["candidate"] - pair["parent"]
                              for key, pair in pairs.items() if key[0] == block)
            for block in range(1, 5)],
        "paired_order_medians_us": {
            order: statistics.median(pair["candidate"] - pair["parent"]
                                     for pair in pairs.values() if pair["order"] == order)
            for order in ("PC", "CP")},
        "measurement_usable": not quality_reasons,
        "quality_reasons": quality_reasons,
        "raw_same_parent": str(same_path.relative_to(ROOT)),
        "raw_parent_and_candidate": str(paired_path.relative_to(ROOT)),
        "samples_omitted": 0,
    }


def environment(label):
    usage = (ROOT / "logs" / f"{label}.usages.txt").read_text()
    def field(name):
        return int(re.search(re.escape(name) + r"\s*:\s*(\d+)", usage).group(1))
    capacity = field("HBM Capacity(MB)")
    rate = field("HBM Usage Rate(%)")
    return {
        "device": 1, "free_hbm_mb": capacity * (100 - rate) // 100,
        "hbm_capacity_mb": capacity, "hbm_used_percent": rate,
        "aicore_percent": field("Aicore Usage Rate(%)"),
        "aivector_percent": field("Aivector Usage Rate(%)"),
        "host_load": (ROOT / "logs" / f"{label}.load.txt").read_text().strip(),
        "usage_evidence": f"logs/{label}.usages.txt",
    }


def main():
    shapes = [shape_result(width) for width in (20480, 18432, 22528)]
    correctness = []
    for width in (18432, 20480, 22528, 24576):
        for side in ("parent", "candidate"):
            path = ROOT / "logs" / f"correctness-{width}-{side}.tsv"
            row, = read_rows(path)
            assert row["status"] == "PASS" and row["mismatches"] == "0"
            correctness.append({
                "shape": [128, width], "dtype": "fp16", "side": side,
                "max_abs_error": float(row["max_abs_error"]),
                "atol": float(row["atol"]), "rtol": float(row["atol"]),
                "mismatches": int(row["mismatches"]), "nonfinite": int(row["nonfinite"]),
                "status": row["status"], "evidence": str(path.relative_to(ROOT)),
            })
    result = {
        "route": "W4-R02", "revision": "V002", "direct_parent": "R31B-V011",
        "branch": "w4/r02-selective-tile-traversal-x",
        "single_change": "FP16 pass-1 tile-major only for tileWidth=4096, tileCount=5, batchRows=2",
        "compile": "PASS", "correctness": "PASS",
        "reference": "Independent CPU formula, double RMS accumulation, dtype-rounded output; every element required",
        "correctness_cases": correctness,
        "local_status": "MEASUREMENT_BLOCKED" if any(not s["measurement_usable"] for s in shapes) else "MEASURED",
        "local_score_us": shapes[0]["local_score_us"],
        "local_delta_percent": shapes[0]["local_delta_percent"],
        "current_local_best": None, "retained_parent": "R31B-V011",
        "promoted": False, "stagnation_increment": 0,
        "official_score": None, "online": "PAUSED", "push": "NO",
        "protocol": {
            "warmups": 45, "samples_per_block": 21, "blocks": 4,
            "same_parent_samples_per_shape": 84, "paired_samples_per_side_per_shape": 84,
            "pair_order": "alternating PC/CP", "all_raw_timing_rows": 756,
            "mad_over_median_limit": 0.10, "block_drift_ratio_limit": 0.10,
        },
        "shapes": shapes,
        "environment_pre": environment("local-pre"),
        "environment_post": environment("local-post"),
        "run_start_utc": "2026-10-08T03:47:35Z", "run_end_utc": "2026-10-08T03:48:54Z",
        "run_rc": 0, "running_device_operation": "NONE",
    }
    print(json.dumps(result, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
