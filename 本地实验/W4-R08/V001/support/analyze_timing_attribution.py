#!/usr/bin/env python3
"""Analyze retained R08 samples and one fixed Parent/Parent collection."""

import csv
import json
import math
import re
import statistics as st
from collections import defaultdict
from decimal import Decimal
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "results/timing-attribution-20261008"
EXPORT = OUT / (
    "pp-profile-absolute/PROF_000001_20261008062352036_ERIQMARACJANGGLA/"
    "mindstudio_profiler_output"
)


def dec(value):
    return Decimal(str(value).strip())


def read_csv(path, delimiter=","):
    with path.open() as source:
        return list(csv.DictReader(source, delimiter=delimiter))


def stats(values):
    center = st.median(values)
    ordered = sorted(values)
    mean = st.mean(values)
    mad = st.median(abs(v - center) for v in values)
    return {
        "n": len(values), "median_us": center, "mean_us": mean,
        "stdev_us": st.pstdev(values),
        "cv": st.pstdev(values) / mean if mean else None,
        "mad_us": mad, "mad_over_median": mad / center if center else None,
        "min_us": ordered[0], "max_us": ordered[-1],
        "p10_us": ordered[max(0, math.ceil(len(values) * 0.10) - 1)],
        "p90_us": ordered[max(0, math.ceil(len(values) * 0.90) - 1)],
    }


def summarize(rows, field):
    values = [float(r[field]) for r in rows]
    blocks = defaultdict(list)
    for r in rows:
        blocks[int(r["block"])].append(float(r[field]))
    medians = [st.median(v) for _, v in sorted(blocks.items())]
    center = st.median(medians)
    first_drift = max(abs(v - medians[0]) for v in medians) / center
    full_range = (max(medians) - min(medians)) / center
    summary = stats(values)
    summary.update({
        "blocks": {str(b): stats(v) for b, v in sorted(blocks.items())},
        "block_medians_us": medians,
        "block_drift_from_first": first_drift,
        "block_relative_range": full_range,
        "stable_10pct": (
            summary["mad_over_median"] <= 0.10
            and all(stats(v)["mad_over_median"] <= 0.10 for v in blocks.values())
            and first_drift <= 0.10 and full_range <= 0.10
        ),
    })
    return summary


def raw_rows(path):
    rows = read_csv(path, "\t")
    for seq, r in enumerate(rows, 1):
        r["timed_sequence"] = seq
        r["block"] = int(r["block"])
        r["sample"] = int(r["sample"])
        r["device_us"] = float(r["device_us"])
        r["wall_us"] = float(r["wall_us"])
        assert r["shape"] == "17x257"
    return rows


def dump(path, data):
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n")


def analyze_old():
    result = {}
    names = [
        "device4-resume-20261008-parent.raw.tsv",
        "device4-resume-20261008-repaired-same.raw.tsv",
        "device4-resume-20261008-repaired-paired.raw.tsv",
    ]
    for name in names:
        rows = raw_rows(ROOT / "logs" / name)
        ordered_blocks = defaultdict(list)
        for r in rows:
            ordered_blocks[(r["block"], r["role"], r["order"])].append(r)
        block_results = []
        for (block, role, order), items in ordered_blocks.items():
            event = [r["device_us"] for r in items]
            wall = [r["wall_us"] for r in items]
            block_results.append({
                "block": block, "role": role, "order": order,
                "sequence_start": items[0]["timed_sequence"],
                "sequence_end": items[-1]["timed_sequence"],
                "event": stats(event), "wall": stats(wall),
                "first_sample": items[0],
                "event_wall_correlation": sum(
                    (a - st.mean(event)) * (b - st.mean(wall)) for a, b in zip(event, wall)
                ) / (len(event) * st.pstdev(event) * st.pstdev(wall)),
                "successive_10_10_11_event_medians_us": [
                    st.median(event[a:b]) for a, b in [(0, 10), (10, 20), (20, 31)]
                ],
                "event_above_50us_count": sum(v > 50 for v in event),
            })
        result[name] = {
            "count": len(rows), "blocks_in_execution_order": block_results,
            "top10_event_all_retained": sorted(rows, key=lambda r: r["device_us"], reverse=True)[:10],
        }
    assert sum(r["count"] for r in result.values()) == 372
    dump(OUT / "old-order-analysis.json", result)


def analyze_pp():
    rows = raw_rows(OUT / "pp.raw.tsv")
    ops = read_csv(EXPORT / "op_summary_20261008062358.csv")
    tasks = read_csv(EXPORT / "task_time_20261008062358.csv")
    trace = json.loads((EXPORT / "msprof_20261008062356.json").read_text())
    ops.sort(key=lambda x: dec(x["Task Start Time(us)"]))
    assert len(rows) == 248 and len(ops) == 339
    task_by_id = {(r["Device_id"], r["stream_id"], r["task_id"]): r for r in tasks}
    assert len(task_by_id) == len(tasks)
    stream_tasks = defaultdict(list)
    for r in tasks:
        stream_tasks[(r["Device_id"], r["stream_id"])].append(r)
    stream_positions = {}
    for stream, items in stream_tasks.items():
        items.sort(key=lambda x: (dec(x["task_start(us)"]), int(x["task_id"])))
        stream_positions[stream] = {r["task_id"]: i for i, r in enumerate(items)}

    def trace_items(name):
        return sorted((e for e in trace if e.get("ph") == "X" and e.get("name") == name),
                      key=lambda e: dec(e["ts"]))

    def cid(e):
        return e["args"]["connection_id"]

    api_events = trace_items("AscendCL@aclrtRecordEvent")
    api_sync = trace_items("AscendCL@aclrtSynchronizeEvent")
    nodes = trace_items("Node@launch")
    allocs = trace_items("Runtime@DevMalloc")
    frees = trace_items("AscendCL@aclrtFree")
    node_by_cid = {cid(e): e for e in nodes}
    device_trace = {
        (str(e["args"]["Physic Stream Id"]), str(e["args"]["Task Id"])): e
        for e in trace if e.get("ph") == "X" and "Physic Stream Id" in e.get("args", {})
    }
    assert len(api_events) == 496 and len(api_sync) == 248 and len(nodes) == 339

    for op in ops:
        key = (op["Device_id"], op["Stream ID"], op["Task ID"])
        task = task_by_id[key]
        assert op["Device_id"] == "2" and op["Block Dim"] == "8"
        assert "w4r08_parent" in op["Op Name"]
        assert op["Op Name"] == task["kernel_name"]
        assert dec(op["Task Start Time(us)"]) == dec(task["task_start(us)"])
        assert dec(op["Task Duration(us)"]) == dec(task["task_time(us)"])
        assert dec(task["task_stop(us)"]) - dec(task["task_start(us)"]) == dec(task["task_time(us)"])

    log = (OUT / "pp-profile-absolute.log").read_text()
    addresses = re.search(r"output_slot1=(\S+) output_slot2=(\S+)", log).groups()
    assert addresses[0] == addresses[1]
    verifications = re.findall(r"verify rows=17 width=257 failures=(\d+) max_abs=([0-9.e+-]+)", log)
    assert len(verifications) == 2 and all(f == "0" for f, _ in verifications)
    assert "post_timing_verify=PASS" in log

    for i, (row, op) in enumerate(zip(rows, ops[91:])):
        block = i // 62
        position = (i % 62) // 31 + 1
        first_role = "parent_slot1" if block % 2 == 0 else "parent_slot2"
        second_role = "parent_slot2" if block % 2 == 0 else "parent_slot1"
        assert row["block"] == block and row["sample"] == i % 31
        assert row["role"] == (first_role if position == 1 else second_role)
        assert row["order"] == ("P1P2" if block % 2 == 0 else "P2P1")
        stream = (op["Device_id"], op["Stream ID"])
        items = stream_tasks[stream]
        pos = stream_positions[stream][op["Task ID"]]
        before, task, after = items[pos - 1:pos + 2]
        assert before["kernel_type"] == after["kernel_type"] == "EVENT_RECORD"
        start = dec(before["task_start(us)"])
        stop = dec(after["task_start(us)"])
        task_start = dec(task["task_start(us)"])
        task_stop = dec(task["task_stop(us)"])
        event_span = stop - start
        assert start <= task_start <= task_stop <= stop
        event_error = row["device_us"] - float(event_span)
        assert abs(event_error) < 0.05
        t_start = device_trace[(stream[1], before["task_id"])]
        t_stop = device_trace[(stream[1], after["task_id"])]
        t_kernel = device_trace[(stream[1], op["Task ID"])]
        host_start, host_stop = api_events[2*i:2*i+2]
        assert cid(host_start) == cid(t_start) and cid(host_stop) == cid(t_stop)
        node = node_by_cid[cid(t_kernel)]
        assert dec(node["ts"]) == dec(nodes[i + 91]["ts"])
        h0, h1 = dec(host_start["ts"]), dec(host_stop["ts"])
        alloc = [e for e in allocs if h0 <= dec(e["ts"]) < h1]
        release = [e for e in frees if h0 <= dec(e["ts"]) < h1]
        assert len(alloc) == len(release) == 1
        sync = api_sync[i]
        assert h0 < dec(node["ts"]) < h1 < dec(sync["ts"])
        outer = [host_start, alloc[0], node, release[0], host_stop, sync]
        api_total = sum(dec(e["dur"]) for e in outer)
        api_span = dec(sync["ts"]) + dec(sync["dur"]) - h0
        row.update({
            "kernel_ordinal": i + 92, "actual_kernel": "PARENT", "block_position": position,
            "output_address": addresses[0], "device": stream[0], "stream_id": stream[1],
            "task_id": op["Task ID"], "task_start_us": str(task_start), "task_stop_us": str(task_stop),
            "task_us": float(dec(task["task_time(us)"])),
            "start_event_task_id": before["task_id"], "stop_event_task_id": after["task_id"],
            "start_event_time_us": str(start), "stop_event_time_us": str(stop),
            "event_timestamp_span_us": float(event_span), "event_elapsed_error_us": event_error,
            "pre_task_gap_us": float(task_start - start),
            "post_task_gap_us": float(stop - task_stop),
            "event_outside_task_us": float(event_span - dec(task["task_time(us)"])),
            "reported_event_minus_task_us": row["device_us"] - float(dec(task["task_time(us)"])),
            "start_event_cid": cid(t_start), "kernel_cid": cid(t_kernel), "stop_event_cid": cid(t_stop),
            "host_start_record_us": float(dec(host_start["dur"])),
            "host_runtime_malloc_us": float(dec(alloc[0]["dur"])),
            "host_node_launch_us": float(dec(node["dur"])),
            "host_acl_free_us": float(dec(release[0]["dur"])),
            "host_stop_record_us": float(dec(host_stop["dur"])),
            "host_sync_event_us": float(dec(sync["dur"])),
            "host_outer_api_total_us": float(api_total), "host_outer_api_span_us": float(api_span),
            "host_between_outer_apis_us": float(api_span - api_total),
        })

    with (OUT / "pp-task-map.tsv").open("w", newline="") as target:
        writer = csv.DictWriter(target, fieldnames=list(rows[0]), delimiter="\t")
        writer.writeheader()
        writer.writerows(rows)
    metrics = ("device_us", "task_us", "wall_us")
    slices = {"all": rows}
    for role in ("parent_slot1", "parent_slot2"):
        slices[role] = [r for r in rows if r["role"] == role]
        for position in (1, 2):
            slices[f"{role}_position{position}"] = [
                r for r in rows if r["role"] == role and r["block_position"] == position
            ]
    for position in (1, 2):
        slices[f"position{position}"] = [r for r in rows if r["block_position"] == position]
    aggregate = {name: {metric: summarize(items, metric) for metric in metrics}
                 for name, items in slices.items()}
    block_differences = []
    for block in range(4):
        side1 = [r for r in rows if r["block"] == block and r["role"] == "parent_slot1"]
        side2 = [r for r in rows if r["block"] == block and r["role"] == "parent_slot2"]
        block_differences.append({
            "block": block, "order": side1[0]["order"],
            **{metric: {
                "slot1_median_us": st.median(r[metric] for r in side1),
                "slot2_median_us": st.median(r[metric] for r in side2),
                "slot2_vs_slot1_percent": (st.median(r[metric] for r in side2) /
                                           st.median(r[metric] for r in side1) - 1) * 100,
            } for metric in metrics},
        })
    reliability = {metric: all(aggregate[name][metric]["stable_10pct"]
                               for name in ("all", "parent_slot1", "parent_slot2"))
                   for metric in ("device_us", "task_us")}
    result = {
        "route": "W4-R08", "revision": "V001",
        "status": "PP_RELIABLE" if all(reliability.values()) else "MEASUREMENT_BLOCKED",
        "reliability_by_timing_range": reliability,
        "shape": "17x257", "dtype": "FP32", "available_core_num_argument": 8,
        "block_count": 8, "local_rows": [3, 2, 2, 2, 2, 2, 2, 2],
        "candidate_prefetch_count_from_source": 9, "new_performance_revisions": 0,
        "candidate_execution": "NOT_EXECUTED", "local_score": None, "local_delta": None,
        "reference_before_after": [{"failures": int(f), "max_abs_error": float(e)} for f, e in verifications],
        "output_address": addresses[0], "profile_kernel_count": len(ops), "task_time_rows": len(tasks),
        "matched_ops": len(ops), "reference_launches": 1, "warmup_launches": 90,
        "timed_launches": len(rows), "task_event_and_host_connection_matches": len(rows),
        "max_event_elapsed_error_us": max(abs(r["event_elapsed_error_us"]) for r in rows),
        "aggregate": aggregate, "block_slot_differences": block_differences,
        "intervals": {field: stats([r[field] for r in rows]) for field in (
            "pre_task_gap_us", "post_task_gap_us", "event_outside_task_us",
            "reported_event_minus_task_us", "host_runtime_malloc_us", "host_node_launch_us",
            "host_acl_free_us", "host_start_record_us", "host_stop_record_us",
            "host_sync_event_us", "host_outer_api_total_us", "host_outer_api_span_us",
            "host_between_outer_apis_us")},
        "per_call_runtime_malloc_count": 248, "per_call_acl_free_count": 248,
        "first_samples_all_eight_blocks": [r for r in rows if r["sample"] == 0],
        "top10_event_all_retained": sorted(rows, key=lambda r: r["device_us"], reverse=True)[:10],
        "top10_task_all_retained": sorted(rows, key=lambda r: r["task_us"], reverse=True)[:10],
        "top10_wall_all_retained": sorted(rows, key=lambda r: r["wall_us"], reverse=True)[:10],
        "limits": [
            "Profiler-enabled host spans are observations; nested API durations are not added twice.",
            "Device and host intervals may overlap; API times are not additive causes of event gaps.",
            "The full ELF differs after host linkage; only device-related sections were compared.",
            "Old device4 event samples cannot be assigned new device2 task durations.",
            "No Candidate measurement in this collection and no Official result.",
        ],
    }
    dump(OUT / "pp-summary.json", result)
    print(json.dumps({k: result[k] for k in ("status", "matched_ops", "timed_launches",
                                           "max_event_elapsed_error_us", "candidate_execution")}, indent=2))
    for name in ("all", "parent_slot1", "parent_slot2", "position1", "position2"):
        print(name, {m: {k: aggregate[name][m][k] for k in (
            "median_us", "mad_over_median", "block_medians_us", "block_relative_range")}
                     for m in ("device_us", "task_us")})
    print("interval_medians_us", {f: v["median_us"] for f, v in result["intervals"].items()})


if __name__ == "__main__":
    analyze_old()
    analyze_pp()
