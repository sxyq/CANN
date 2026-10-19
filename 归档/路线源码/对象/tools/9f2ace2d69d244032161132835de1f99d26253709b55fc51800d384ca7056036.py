#!/usr/bin/env python3
"""Summarize the retained R11 V002 device-1 measurements without running a device."""

import argparse
import csv
import json
import math
import re
import statistics
from decimal import Decimal
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CAPTURE = ROOT / "results/requal-device1-20261008-task"
CASES = {"target-fp32-64x8192": 40, "control-fp32-12x8192": 12}


def read_rows(path, delimiter=","):
    with path.open(newline="") as stream:
        return list(csv.DictReader(stream, delimiter=delimiter))


def percentile(values, fraction):
    ordered = sorted(values)
    index = (len(ordered) - 1) * fraction
    low = math.floor(index)
    high = math.ceil(index)
    return ordered[low] + (ordered[high] - ordered[low]) * (index - low)


def describe(values):
    center = statistics.median(values)
    mean = statistics.mean(values)
    deviation = statistics.pstdev(values)
    mad = statistics.median(abs(value - center) for value in values)
    return {
        "n": len(values), "median": center, "mean": mean,
        "stdev": deviation, "cv": deviation / abs(mean) if mean else None,
        "mad": mad, "mad_over_median": mad / abs(center) if center else None,
        "min": min(values), "max": max(values),
        "p10": percentile(values, 0.1), "p90": percentile(values, 0.9),
    }


def describe_blocks(rows, field):
    values = [float(row[field]) for row in rows]
    result = describe(values)
    blocks = {
        block: describe([float(row[field]) for row in rows if row["block"] == block])
        for block in dict.fromkeys(row["block"] for row in rows)
    }
    centers = [block["median"] for block in blocks.values()]
    drift = (max(centers) - min(centers)) / result["median"]
    result.update({"blocks": blocks, "block_median_relative_range": drift,
                   "within_existing_limits": result["mad_over_median"] <= 0.10 and drift <= 0.10})
    return result


def summarize(rows, parent_field, candidate_field=None):
    result = {}
    for case in CASES:
        group = [row for row in rows if row["case"] == case]
        parent = describe_blocks(group, parent_field)
        entry = {"parent_us": parent}
        if candidate_field:
            candidate = describe_blocks(group, candidate_field)
            differences = [float(row[candidate_field]) - float(row[parent_field]) for row in group]
            deltas = [100 * (float(row[candidate_field]) / float(row[parent_field]) - 1) for row in group]
            entry.update({
                "candidate_us": candidate,
                "median_ratio_delta_percent": 100 * (candidate["median"] / parent["median"] - 1),
                "paired_difference_us": describe(differences),
                "paired_delta_percent": describe(deltas),
                "order_difference_us": {
                    order: describe([float(row[candidate_field]) - float(row[parent_field])
                                     for row in group if row["order"] == order])
                    for order in ("P-C", "C-P")
                },
                "block_difference_median_us": {
                    block: statistics.median(float(row[candidate_field]) - float(row[parent_field])
                                             for row in group if row["block"] == block)
                    for block in dict.fromkeys(row["block"] for row in group)
                },
            })
        result[case] = entry
    return result


def match_tasks(mode):
    profile = CAPTURE / (mode + "-profile")
    operators = read_rows(next(profile.glob("op_summary_*.csv")))
    operators.sort(key=lambda row: Decimal(row["Task Start Time(us)"].strip()))
    task_rows = read_rows(next(profile.glob("task_time_*.csv")))
    tasks = {(row["stream_id"], row["task_id"]): row for row in task_rows
             if row["kernel_type"] == "AI_VECTOR_CORE"}
    raw = read_rows(CAPTURE / (mode + "-event.raw.tsv"), "\t")
    count_per_case, outside_timing = (129, 47) if mode == "same" else (180, 92)
    assert len(operators) == 2 * count_per_case == len(tasks)
    assert {row["Device_id"] for row in operators} == {"1"}
    assert {row["Task Type"] for row in operators} == {"AI_VECTOR_CORE"}
    assert len({row["Stream ID"] for row in operators}) == 1
    for row in operators:
        task = tasks[(row["Stream ID"], row["Task ID"])]
        assert Decimal(row["Task Duration(us)"]) == Decimal(task["task_time(us)"])
        assert Decimal(row["Task Start Time(us)"].strip()) == Decimal(task["task_start(us)"].strip())
    mapped = []
    for case_index, (case, block_dim) in enumerate(CASES.items()):
        calls = operators[case_index * count_per_case:(case_index + 1) * count_per_case]
        assert {int(row["Block Dim"]) for row in calls} == {block_dim}
        timed = calls[outside_timing:]
        samples = [row for row in raw if row["case"] == case]
        assert len(timed) == len(samples) * (1 if mode == "same" else 2)
        for index, sample in enumerate(samples):
            entry = dict(sample)
            if mode == "same":
                calls_by_side = {"parent": timed[index]}
            else:
                ordered = timed[2 * index:2 * index + 2]
                parent, candidate = ordered if sample["order"] == "P-C" else ordered[::-1]
                calls_by_side = {"parent": parent, "candidate": candidate}
            for side, call in calls_by_side.items():
                duration = float(call["Task Duration(us)"])
                entry[side + "_task_us"] = duration
                entry[side + "_task_id"] = call["Task ID"]
                entry[side + "_stream_id"] = call["Stream ID"]
                entry[side + "_task_start_us"] = call["Task Start Time(us)"].strip()
                entry[side + "_event_minus_task_us"] = float(sample[side + "_device_us"]) - duration
            mapped.append(entry)
    return mapped, {"all_kernel_calls": len(operators), "timed_kernel_calls": 164 if mode == "same" else 176,
                    "untimed_calls_per_case": outside_timing,
                    "op_summary_and_task_time_agree": True}


def resource_context():
    paths = sorted(CAPTURE.glob("*-*.usages.txt"))
    paths += [ROOT / "logs" / (mode + "-device1-" + phase + ".txt")
              for mode in ("same", "local") for phase in ("preflight", "postflight")]
    result = {}
    for path in paths:
        text = path.read_text()
        capacity = int(re.search(r"HBM Capacity\(MB\)\s*:\s*(\d+)", text)[1])
        usage = int(re.search(r"^\s*HBM Usage Rate\(%\)\s*:\s*(\d+)", text, re.M)[1])
        result[str(path.relative_to(ROOT))] = {
            "free_hbm_mb_from_usage_percent": capacity * (100 - usage) // 100,
            "aicore_usage_percent": int(re.search(r"Aicore Usage Rate\(%\)\s*:\s*(\d+)", text)[1]),
        }
    return result


def protocol_mapping(capture, phase):
    profile = capture / (phase + "-profile")
    op_paths = list(profile.glob("op_summary_*.csv"))
    task_paths = list(profile.glob("task_time_*.csv"))
    assert len(op_paths) == len(task_paths) == 1
    operators = read_rows(op_paths[0])
    operators.sort(key=lambda row: Decimal(row["Task Start Time(us)"].strip()))
    task_rows = read_rows(task_paths[0])
    tasks = {(row["Device_id"], row["stream_id"], row["task_id"]): row for row in task_rows}
    assert len(tasks) == len(task_rows)
    assert len(operators) == 360
    assert {row["Device_id"] for row in operators} == {"1"}
    assert {row["Task Type"] for row in operators} == {"AI_VECTOR_CORE"}
    assert len({row["Stream ID"] for row in operators}) == 1
    for op in operators:
        task = tasks[(op["Device_id"], op["Stream ID"], op["Task ID"])]
        assert Decimal(op["Task Duration(us)"]) == Decimal(task["task_time(us)"])
        assert Decimal(op["Task Start Time(us)"].strip()) == Decimal(task["task_start(us)"].strip())

    log = (capture / (phase + "-profile.log")).read_text()
    addresses = {}
    references = []
    for line in log.splitlines():
        if line.startswith("OUTPUT_SLOTS "):
            fields = dict(re.findall(r"(\w+)=([^\s]+)", line))
            assert fields["slot_P_address"] != fields["slot_C_address"]
            assert fields["slot_P_kernel"] == "PARENT"
            assert fields["slot_C_kernel"] == ("PARENT" if phase == "pp" else "CANDIDATE")
            assert (fields["warmup_pairs"], fields["pair_blocks"], fields["pairs_per_block"]) == ("45", "4", "11")
            addresses[fields["case"]] = fields
        if line.startswith("CORRECTNESS "):
            fields = dict(re.findall(r"(\w+)=([^\s]+)", line))
            assert fields["parent_failures"] == fields["candidate_failures"] == "0"
            assert fields["reference"] == "CPU_FP32"
            references.append(fields)
    assert set(addresses) == set(CASES)
    assert {(row["case"], row["phase"]) for row in references} == {
        (case, moment) for case in CASES for moment in ("before_measure", "after_measure")}

    raw = read_rows(capture / (phase + "-event.raw.tsv"), "\t")
    assert len(raw) == 88
    pairs, mapped = [], []
    for case_index, (case, block_dim) in enumerate(CASES.items()):
        calls = operators[case_index * 180:(case_index + 1) * 180]
        assert {int(op["Block Dim"]) for op in calls} == {block_dim}
        samples = [row for row in raw if row["case"] == case]
        assert [(int(row["block"]), int(row["pair"])) for row in samples] == [
            (block, pair) for block in range(4) for pair in range(11)]
        timed = calls[92:]
        for index, sample in enumerate(samples):
            entry = dict(sample)
            expected_order = "P-C" if (int(sample["block"]) + int(sample["pair"])) % 2 == 0 else "C-P"
            assert sample["order"] == expected_order
            for position, slot in enumerate(expected_order.split("-"), 1):
                side = "parent" if slot == "P" else "candidate"
                op = timed[2 * index + position - 1]
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
                device_us = float(sample[side + "_device_us"])
                kernel_us = float(task["task_time(us)"])
                assert math.isfinite(device_us) and device_us > 0
                entry[side + "_task_us"] = kernel_us
                mapped.append({
                    "case": case, "phase": phase, "block": sample["block"],
                    "pair": sample["pair"], "order": expected_order, "position": position,
                    "slot": slot, "function": addresses[case]["slot_" + slot + "_kernel"],
                    "output_address": addresses[case]["slot_" + slot + "_address"],
                    "launch_ordinal": case_index * 180 + 92 + 2 * index + position,
                    "timed_ordinal": case_index * 88 + 2 * index + position,
                    "device_id": key[0], "stream_id": key[1], "task_id": key[2],
                    "block_dim": block_dim, "kernel_start_us": str(task_start),
                    "device_us": device_us, "kernel_us": kernel_us,
                    "start_record_task_id": start["task_id"], "stop_record_task_id": stop["task_id"],
                    "trace_interval_us": float(event_stop - event_start),
                    "trace_interval_error_us": float(event_stop - event_start) - device_us,
                    "start_to_kernel_us": float(task_start - event_start),
                    "kernel_to_stop_us": float(event_stop - task_stop),
                    "event_minus_kernel_us": device_us - kernel_us,
                })
            pairs.append(entry)
    counts = {"all_kernel_calls": len(operators), "timed_kernel_calls": len(mapped),
              "reference_calls": 4, "warmup_calls": 180, "all_task_rows": len(tasks),
              "all_op_task_pairs_match": True, "all_timed_tasks_have_event_brackets": True}
    return pairs, mapped, addresses, references, counts


def protocol_summary(capture, phase):
    pairs, mapped, addresses, references, counts = protocol_mapping(capture, phase)
    by_metric = {
        "event": summarize(pairs, "parent_device_us", "candidate_device_us"),
        "kernel_task": summarize(pairs, "parent_task_us", "candidate_task_us"),
    }
    shapes = {}
    for case in CASES:
        calls = [row for row in mapped if row["case"] == case]
        shape = {"addresses": addresses[case], "metrics": {}}
        for metric, field in (("event", "device_us"), ("kernel_task", "kernel_us")):
            entry = by_metric[metric][case]
            entry["pooled_us"] = describe_blocks(calls, field)
            entry["by_position_us"] = {
                str(position): describe([row[field] for row in calls if row["position"] == position])
                for position in (1, 2)}
            entry["by_slot_and_position_us"] = {
                slot: {str(position): describe([row[field] for row in calls
                                               if row["slot"] == slot and row["position"] == position])
                       for position in (1, 2)} for slot in ("P", "C")}
            differences = [calls[i + 1][field] - calls[i][field] for i in range(0, len(calls), 2)]
            entry["second_minus_first_us"] = describe(differences)
            entry["paired_abs_difference_us"] = describe([abs(value) for value in differences])
            entry["qualified"] = all(entry[key]["within_existing_limits"]
                                     for key in ("pooled_us", "parent_us", "candidate_us"))
            entry["first_timed_sample"] = calls[0]
            entry["largest_sample"] = max(calls, key=lambda row: row[field])
            shape["metrics"][metric] = entry
        for field in ("event_minus_kernel_us", "start_to_kernel_us", "kernel_to_stop_us",
                      "trace_interval_error_us"):
            shape[field] = describe([row[field] for row in calls])
        shape["max_abs_trace_interval_error_us"] = max(abs(row["trace_interval_error_us"]) for row in calls)
        shapes[case] = shape

    resource = {}
    for path in sorted(capture.glob("*.usages.txt")):
        text = path.read_text()
        capacity = int(re.search(r"HBM Capacity\(MB\)\s*:\s*(\d+)", text)[1])
        usage = int(re.search(r"^\s*HBM Usage Rate\(%\)\s*:\s*(\d+)", text, re.M)[1])
        resource[path.name] = {
            "free_hbm_mb_from_usage_percent": capacity * (100 - usage) // 100,
            "aicore_usage_percent": int(re.search(r"Aicore Usage Rate\(%\)\s*:\s*(\d+)", text)[1]),
        }
    qualified = all(shape["metrics"][metric]["qualified"]
                    for shape in shapes.values() for metric in by_metric)
    result = {"route": "W4-R11", "revision": "V002", "direct_parent": "R31B V011",
              "phase": phase, "new_performance_revisions": 0, "current_local_best": None,
              "all_samples_retained": True, "qualification": "PASS" if qualified else "MEASUREMENT_BLOCKED",
              "candidate_measured": phase == "pc", "counts": counts, "reference_results": references,
              "shapes": shapes, "resource_context": resource,
              "units": "All times are us. Delta fields ending percent use percent; CV/MAD/ranges are ratios. P/C label output slots; both call Parent in pp mode."}
    destination = capture / (phase + "-task-map.tsv")
    with destination.open("x", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(mapped[0]), delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(mapped)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--local-protocol", type=Path)
    parser.add_argument("--phase", choices=("pp", "pc"), default="pp")
    args = parser.parse_args()
    if args.local_protocol:
        print(json.dumps(protocol_summary(args.local_protocol, args.phase), indent=2,
                         ensure_ascii=False, allow_nan=False))
        return
    same, same_counts = match_tasks("same")
    local, local_counts = match_tasks("local")
    original_same = read_rows(ROOT / "results/requal-device1-20261008-original-same.raw.tsv", "\t")
    original_local = read_rows(ROOT / "results/requal-device1-20261008-original-pc.raw.tsv", "\t")
    result = {
        "route": "W4-R11", "revision": "V002", "direct_parent": "R31B V011",
        "new_performance_revisions": 0, "current_local_best": None,
        "units": "Latencies are us; delta_percent is percent; cv, mad_over_median and relative ranges are dimensionless.",
        "original_event": {"same": summarize(original_same, "parent_device_us"),
                           "local": summarize(original_local, "parent_device_us", "candidate_device_us")},
        "profiled_event": {"same": summarize(same, "parent_device_us"),
                           "local": summarize(local, "parent_device_us", "candidate_device_us")},
        "kernel_task": {"same": summarize(same, "parent_task_us"),
                        "local": summarize(local, "parent_task_us", "candidate_task_us")},
        "call_mapping_counts": {"same": same_counts, "local": local_counts},
        "matched_raw_samples": {"same": same, "local": local},
        "event_minus_task_us": {
            case: describe([row[side + "_event_minus_task_us"] for row in local if row["case"] == case
                            for side in ("parent", "candidate")]) for case in CASES
        },
        "resource_context": resource_context(),
    }
    print(json.dumps(result, indent=2, ensure_ascii=False, allow_nan=False))


if __name__ == "__main__":
    main()
