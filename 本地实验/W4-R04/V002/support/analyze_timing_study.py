#!/usr/bin/env python3
"""Offline correspondence and statistics for the bounded R04 timing study."""

import csv
import json
import math
import statistics
from collections import Counter, defaultdict
from decimal import Decimal
from pathlib import Path

HERE = Path(__file__).resolve().parents[1]
STUDY = HERE / "logs/timing-attribution-20261008T1155Z"
RAW = STUDY / "raw/samples-timing-attribution-20261008T1155Z.tsv"
REF = STUDY / "raw/samples-timing-attribution-20261008T1155Z.tsv.reference.tsv"
EXPORT = STUDY / "mindstudio_profiler_output"
TASK_CSV = EXPORT / "task_time_20261008115611.csv"
OP_CSV = EXPORT / "op_summary_20261008115611.csv"
TRACE_JSON = EXPORT / "msprof_20261008115609.json"


def d(value):
    return Decimal(str(value).strip())


def read_csv(path, delimiter=","):
    with path.open(newline="") as stream:
        return list(csv.DictReader(stream, delimiter=delimiter))


def stats(values):
    vals = [float(value) for value in values]
    med = statistics.median(vals)
    mad = statistics.median(abs(value - med) for value in vals)
    mean = statistics.mean(vals)
    stdev = statistics.pstdev(vals)
    ordered = sorted(vals)

    def quantile(q):
        point = (len(ordered) - 1) * q
        low, high = math.floor(point), math.ceil(point)
        return ordered[low] + (ordered[high] - ordered[low]) * (point - low)

    return {
        "n": len(vals), "median_us": med, "mean_us": mean,
        "stdev_us": stdev, "cv": stdev / mean if mean else None,
        "mad_us": mad, "mad_over_median": mad / med if med else None,
        "min_us": min(vals), "max_us": max(vals),
        "p10_us": quantile(.10), "p90_us": quantile(.90),
    }


def main():
    raw = read_csv(RAW, "\t")
    references = read_csv(REF, "\t")
    tasks = read_csv(TASK_CSV)
    ops = read_csv(OP_CSV)
    tasks = [{key.strip(): value.strip() for key, value in row.items()} for row in tasks]
    ops = [{key.strip(): value.strip() for key, value in row.items()} for row in ops]
    trace = json.loads(TRACE_JSON.read_text(), parse_float=Decimal)
    assert len(raw) == 768
    assert len(references) == 6
    assert all(row["parent_failures"] == row["candidate_failures"] == "0"
               and row["parent_candidate_bitwise_mismatches"] == "0"
               for row in references)
    assert len(tasks) == 4298 and len(ops) == 1074

    task_by_id = {(row["Device_id"], row["stream_id"], row["task_id"]): row for row in tasks}
    assert len(task_by_id) == len(tasks)
    op_by_id = {(row["Device_id"], row["Stream ID"], row["Task ID"]): row for row in ops}
    assert len(op_by_id) == len(ops)
    raw_sequence = [int(row["sequence"]) for row in raw]
    assert raw_sequence == list(range(1, 769))

    records = sorted((event for event in trace if event.get("ph") == "X"
                      and event.get("name") == "AscendCL@aclrtRecordEvent"),
                     key=lambda event: d(event["ts"]))
    syncs = sorted((event for event in trace if event.get("ph") == "X"
                    and event.get("name") == "AscendCL@aclrtSynchronizeEvent"),
                   key=lambda event: d(event["ts"]))
    nodes = [event for event in trace if event.get("ph") == "X" and event.get("name") == "Node@launch"]
    assert len(records) == 1536 and len(syncs) == 768 and len(nodes) == 1074
    record_by_connection = {event["args"]["connection_id"]: event for event in records}
    node_by_connection = {event["args"]["connection_id"]: event for event in nodes}
    device_events = [event for event in trace if event.get("ph") == "X"
                     and "Physic Stream Id" in event.get("args", {})]
    device_event_by_id = {(str(event["args"]["Physic Stream Id"]),
                           str(event["args"]["Task Id"])): event
                          for event in device_events}
    event_records = [event for event in device_events if event["name"] == "EVENT_RECORD"]
    event_record_by_connection = {event["args"]["connection_id"]: event for event in event_records}

    timed = [row for row in raw if row["mode"] in ("PP", "PC")]
    device_event_by_connection = {event["args"]["connection_id"]: event
                                 for event in device_events}
    for index, row in enumerate(timed):
        row["sequence"] = int(row["sequence"])
        row["block"] = int(row["block"])
        row["cycle"] = int(row["cycle"])
        row["position"] = int(row["position"])
        row["device_event_us"] = float(row["device_event_us"])
        row["wall_us"] = float(row["wall_us"])
        assert row["sequence"] == index + 1
        host_start, host_stop = records[index * 2:index * 2 + 2]
        assert host_start["args"]["id"] == host_stop["args"]["id"] == "aclrtRecordEvent"
        start_device = event_record_by_connection[host_start["args"]["connection_id"]]
        stop_device = event_record_by_connection[host_stop["args"]["connection_id"]]
        stream = str(start_device["args"]["Physic Stream Id"])
        assert stream == str(stop_device["args"]["Physic Stream Id"])
        start_id = str(start_device["args"]["Task Id"])
        stop_id = str(stop_device["args"]["Task Id"])
        start_task = task_by_id[("1", stream, start_id)]
        stop_task = task_by_id[("1", stream, stop_id)]
        assert start_task["kernel_type"] == stop_task["kernel_type"] == "EVENT_RECORD"
        host_launches = [event for event in nodes
                         if event["args"]["connection_id"] in node_by_connection
                         and d(host_start["ts"]) <= d(event["ts"]) < d(host_stop["ts"])
                         and event["args"]["Thread Id"] == host_start["args"]["Thread Id"]]
        assert len(host_launches) == 1
        launch = host_launches[0]
        matching_device_event = device_event_by_connection[launch["args"]["connection_id"]]
        assert matching_device_event["name"] == "_Z24add_rms_norm_bias_customIfEvPhS0_S0_S0_S0_mmjff"
        task_id = str(matching_device_event["args"]["Task Id"])
        op = op_by_id[("1", stream, task_id)]
        task = task_by_id[("1", stream, task_id)]
        assert op["Op Name"] == task["kernel_name"]
        assert d(op["Task Start Time(us)"]) == d(task["task_start(us)"])
        assert d(op["Task Duration(us)"]) == d(task["task_time(us)"])
        start, stop = d(start_task["task_start(us)"]), d(stop_task["task_start(us)"])
        task_start, task_stop = d(task["task_start(us)"]), d(task["task_stop(us)"])
        assert start <= task_start <= task_stop <= stop
        row.update({
            "stream_id": stream, "task_id": task_id,
            "task_us": float(d(task["task_time(us)"])),
            "task_start_us": str(task_start), "task_stop_us": str(task_stop),
            "event_start_us": str(start), "event_stop_us": str(stop),
            "event_task_span_us": float(stop - start),
            "event_timestamp_error_us": float(stop - start) - float(d(row["device_event_us"])),
            "pre_task_gap_us": float(task_start - start),
            "post_task_gap_us": float(stop - task_stop),
            "host_start_record_us": float(d(host_start["dur"])),
            "host_stop_record_us": float(d(host_stop["dur"])),
            "host_node_launch_us": float(d(launch["dur"])),
            "start_connection_id": host_start["args"]["connection_id"],
            "kernel_connection_id": launch["args"]["connection_id"],
            "stop_connection_id": host_stop["args"]["connection_id"],
        })

    # Recover block preconditions using the explicitly synchronized final call
    # of the opposite-mode block. This is context metadata, not a timed sample.
    for row in timed:
        row["predecessor_reset"] = ("P" if row["mode"] == "PP" else
                                     "C" if row["cycle"] == 1 and row["position"] == 1 else "NONE")

    # Validate each block-local 8-call PC cycle in capture order. Cycle numbers
    # in raw are a continuous counter, so don't use them as block-local IDs.
    for row in timed:
        row["block_cycle"] = None
    for block in range(1, 5):
        for case in sorted({r["case"] for r in timed}):
            block_rows = sorted((r for r in timed if r["case"] == case and r["block"] == block and r["mode"] == "PC"),
                                key=lambda r: r["sequence"])
            assert len(block_rows) == 32
            for cycle in range(4):
                cycle_rows = block_rows[cycle * 8:(cycle + 1) * 8]
                for row in cycle_rows:
                    row["block_cycle"] = cycle + 1
                assert len(cycle_rows) == 8
                assert "".join(row["variant"] for row in cycle_rows) == "PPPCCPCC"
                assert [row["position"] for row in cycle_rows] == [1, 2] * 4
                assert [row["previous_variant"] for row in cycle_rows] == list("CPPPCCPC")

    combinations = defaultdict(list)
    exact_transitions = defaultdict(list)
    for row in timed:
        if row["mode"] == "PC":
            combinations[(row["previous_variant"], row["position"])].append(row)
            exact_transitions[(row["previous_variant"], row["variant"], row["position"])].append(row)
    assert len(combinations) == 4 and {len(rows) for rows in combinations.values()} == {96}
    assert all(Counter(row["variant"] for row in rows) == Counter({"P": 48, "C": 48})
               for rows in combinations.values())
    assert len(exact_transitions) == 8 and {len(rows) for rows in exact_transitions.values()} == {48}

    summaries = {}
    for case in sorted({row["case"] for row in timed}):
        case_rows = [row for row in timed if row["case"] == case]
        summaries[case] = {"pp": {}, "pc": {}, "predecessor_position": {}, "transitions": {}}
        for mode in ("PP", "PC"):
            for variant in ("P", "C"):
                selected = [row for row in case_rows if row["mode"] == mode and row["variant"] == variant]
                if selected:
                    summaries[case][mode.lower()][variant] = {
                        "event": stats(row["device_event_us"] for row in selected),
                        "task": stats(row["task_us"] for row in selected),
                        "wall": stats(row["wall_us"] for row in selected),
                        "event_task_gap": stats(row["event_task_span_us"] - row["task_us"] for row in selected),
                        "by_position": {},
                        "block_task_medians_us": [],
                    }
                    entry = summaries[case][mode.lower()][variant]
                    for position in (1, 2):
                        pos = [row for row in selected if row["position"] == position]
                        entry["by_position"][str(position)] = {
                            "event": stats(row["device_event_us"] for row in pos),
                            "task": stats(row["task_us"] for row in pos),
                        }
                    entry["block_task_medians_us"] = [
                        statistics.median(row["task_us"] for row in selected if row["block"] == block)
                        for block in range(1, 5)
                    ]
            if mode == "PC":
                for key, values in sorted(combinations.items()):
                    selected = [row for row in values if row["case"] == case]
                    p = [row for row in selected if row["variant"] == "P"]
                    c = [row for row in selected if row["variant"] == "C"]
                    p_med = statistics.median(row["task_us"] for row in p)
                    c_med = statistics.median(row["task_us"] for row in c)
                    pair_indices = sorted({(row["block"], row["block_cycle"]) for row in selected})
                    paired = []
                    for block, cycle in pair_indices:
                        cell = [r for r in selected if r["block"] == block and r["block_cycle"] == cycle]
                        p_value = next((row["task_us"] for row in cell if row["variant"] == "P"), None)
                        c_value = next((row["task_us"] for row in cell if row["variant"] == "C"), None)
                        if p_value is not None and c_value is not None:
                            paired.append((c_value - p_value, (c_value / p_value - 1) * 100))
                    summaries[case]["predecessor_position"]["/".join(map(str, key))] = {
                        "P_n": len(p), "C_n": len(c), "P_task_median_us": p_med,
                        "C_task_median_us": c_med,
                        "ratio_delta_percent": (c_med / p_med - 1) * 100,
                        "paired_task_delta_median_us": statistics.median(x[0] for x in paired),
                        "paired_task_delta_percent_median": statistics.median(x[1] for x in paired),
                        "P_event_median_us": statistics.median(row["device_event_us"] for row in p),
                        "C_event_median_us": statistics.median(row["device_event_us"] for row in c),
                    }
                for key, values in sorted(exact_transitions.items()):
                    selected = [row for row in values if row["case"] == case]
                    summaries[case]["transitions"]["->".join(map(str, key))] = {
                        "n": len(selected),
                        "task": stats(row["task_us"] for row in selected),
                        "event": stats(row["device_event_us"] for row in selected),
                    }

    # Host API durations are descriptive and kept separate from device times.
    host_summary = {
        name: stats(row[name] for row in timed)
        for name in ("host_start_record_us", "host_node_launch_us", "host_stop_record_us", "wall_us")
    }
    output_fields = list(timed[0].keys())
    with (STUDY / "task-event-host-map.tsv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, output_fields, delimiter="\t", extrasaction="ignore")
        writer.writeheader()
        writer.writerows(timed)

    result = {
        "route": "W4-R04", "revision": "V002",
        "event_id": "W4-R04-V002-TIMING-ATTRIBUTION-20261008",
        "classification": "TIMING_ATTRIBUTION_SUPPLEMENT",
        "device": 1, "dtype": "FP32", "core_count": 40,
        "timed_samples": len(timed), "raw_samples_retained": len(raw),
        "task_rows": len(tasks), "op_rows": len(ops), "timeline_events": len(trace),
        "mapped_timed_calls": len(timed), "event_to_task_matches": len(timed),
        "reference_before_after": references,
        "pc_transition_position_cells": {"->".join(map(str, key)): len(value)
                                           for key, value in sorted(exact_transitions.items())},
        "all_calls": summaries, "host_summary": host_summary,
        "max_abs_event_timestamp_error_us": max(abs(row["event_timestamp_error_us"]) for row in timed),
        "resource_before": (STUDY / "timing-attribution-20261008T1155Z-pre-usages.txt").read_text(),
        "resource_after": (STUDY / "post-context.txt").read_text(),
        "measurement_status": "MEASUREMENT_BLOCKED",
        "new_performance_revisions": 0, "valid_local_results": 0,
        "current_local_best": None, "official_score": None, "online_state": "PAUSED",
        "push": "NO",
        "limitations": [
            "P/P uses Parent only; no fabricated Parent peer or Candidate result.",
            "Profiler task/event mapping uses stream/task IDs, host connection IDs, and exact Decimal timestamps.",
            "device event and wall/context durations are diagnostic and do not replace task-time as the stated primary measure.",
            "four blocks and one process capture do not establish repeatable Local improvement.",
        ],
    }
    (STUDY / "timing-summary.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n")
    print(json.dumps({"mapped": len(timed), "raw": len(raw),
                      "trace": len(trace), "task_rows": len(tasks), "op_rows": len(ops),
                      "max_event_error_us": result["max_abs_event_timestamp_error_us"],
                      "summary": summaries}, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
