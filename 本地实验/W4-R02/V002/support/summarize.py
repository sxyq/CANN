#!/usr/bin/env python3
"""Summarize the saved R02 samples without dropping any observations."""

import csv
import json
import re
import statistics
import sys
from decimal import Decimal
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


def timing_groups(rows, field):
    result = stats([float(row[field]) for row in rows])
    groups = [stats([float(row[field]) for row in rows if int(row["block"]) == block])
              for block in range(1, 5)]
    centers = [group["median_us"] for group in groups]
    result["block_statistics"] = groups
    result["block_medians_us"] = centers
    result["block_drift_ratio"] = (max(centers) - min(centers)) / result["median_us"]
    result["within_existing_limits"] = (
        result["mad_over_median"] <= 0.10 and result["block_drift_ratio"] <= 0.10)
    return result


def timing_metric(rows, field):
    slots = {slot: timing_groups([row for row in rows if row["slot"] == slot], field)
             for slot in ("P", "C")}
    pooled = timing_groups(rows, field)
    pairs = []
    for index in range(0, len(rows), 2):
        first, second = rows[index:index + 2]
        pair = {row["slot"]: float(row[field]) for row in (first, second)}
        pairs.append({"order": first["order"], "block": first["block"],
                      "slot_delta_us": pair["C"] - pair["P"],
                      "slot_delta_percent": 100 * (pair["C"] / pair["P"] - 1),
                      "second_minus_first_us": float(second[field]) - float(first[field])})
    return {
        "pooled": pooled, "slots": slots,
        "qualified": pooled["within_existing_limits"] and all(
            value["within_existing_limits"] for value in slots.values()),
        "by_position": {
            str(position): stats([float(row[field]) for row in rows if row["position"] == position])
            for position in (1, 2)},
        "by_slot_and_position": {
            slot: {str(position): stats([float(row[field]) for row in rows
                                        if row["slot"] == slot and row["position"] == position])
                   for position in (1, 2)} for slot in ("P", "C")},
        "slot_median_ratio_delta_percent": 100 * (
            slots["C"]["median_us"] / slots["P"]["median_us"] - 1),
        "paired_slot_delta_us": stats([pair["slot_delta_us"] for pair in pairs]),
        "median_per_pair_slot_delta_percent": statistics.median(
            pair["slot_delta_percent"] for pair in pairs),
        "paired_slot_delta_by_order_us": {
            order: stats([pair["slot_delta_us"] for pair in pairs if pair["order"] == order])
            for order in ("PC", "CP")},
        "paired_slot_delta_by_block_us": [
            statistics.median(pair["slot_delta_us"] for pair in pairs if int(pair["block"]) == block)
            for block in range(1, 5)],
        "second_minus_first_us": stats([pair["second_minus_first_us"] for pair in pairs]),
        "paired_absolute_difference_us": stats([abs(pair["slot_delta_us"]) for pair in pairs]),
    }


def old_order_analysis():
    results = []
    for width in (20480, 18432, 22528):
        rows = read_rows(ROOT / "local" / f"paired-{width}.tsv")
        for index, row in enumerate(rows):
            row["slot"] = "P" if row["side"] == "parent" else "C"
            row["position"] = index % 2 + 1
        metric = timing_metric(rows, "device_event_us")
        results.append({
            "width": width, "source": f"local/paired-{width}.tsv", "timed_calls": len(rows),
            "candidate_guard_two_row_batches": width in (18432, 20480),
            "historical_output_address": "UNKNOWN", "source_output_relation": "SHARED",
            "slot_position_medians_us": {
                slot: {position: value["median_us"] for position, value in positions.items()}
                for slot, positions in metric["by_slot_and_position"].items()},
            "slot_block_medians_us": {
                slot: value["block_medians_us"] for slot, value in metric["slots"].items()},
            "paired_delta_by_order_medians_us": {
                order: value["median_us"] for order, value in metric["paired_slot_delta_by_order_us"].items()},
            "paired_delta_by_block_medians_us": metric["paired_slot_delta_by_block_us"],
            "second_minus_first_median_us": metric["second_minus_first_us"]["median_us"],
        })
    return results


def timing_scope_summary(phase):
    capture = ROOT / "timing-scope-20261008"
    profile = capture / (phase + "-profile")
    op_paths = list(profile.glob("**/op_summary_*.csv"))
    task_paths = list(profile.glob("**/task_time_*.csv"))
    assert len(op_paths) == len(task_paths) == 1
    with op_paths[0].open() as stream:
        operators = list(csv.DictReader(stream))
    with task_paths[0].open() as stream:
        task_rows = list(csv.DictReader(stream))
    operators.sort(key=lambda row: Decimal(row["Task Start Time(us)"].strip()))
    tasks = {(row["Device_id"], row["stream_id"], row["task_id"]): row for row in task_rows}
    assert len(tasks) == len(task_rows)
    assert len(operators) == 258
    assert {row["Device_id"] for row in operators} == {"1"}
    assert {row["Task Type"] for row in operators} == {"AI_VECTOR_CORE"}
    assert {int(row["Block Dim"]) for row in operators} == {40}
    assert len({row["Stream ID"] for row in operators}) == 1
    for op in operators:
        task = tasks[(op["Device_id"], op["Stream ID"], op["Task ID"])]
        assert Decimal(op["Task Duration(us)"]) == Decimal(task["task_time(us)"])
        assert Decimal(op["Task Start Time(us)"].strip()) == Decimal(task["task_start(us)"].strip())

    raw_path = capture / (phase + "-event.tsv")
    raw = read_rows(raw_path)
    metadata = {}
    for line in raw_path.read_text().splitlines():
        if line.startswith("#"):
            metadata.update(re.findall(r"(\w+)=([^\s]+)", line))
    assert metadata["SLOT_P_OUTPUT"] == metadata["SLOT_C_OUTPUT"]
    assert metadata["OUTPUT_RELATION"] == "SHARED"
    assert metadata["SLOT_P_FUNCTION"] == "run_kernel_parent"
    assert metadata["SLOT_C_FUNCTION"] == (
        "run_kernel_parent" if phase == "pp" else "run_kernel_candidate")
    assert (metadata["WARMUPS"], metadata["SAMPLES_PER_BLOCK"], metadata["BLOCKS"]) == ("45", "21", "4")
    expected = []
    for block in range(1, 5):
        for sample in range(1, 22):
            order = "PC" if (block + sample) % 2 == 0 else "CP"
            for slot in order:
                expected.append((block, sample, order, "parent" if slot == "P" else "candidate"))
    assert [(int(row["block"]), int(row["sample"]), row["order"], row["side"])
            for row in raw] == expected
    assert {row["width"] for row in raw} == {"20480"}
    assert {row["dtype_id"] for row in raw} == {"1"}
    mapped = []
    for index, (sample, op) in enumerate(zip(raw, operators[90:])):
        slot = "P" if sample["side"] == "parent" else "C"
        expected_source = "R31B-V011" if phase == "pp" or slot == "P" else "W4-R02-V002"
        assert sample["source_id"] == expected_source
        key = (op["Device_id"], op["Stream ID"], op["Task ID"])
        task = tasks[key]
        start = tasks[(key[0], key[1], str(int(key[2]) - 1))]
        stop = tasks[(key[0], key[1], str(int(key[2]) + 1))]
        assert start["kernel_type"] == stop["kernel_type"] == "EVENT_RECORD"
        event_start = Decimal(start["task_start(us)"].strip())
        event_stop = Decimal(stop["task_start(us)"].strip())
        task_start = Decimal(task["task_start(us)"].strip())
        task_stop = Decimal(task["task_stop(us)"].strip())
        assert event_start <= task_start <= task_stop <= event_stop
        event_us = float(sample["device_event_us"])
        task_us = float(task["task_time(us)"])
        mapped.append({
            "block": int(sample["block"]), "sample": int(sample["sample"]),
            "order": sample["order"], "slot": slot, "position": index % 2 + 1,
            "source_id": sample["source_id"], "function": metadata["SLOT_" + slot + "_FUNCTION"],
            "output_address": metadata["SLOT_" + slot + "_OUTPUT"],
            "launch_ordinal": index + 91, "timed_ordinal": index + 1,
            "device_id": key[0], "stream_id": key[1], "task_id": key[2], "block_dim": 40,
            "task_start_us": str(task_start), "task_stop_us": str(task_stop),
            "device_event_us": event_us, "wall_us": float(sample["wall_us"]), "kernel_task_us": task_us,
            "start_record_task_id": start["task_id"], "stop_record_task_id": stop["task_id"],
            "start_record_us": str(event_start), "stop_record_us": str(event_stop),
            "trace_event_us": float(event_stop - event_start),
            "trace_minus_acl_event_us": float(event_stop - event_start) - event_us,
            "start_to_kernel_us": float(task_start - event_start),
            "kernel_to_stop_us": float(event_stop - task_stop),
            "event_minus_kernel_us": event_us - task_us,
        })
    assert len(mapped) == 168
    metrics = {name: timing_metric(mapped, field) for name, field in (
        ("event", "device_event_us"), ("kernel_task", "kernel_task_us"))}
    qualified = all(metric["qualified"] for metric in metrics.values())
    references = []
    log = (capture / (phase + "-profile.log")).read_text()
    if phase == "pp":
        for line in log.splitlines():
            if line.startswith("parent\tR31B-V011\t"):
                fields = line.split("\t")
                assert len(fields) == 11 and fields[8:11] == ["0", "0", "PASS"]
                references.append({"scope": "shared output after all timed calls", "source": fields[1],
                                   "rows": int(fields[3]), "width": int(fields[4]), "dtype": fields[5],
                                   "max_abs_error": float(fields[6]), "atol": float(fields[7]),
                                   "rtol": float(fields[7]), "mismatches": 0, "nonfinite": 0, "status": "PASS"})
        assert len(references) == 1
    resource = {}
    for path in sorted(capture.glob("*.usages.txt")):
        contents = path.read_text()
        def value(name):
            return int(re.search(re.escape(name) + r"\s*:\s*(\d+)", contents)[1])
        label = path.name.removesuffix(".usages.txt")
        capacity, rate = value("HBM Capacity(MB)"), value("HBM Usage Rate(%)")
        resource[label] = {
            "device": 1, "hbm_capacity_mb": capacity, "hbm_used_percent": rate,
            "free_hbm_mb_from_usages": capacity * (100 - rate) // 100,
            "aicore_percent": value("Aicore Usage Rate(%)"), "aivector_percent": value("Aivector Usage Rate(%)"),
            "host_load": (capture / (label + ".load.txt")).read_text().strip(),
            "time_utc": (capture / (label + ".time.txt")).read_text().strip(),
        }
    result = {
        "route": "W4-R02", "revision": "V002", "direct_parent": "R31B-V011", "phase": phase,
        "new_performance_revisions": 0, "current_local_best": None,
        "candidate_measured": phase == "pc", "qualification": "PASS" if qualified else "MEASUREMENT_BLOCKED",
        "local_score_us": None if phase == "pp" else metrics["event"]["slots"]["C"]["median_us"],
        "local_delta_percent": None if phase == "pp" else metrics["event"]["slot_median_ratio_delta_percent"],
        "official_score": None, "online": "PAUSED", "push": "NO", "samples_omitted": 0,
        "protocol": {"rows": 128, "width": 20480, "dtype": "fp16", "warmups_per_slot": 45,
                     "blocks": 4, "pairs_per_block": 21, "samples_per_slot": 84,
                     "mad_over_median_limit": 0.10, "block_drift_ratio_limit": 0.10,
                     "pp_requires_both_time_scopes_and_all_slots": True},
        "counts": {"op_summary_rows": len(operators), "task_time_rows": len(task_rows),
                   "warmup_calls": 90, "timed_calls": len(mapped), "new_reference_kernel_calls": 0,
                   "all_op_task_pairs_match": True, "all_timed_calls_have_event_brackets": True},
        "metadata": metadata, "reference_results": references, "metrics": metrics,
        "timing_intervals": {field: stats([row[field] for row in mapped]) for field in (
            "wall_us", "event_minus_kernel_us", "start_to_kernel_us", "kernel_to_stop_us", "trace_minus_acl_event_us")},
        "max_abs_trace_minus_acl_event_us": max(abs(row["trace_minus_acl_event_us"]) for row in mapped),
        "first_timed_sample": mapped[0],
        "largest_kernel_samples": sorted(mapped, key=lambda row: row["kernel_task_us"], reverse=True)[:5],
        "resource_context": resource, "old_pc_order_analysis": old_order_analysis(),
        "source_files": {"raw": str(raw_path.relative_to(ROOT)),
                         "op_summary": str(op_paths[0].relative_to(ROOT)),
                         "task_time": str(task_paths[0].relative_to(ROOT))},
        "units": "Times in us; percent fields in percent; CV/MAD/drift in ratios. In pp, P/C are slots calling Parent.",
    }
    with (capture / (phase + "-task-map.tsv")).open("x", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(mapped[0]), delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(mapped)
    return result


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "--timing-scope" and sys.argv[2] in ("pp", "pc"):
        print(json.dumps(timing_scope_summary(sys.argv[2]), ensure_ascii=False, indent=2))
        return
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
