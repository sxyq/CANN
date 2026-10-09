#!/usr/bin/env python3
"""Summarize every retained V002 sample without filtering observations."""

import csv
import json
import math
import re
import statistics
from pathlib import Path


VERSION = Path(__file__).resolve().parents[1]
LOGS = VERSION / "logs"


def only(pattern):
    paths = sorted(LOGS.glob(pattern))
    assert len(paths) == 1, (pattern, paths)
    return paths[0]


def stats(values):
    median = statistics.median(values)
    mean = statistics.mean(values)
    mad = statistics.median(abs(value - median) for value in values)
    return {
        "count": len(values), "median_us": median, "mean_us": mean,
        "min_us": min(values), "max_us": max(values), "mad_us": mad,
        "mad_over_median": mad / median,
        "population_cv": statistics.pstdev(values) / mean,
    }


def phase(name):
    path = only(name + "-*-raw.tsv")
    rows = list(csv.DictReader(path.open(), delimiter="\t"))
    cases = {}
    for row in rows:
        cases.setdefault(row["case"], []).append(row)
    assert len(cases) == 3
    result = {"raw_path": str(path.relative_to(VERSION)), "cases": {}}
    for case, data in cases.items():
        assert len(data) == 44
        assert {(int(row["block"]), int(row["pair"])) for row in data} == {
            (block, pair) for block in range(4) for pair in range(11)
        }
        parent = [float(row["parent_device_us"]) for row in data]
        candidate = [float(row["candidate_device_us"]) for row in data]
        assert all(value > 0 and math.isfinite(value) for value in parent + candidate)
        for row, p, c in zip(data, parent, candidate):
            expected_order = "P-C" if (int(row["block"]) + int(row["pair"])) % 2 == 0 else "C-P"
            assert row["order"] == expected_order
            assert abs((c / p - 1) * 100 - float(row["delta_pct"])) < 1e-6
        pstats, cstats = stats(parent), stats(candidate)
        ratio_delta = (cstats["median_us"] / pstats["median_us"] - 1) * 100
        block_ratios = []
        for block in range(4):
            selected = [row for row in data if int(row["block"]) == block]
            p = statistics.median(float(row["parent_device_us"]) for row in selected)
            c = statistics.median(float(row["candidate_device_us"]) for row in selected)
            block_ratios.append((c / p - 1) * 100)
        result["cases"][case] = {
            "parent": pstats, "candidate": cstats, "median_ratio_delta_pct": ratio_delta,
            "paired_delta_us_median": statistics.median(c - p for p, c in zip(parent, candidate)),
            "paired_delta_pct_median": statistics.median((c / p - 1) * 100 for p, c in zip(parent, candidate)),
            "block_median_ratio_delta_pct": block_ratios,
            "dispersion_within_declared_limit": max(pstats["mad_over_median"], cstats["mad_over_median"]) <= 0.05,
        }
    return result


def main():
    same, local = phase("same-binary"), phase("local")
    assert set(same["cases"]) == set(local["cases"])
    same_ok = all(abs(case["median_ratio_delta_pct"]) <= 2 and
                  case["dispersion_within_declared_limit"] for case in same["cases"].values())
    local_ok = all(case["dispersion_within_declared_limit"] for case in local["cases"].values())
    targets = [case for name, case in local["cases"].items() if name.startswith("target-")]
    assert len(targets) == 2
    score = math.exp(statistics.mean(math.log(case["parent"]["median_us"] /
                                            case["candidate"]["median_us"]) for case in targets))
    correctness_path = only("correctness-[0-9]*.log")
    correctness = [dict(re.findall(r"(\w+)=([^\s]+)", line))
                   for line in correctness_path.read_text().splitlines() if line.startswith("CORRECTNESS ")]
    assert len(correctness) == 9
    assert all(row["parent_failures"] == row["candidate_failures"] == row["pair_bitwise_mismatches"] == "0"
               for row in correctness)
    print(json.dumps({
        "route": "W4-R04", "revision": "V002", "direct_parent": "R31B-V011",
        "same_binary": same, "local": local,
        "correctness": {"path": str(correctness_path.relative_to(VERSION)), "cases": correctness, "verdict": "PASS"},
        "local_score_observed": score, "local_delta_pct_observed": (1 / score - 1) * 100,
        "same_binary_valid": same_ok, "local_dispersion_valid": local_ok,
        "status": "NUMERIC_LOCAL_AVAILABLE" if same_ok and local_ok else "MEASUREMENT_BLOCKED",
        "new_performance_revisions": 1, "valid_local_results": int(same_ok and local_ok),
        "retained_pairs": 264, "retained_side_samples": 528, "raw_filtering": "NONE",
        "official_score": None, "online": "PAUSED", "push": "NO",
    }, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
