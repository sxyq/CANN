#!/usr/bin/env python3
"""Summarize Parent-only counters and P/P device-event samples."""

import csv
import json
import math
import re
import statistics
from pathlib import Path


ROOT = Path(__file__).resolve().parent


def quantile(values, fraction):
    ordered = sorted(values)
    pos = (len(ordered) - 1) * fraction
    left = math.floor(pos)
    right = math.ceil(pos)
    return ordered[left] + (ordered[right] - ordered[left]) * (pos - left)


def stats(values):
    med = statistics.median(values)
    mean = statistics.mean(values)
    return {
        "n": len(values),
        "median_us": med,
        "mean_us": mean,
        "mad_us": statistics.median(abs(x - med) for x in values),
        "stdev_us": statistics.stdev(values),
        "cv": statistics.stdev(values) / mean,
        "p10_us": quantile(values, 0.1),
        "p90_us": quantile(values, 0.9),
        "min_us": min(values),
        "max_us": max(values),
    }


def read_csv(path, delimiter=","):
    with path.open() as stream:
        return list(csv.DictReader(stream, delimiter=delimiter))


same_parent = {}
for path in sorted((ROOT / "results").glob("same_*.tsv")):
    data = read_csv(path, "\t")
    if not data:
        continue
    first = [float(row["p1_us"]) for row in data]
    second = [float(row["p2_us"]) for row in data]
    combined = first + second
    deltas = [b - a for a, b in zip(first, second)]
    block_medians = []
    for block in sorted({row["block"] for row in data}):
        block_medians.append(statistics.median(
            float(row[side]) for row in data if row["block"] == block
            for side in ("p1_us", "p2_us")
        ))
    drift = max(block_medians) - min(block_medians)
    floor = max(quantile([abs(value) for value in deltas], 0.9), drift)
    merged_stats = stats(combined)
    same_parent[path.stem] = {
        "source": str(path.relative_to(ROOT)),
        "p1": stats(first),
        "p2": stats(second),
        "all": merged_stats,
        "pair_delta_median_us": statistics.median(deltas),
        "block_medians_us": block_medians,
        "block_drift_us": drift,
        "measurement_floor_us": floor,
        "floor_definition": "max(p90(abs(P2-P1)), range(block medians)); no discarded samples",
        "shape_repeatable": merged_stats["mad_us"] / merged_stats["median_us"] <= 0.1
        and drift / merged_stats["median_us"] <= 0.1,
    }


profiles = []
for path in sorted(ROOT.glob("**/OpBasicInfo.csv")):
    case = re.search(r"profile_(\d+)x(\d+)_(fp32|fp16)(.*?)/OPPROF_", str(path))
    if case is None or not (path.parent / "Memory.csv").exists():
        continue
    rows, width = int(case[1]), int(case[2])
    dtype = case[3]
    basic = read_csv(path)[0]
    pipes = read_csv(path.parent / "PipeUtilization.csv")
    memory = read_csv(path.parent / "Memory.csv")
    core_count = int(basic["Block Dim"])
    assert rows % core_count == 0
    local_rows = rows // core_count
    tiles = math.ceil(width / 4096)
    param_commands = 2 * tiles
    input_commands = 2 * tiles * local_rows
    param_kib = width * (4 if dtype == "fp32" else 2) * 2 / 1024
    times = [float(row["aiv_mte2_time(us)"]) for row in pipes]
    counts = [int(row["aiv_mte2_instructions"]) for row in memory]
    fractions = [param_kib / float(row["GM_to_UB_datas(KB)"]) for row in memory]
    param_proxy = statistics.mean(time * fraction for time, fraction in zip(times, fractions))
    mergeable = dtype == "fp32" and 4096 < width <= 8192 and local_rows > 1
    saving_proxy = param_proxy * 0.5 if mergeable else 0.0
    cores_arg = 1 if "singlecore" in case[4] else 0
    noise = same_parent.get(f"same_{rows}x{width}_c{cores_arg}")
    signal = "UNPROVEN"
    if not mergeable:
        signal = "NO_SITE_ACTIVE"
    elif noise is not None and saving_proxy <= noise["measurement_floor_us"]:
        signal = "NO_PROXY_BELOW_FLOOR"
    profiles.append({
        "source": str(path.parent.relative_to(ROOT)),
        "origin": "PRIOR_SUPPORT" if "prior-profile" in path.parts else "THIS_RUN",
        "shape": [rows, width], "dtype": dtype,
        "block_count": core_count, "local_rows_per_core": local_rows,
        "param_mte2_command_count_per_core_source": param_commands,
        "param_mte2_command_count_launch_source": param_commands * core_count,
        "total_mte2_instructions_per_core_measured": counts,
        "unattributed_instructions_per_core": [n - param_commands - input_commands for n in counts],
        "mte2_time_per_core_us": times,
        "param_mte2_time_proxy_mean_us": param_proxy,
        "proxy_definition": "MTE2 active time * parameter bytes / measured GM-to-UB bytes; no tensor-specific timestamps",
        "total_kernel_time_us": float(basic["Task Duration(us)"]),
        "merge_site_active": mergeable,
        "removable_parameter_commands_per_core": 2 if mergeable else 0,
        "expected_saving_measured_us": None,
        "optimistic_command_share_saving_proxy_us": saving_proxy,
        "saving_proxy_limit": "Half the parameter-time proxy; bytes stay constant, issue overhead and overlap are not isolated",
        "measurement_floor_us": noise["measurement_floor_us"] if noise else None,
        "signal_above_noise": signal,
    })

result = {"route": "W4-R14", "candidate_written": False, "same_parent": same_parent, "profiles": profiles}
print(json.dumps(result, indent=2, ensure_ascii=False))
