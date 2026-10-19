#!/usr/bin/env python3
"""Analyze R07's retained samples and the one V002 timing study.

Decimal time and connection-id matching follow the R08 analyzer at 1bc84959.
R07 has two shared libraries and alternates order per pair.
"""

import csv
import json
import math
import re
import statistics as st
from collections import Counter, defaultdict
from decimal import Decimal
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "results/timing-attribution-20261008"
EXPORT = OUT / "profile/PROF_000001_20261008065214077_RJGDCBJJRBCDABBB/mindstudio_profiler_output"


def dec(value):
    return Decimal(str(value).strip())


def read_rows(path, delimiter=","):
    return list(csv.DictReader((line for line in path.read_text().splitlines()
                               if not line.startswith("#")), delimiter=delimiter))


def stats(values):
    ordered = sorted(float(v) for v in values)
    center = st.median(ordered)
    mean = st.mean(ordered)
    mad = st.median(abs(v - center) for v in ordered)

    def quantile(q):
        location = (len(ordered) - 1) * q
        lo, hi = math.floor(location), math.ceil(location)
        return ordered[lo] + (ordered[hi] - ordered[lo]) * (location - lo)

    return dict(n=len(ordered), median_us=center, mean_us=mean,
                stdev_us=st.pstdev(ordered), cv=st.pstdev(ordered) / mean if mean else None,
                mad_us=mad, mad_over_median=mad / center if center else None,
                min_us=ordered[0], max_us=ordered[-1], p10_us=quantile(.1), p90_us=quantile(.9))


def summarize(rows, field):
    result = stats(r[field] for r in rows)
    blocks = defaultdict(list)
    for row in rows:
        blocks[int(row["block"])].append(float(row[field]))
    result["blocks"] = {str(k): stats(v) for k, v in sorted(blocks.items())}
    medians = [v["median_us"] for v in result["blocks"].values()]
    result["block_medians_us"] = medians
    result["block_relative_range"] = (max(medians) - min(medians)) / st.median(medians)
    return result


def dump(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n")


def analyze_old():
    result = {}
    for width in (12288, 8192):
        for mode in ("same", "paired"):
            name = f"local-a1-{width}-{mode}.tsv"
            rows = read_rows(ROOT / "logs" / name, "\t")
            previous = "candidate" if mode == "paired" else "parent"
            switches = Counter()
            for i, row in enumerate(rows):
                row["ordinal"] = i + 1
                row["block"] = int(row["block"])
                row["sample"] = int(row["sample"])
                row["device_event_us"] = float(row["device_event_us"])
                row["wall_us"] = float(row["wall_us"])
                row["position"] = i % 2 + 1 if mode == "paired" else None
                row["previous_side"] = previous
                row["library_switch"] = row["side"] != previous
                if mode == "paired":
                    expected = "CP" if (row["block"] + row["sample"]) % 2 else "PC"
                    assert row["order"] == expected
                    assert row["side"] == ("parent" if expected[i % 2] == "P" else "candidate")
                    switches[f"position{row['position']}_switch{int(row['library_switch'])}"] += 1
                previous = row["side"]
            slices = {"all": rows}
            if mode == "paired":
                for side in ("parent", "candidate"):
                    slices[side] = [r for r in rows if r["side"] == side]
                    for position in (1, 2):
                        slices[f"{side}_position{position}"] = [
                            r for r in rows if r["side"] == side and r["position"] == position]
                for position in (1, 2):
                    slices[f"position{position}"] = [r for r in rows if r["position"] == position]
            slices["first_pair_or_sample"] = [r for r in rows if r["sample"] == 1]
            slices["later_samples"] = [r for r in rows if r["sample"] > 1]
            result[name] = {
                "n": len(rows), "groups": {
                    k: {field: summarize(v, field) for field in ("device_event_us", "wall_us")}
                    for k, v in slices.items()},
                "position_library_switch_counts": dict(switches),
                "largest_events_all_retained": sorted(rows, key=lambda r: r["device_event_us"], reverse=True)[:5],
            }
            if mode == "paired":
                result[name]["median_second_minus_first_us"] = st.median(
                    rows[i + 1]["device_event_us"] - rows[i]["device_event_us"]
                    for i in range(0, len(rows), 2))
    assert sum(r["n"] for r in result.values()) == 620
    dump(OUT / "old-order-analysis.json", result)


def analyze_study():
    raw_path = OUT / "samples.tsv"
    headers = [line for line in raw_path.read_text().splitlines() if line.startswith("#")]
    rows = read_rows(raw_path, "\t")
    reference = read_rows(OUT / "samples.tsv.reference.tsv", "\t")
    assert len(reference) == 4
    assert all(r["status"] == "PASS" and r["mismatches"] == "0" and r["nonfinite"] == "0"
               and r["device"] == "2" and r["width"] == "12288" for r in reference)
    library_rows = [re.search(r"side=(\S+) function=(\S+) object=(\S+)", h).groups()
                    for h in headers if h.startswith("# LIBRARY")]
    libraries = {side: {"function": function, "object": obj} for side, function, obj in library_rows}
    assert libraries["parent"] == libraries["parent_peer"]
    assert libraries["parent"]["object"] != libraries["candidate"]["object"]
    address = re.search(r"OUTPUT_ADDRESS=(\S+)", "\n".join(headers)).group(1)
    ops = read_rows(EXPORT / "op_summary_20261008065222.csv")
    tasks = read_rows(EXPORT / "task_time_20261008065222.csv")
    trace = json.loads((EXPORT / "msprof_20261008065221.json").read_text(), parse_float=Decimal)
    ops.sort(key=lambda r: dec(r["Task Start Time(us)"]))
    assert len(rows) == 128 and len(ops) == 222
    task_by_id = {(r["Device_id"], r["stream_id"], r["task_id"]): r for r in tasks}
    assert len(task_by_id) == len(tasks)
    for op in ops:
        task = task_by_id[(op["Device_id"], op["Stream ID"], op["Task ID"])]
        assert op["Device_id"] == "2" and op["Block Dim"] == "40"
        assert op["Op Name"] == "_Z24add_rms_norm_bias_customIDhEvPhS0_S0_S0_S0_mmjff"
        assert op["Op Name"] == task["kernel_name"]
        assert dec(op["Task Start Time(us)"]) == dec(task["task_start(us)"])
        assert dec(op["Task Duration(us)"]) == dec(task["task_time(us)"])
        assert dec(task["task_stop(us)"]) - dec(task["task_start(us)"]) == dec(task["task_time(us)"])

    def items(name):
        return sorted((e for e in trace if e.get("ph") == "X" and e.get("name") == name),
                      key=lambda e: dec(e["ts"]))

    def cid(event):
        return event["args"]["connection_id"]

    records = items("AscendCL@aclrtRecordEvent")
    syncs = items("AscendCL@aclrtSynchronizeEvent")
    nodes = items("Node@launch")
    allocs, frees = items("Runtime@DevMalloc"), items("AscendCL@aclrtFree")
    assert len(records) == 256 and len(syncs) == 128 and len(nodes) == 222
    node_by_cid = {cid(e): e for e in nodes}
    device_trace = [e for e in trace if e.get("ph") == "X" and "Physic Stream Id" in e.get("args", {})]
    trace_by_id = {(str(e["args"]["Physic Stream Id"]), str(e["args"]["Task Id"])): e for e in device_trace}
    event_by_cid = {cid(e): e for e in device_trace if e["name"] == "EVENT_RECORD"}
    mode_sequence = ("PP", "PC", "PC", "PP", "PC", "PP", "PP", "PC")
    previous = "candidate"

    for i, row in enumerate(rows):
        for field in ("block", "sample", "position", "kernel_sequence", "cpu_after"):
            row[field] = int(row[field])
        for field in ("device_event_us", "wall_us"):
            row[field] = float(row[field])
        assert row["block"] == i // 16 + 1 and row["sample"] == (i % 16) // 2 + 1
        assert row["position"] == i % 2 + 1 and row["kernel_sequence"] == i + 93
        assert row["study_mode"] == mode_sequence[i // 16]
        peer = "candidate" if row["study_mode"] == "PC" else "parent_peer"
        swapped = (row["block"] + row["sample"]) % 2 != 0
        expected = (peer, "parent") if swapped else ("parent", peer)
        assert row["side"] == expected[i % 2]
        actual = "candidate" if row["side"] == "candidate" else "parent"
        row["actual_library"] = actual
        row["previous_library"] = previous
        row["library_switch"] = int(actual != previous)
        previous = actual
        op = ops[row["kernel_sequence"] - 1]
        stream = op["Stream ID"]
        task = task_by_id[("2", stream, op["Task ID"])]
        host_start, host_stop = records[2*i:2*i + 2]
        start_trace, stop_trace = event_by_cid[cid(host_start)], event_by_cid[cid(host_stop)]
        assert str(start_trace["args"]["Physic Stream Id"]) == str(stop_trace["args"]["Physic Stream Id"]) == stream
        start_id, stop_id = str(start_trace["args"]["Task Id"]), str(stop_trace["args"]["Task Id"])
        start_task, stop_task = task_by_id[("2", stream, start_id)], task_by_id[("2", stream, stop_id)]
        assert start_task["kernel_type"] == stop_task["kernel_type"] == "EVENT_RECORD"
        start, stop = dec(start_task["task_start(us)"]), dec(stop_task["task_start(us)"])
        task_start, task_stop = dec(task["task_start(us)"]), dec(task["task_stop(us)"])
        assert dec(start_trace["ts"]) == start and dec(stop_trace["ts"]) == stop
        assert start <= task_start <= task_stop <= stop
        device_kernel = trace_by_id[(stream, op["Task ID"])]
        node = node_by_cid[cid(device_kernel)]
        assert dec(node["ts"]) == dec(nodes[row["kernel_sequence"] - 1]["ts"])
        h0, h1 = dec(host_start["ts"]), dec(host_stop["ts"])
        alloc = [e for e in allocs if h0 <= dec(e["ts"]) < h1 and e["tid"] == host_start["tid"]]
        release = [e for e in frees if h0 <= dec(e["ts"]) < h1 and e["tid"] == host_start["tid"]]
        assert len(alloc) == len(release) == 1
        assert h0 < dec(node["ts"]) < h1 < dec(syncs[i]["ts"])
        inside = [r for r in tasks if r["Device_id"] == "2" and r["stream_id"] == stream
                  and start < dec(r["task_start(us)"]) < stop]
        task_us = dec(task["task_time(us)"])
        row.update({
            "timed_sequence": i + 1, "output_address": address, "stream_id": stream,
            "task_id": op["Task ID"], "task_start_us": str(task_start), "task_stop_us": str(task_stop),
            "task_us": float(task_us), "start_event_task_id": start_id, "stop_event_task_id": stop_id,
            "start_event_time_us": str(start), "stop_event_time_us": str(stop),
            "event_timestamp_span_us": float(stop - start),
            "event_elapsed_error_us": row["device_event_us"] - float(stop - start),
            "pre_task_gap_us": float(task_start - start), "post_task_gap_us": float(stop - task_stop),
            "event_outside_task_us": float(stop - start - task_us),
            "device_tasks_inside_interval": len(inside),
            "device_task_ids_inside_interval": ",".join(r["task_id"] for r in inside),
            "start_event_cid": cid(start_trace), "kernel_cid": cid(device_kernel), "stop_event_cid": cid(stop_trace),
            "host_start_record_us": float(dec(host_start["dur"])),
            "host_runtime_malloc_us": float(dec(alloc[0]["dur"])),
            "host_runtime_malloc_start_us": str(alloc[0]["ts"]),
            "host_node_launch_us": float(dec(node["dur"])),
            "host_acl_free_us": float(dec(release[0]["dur"])),
            "host_acl_free_start_us": str(release[0]["ts"]),
            "host_stop_record_us": float(dec(host_stop["dur"])),
            "host_sync_event_us": float(dec(syncs[i]["dur"])),
        })

    with (OUT / "task-map.tsv").open("w", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=list(rows[0]), delimiter="\t")
        writer.writeheader()
        writer.writerows(rows)

    metrics = ("device_event_us", "task_us", "wall_us", "event_outside_task_us")
    aggregate, differences = {}, {}
    for mode in ("PP", "PC"):
        selected = [r for r in rows if r["study_mode"] == mode]
        peer = "parent_peer" if mode == "PP" else "candidate"
        slices = {"all": selected}
        for side in ("parent", peer):
            slices[side] = [r for r in selected if r["side"] == side]
            for position in (1, 2):
                slices[f"{side}_position{position}"] = [r for r in slices[side] if r["position"] == position]
        for position in (1, 2):
            slices[f"position{position}"] = [r for r in selected if r["position"] == position]
        for switch in (0, 1):
            values = [r for r in selected if r["library_switch"] == switch]
            if values:
                slices[f"library_switch{switch}"] = values
        aggregate[mode] = {k: {m: summarize(v, m) for m in metrics} for k, v in slices.items()}
        comparisons = {}
        for metric in metrics:
            left = [r for r in selected if r["side"] == "parent"]
            right = [r for r in selected if r["side"] == peer]
            a, b = st.median(r[metric] for r in left), st.median(r[metric] for r in right)
            pairs = []
            for offset in range(0, len(selected), 2):
                pair = {r["side"]: r for r in selected[offset:offset+2]}
                pairs.append((pair["parent"][metric], pair[peer][metric]))
            comparisons[metric] = {
                "parent_median_us": a, "peer_median_us": b,
                "ratio_of_medians_percent": (b / a - 1) * 100,
                "median_paired_delta_us": st.median(b1 - a1 for a1, b1 in pairs),
                "median_paired_percent": st.median((b1 / a1 - 1) * 100 for a1, b1 in pairs),
                "block_ratio_percent": {block: (aggregate[mode][peer][metric]["blocks"][block]["median_us"] /
                                                block_stats["median_us"] - 1) * 100
                                        for block, block_stats in aggregate[mode]["parent"][metric]["blocks"].items()},
                "same_position_ratio_percent": {str(p): (
                    aggregate[mode][f"{peer}_position{p}"][metric]["median_us"] /
                    aggregate[mode][f"parent_position{p}"][metric]["median_us"] - 1) * 100 for p in (1, 2)},
            }
        differences[mode] = comparisons

    fields = ("pre_task_gap_us", "post_task_gap_us", "event_outside_task_us", "host_runtime_malloc_us",
              "host_node_launch_us", "host_acl_free_us", "host_start_record_us", "host_stop_record_us", "host_sync_event_us")
    result = {
        "route": "W4-R07", "revision": "V002", "classification": "TIMING_ATTRIBUTION_SUPPLEMENT",
        "status": "MEASUREMENT_BLOCKED", "candidate_execution": "EXECUTED",
        "accepted_local_score_us": None, "accepted_local_delta_percent": None, "current_local_best": None,
        "new_performance_revisions": 0, "valid_numeric_local_results": 0, "stagnation_increment": 0,
        "shape": [128, 12288], "dtype": "fp16", "block_count": 40,
        "device": 2, "libraries": libraries, "output_address": address,
        "reference_before_after": reference,
        "profile_kernel_count": len(ops), "task_time_rows": len(tasks), "timeline_items": len(trace),
        "reference_launches": 4, "warmup_launches": 90, "timed_launches": len(rows),
        "task_event_host_matches": len(rows),
        "max_event_elapsed_error_us": max(abs(r["event_elapsed_error_us"]) for r in rows),
        "host_internal_allocations_matched": len(rows), "host_internal_frees_matched": len(rows),
        "sample_cpu_counts": dict(Counter(r["cpu_after"] for r in rows)),
        "aggregate": aggregate, "differences": differences,
        "intervals": {f: stats(r[f] for r in rows) for f in fields},
        "largest_events_all_retained": sorted(rows, key=lambda r: r["device_event_us"], reverse=True)[:5],
        "largest_tasks_all_retained": sorted(rows, key=lambda r: r["task_us"], reverse=True)[:5],
        "limits": [
            "Only this explicit FP16 input was run again. Other inputs retain their earlier reference results.",
            "Per-library host entry is known from runner metadata; profiler kernel names do not distinguish the two libraries.",
            "Position and previous-library transition are related in PC order; temporal grouping remains observable context.",
            "API and device times can overlap. Nested API durations are not additive explanations of event gaps.",
            "No paired unprofiled run; profiler overhead is not isolated. New task durations do not describe old event samples.",
            "The fixed finite study is for attribution, with no accepted Local or Official result.",
        ],
    }
    dump(OUT / "timing-summary.json", result)
    print(json.dumps({k: result[k] for k in ("status", "profile_kernel_count", "task_event_host_matches",
                                           "max_event_elapsed_error_us", "sample_cpu_counts")}, indent=2))
    for mode in ("PP", "PC"):
        print(mode, "comparisons", json.dumps(differences[mode], indent=2))
        for side in ("all", "parent", "parent_peer" if mode == "PP" else "candidate", "position1", "position2"):
            print(mode, side, {m: {k: aggregate[mode][side][m][k] for k in (
                "median_us", "mad_over_median", "block_medians_us", "block_relative_range")}
                              for m in ("device_event_us", "task_us")})
    print("interval_medians_us", {f: v["median_us"] for f, v in result["intervals"].items()})


if __name__ == "__main__":
    analyze_old()
    analyze_study()
