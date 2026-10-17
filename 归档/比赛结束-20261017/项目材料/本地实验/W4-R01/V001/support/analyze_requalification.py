#!/usr/bin/env python3
"""Summarize the completed R01 requalification without running NPU work."""

import csv
from datetime import datetime
from decimal import Decimal
import json
import math
from pathlib import Path
import re
import statistics
import sys

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


def task_percentile(values, fraction):
    ordered = sorted(values)
    offset = (len(ordered) - 1) * fraction
    left = int(offset)
    right = min(left + 1, len(ordered) - 1)
    return ordered[left] + (ordered[right] - ordered[left]) * (offset - left)


def task_stats(values):
    return {**stats(values), "p10_us": task_percentile(values, 0.10),
            "p90_us": task_percentile(values, 0.90),
            "mad_us": statistics.median(abs(x - statistics.median(values)) for x in values)}


def task_scope(pairs, mapped, field):
    first = [row["P_" + field] for row in pairs]
    second = [row["C_" + field] for row in pairs]
    combined = first + second
    center = statistics.median(combined)
    delta = [b - a for a, b in zip(first, second)]
    groups = []
    side_medians = []
    for number in (1, 2, 3):
        subset = [row for row in pairs if row["group"] == number]
        a = [row["P_" + field] for row in subset]
        b = [row["C_" + field] for row in subset]
        side_medians.extend([statistics.median(a), statistics.median(b)])
        groups.append({"group": number, "P": task_stats(a), "C": task_stats(b),
                       "combined": task_stats(a + b),
                       "median_pair_delta_us": statistics.median(y - x for x, y in zip(a, b)),
                       "median_ratio_delta_percent": (statistics.median(b) / statistics.median(a) - 1) * 100})
    drift = max(abs(value - center) for value in side_medians) / center
    group_centers = [row["combined"]["median_us"] for row in groups]
    group_span = max(group_centers) - min(group_centers)
    floor = max(task_percentile([abs(value) for value in delta], .90), group_span)
    original_limits = all(row[slot]["mad_over_median"] <= .10 for row in groups for slot in ("P", "C")) and drift <= .10
    return {
        "P": task_stats(first), "C": task_stats(second), "combined": task_stats(combined),
        "groups": groups, "group_center_range_us": group_span,
        "group_drift_over_joint_median": drift,
        "both_sides_all_groups_within_original_limits": original_limits,
        "median_ratio_delta_percent": (statistics.median(second) / statistics.median(first) - 1) * 100,
        "median_pair_delta_us": statistics.median(delta),
        "median_pair_delta_percent": statistics.median((b / a - 1) * 100 for a, b in zip(first, second)),
        "pair_abs_delta_p90_us": task_percentile([abs(value) for value in delta], .90),
        "measurement_floor_us": floor,
        "old_target_gap_us": .06,
        "resolves_old_target_gap": original_limits and floor <= .06 and
                                   abs(statistics.median(second) - statistics.median(first)) <= .06,
        "by_order": {order: {
            "pairs": len([row for row in pairs if row["order"] == order]),
            "median_pair_delta_us": statistics.median(row["C_" + field] - row["P_" + field]
                                                        for row in pairs if row["order"] == order),
            "median_pair_delta_percent": statistics.median((row["C_" + field] / row["P_" + field] - 1) * 100
                                                            for row in pairs if row["order"] == order),
        } for order in ("PC", "CP")},
        "by_slot_position": {slot + "_" + str(position): task_stats([
            row[field] for row in mapped if row["slot"] == slot and row["position"] == position])
            for slot in ("P", "C") for position in (1, 2)},
        "by_position": {str(position): task_stats([row[field] for row in mapped if row["position"] == position])
                        for position in (1, 2)},
    }


def task_host_trace(root, calls, profile_prefix="comparison"):
    paths = list(root.glob(f"{profile_prefix}-profile/**/msprof_*.json"))
    assert len(paths) == 1
    trace = json.loads(paths[0].read_text(), parse_float=Decimal)
    hardware = {(str(row["args"]["Physic Stream Id"]), str(row["args"]["Task Id"])): row
                for row in trace if row.get("ph") == "X" and "Physic Stream Id" in row.get("args", {})}
    api = {(row["name"], row["args"]["connection_id"]): row for row in trace
           if row.get("ph") == "X" and row.get("args", {}).get("level") in ("acl", "node", "runtime")}
    allocations = [row for row in trace if row.get("name") == "Runtime@DevMalloc" and row.get("ph") == "X"]
    releases = [row for row in trace if row.get("name") == "AscendCL@aclrtFree" and row.get("ph") == "X"]
    synchronizations = sorted((row for row in trace if row.get("name") == "AscendCL@aclrtSynchronizeEvent"
                              and row.get("ph") == "X"), key=lambda row: Decimal(row["ts"]))
    assert len(synchronizations) == len(calls)
    for call in calls:
        kernel = hardware[(call["stream_id"], call["task_id"])]
        start = hardware[(call["stream_id"], call["start_record_task_id"])]
        stop = hardware[(call["stream_id"], call["stop_record_task_id"])]
        assert Decimal(kernel["ts"]) == Decimal(call["kernel_start_us"])
        assert Decimal(start["ts"]) == Decimal(call["event_start_us"])
        assert Decimal(stop["ts"]) == Decimal(call["event_stop_us"])
        start_api = api[("AscendCL@aclrtRecordEvent", start["args"]["connection_id"])]
        stop_api = api[("AscendCL@aclrtRecordEvent", stop["args"]["connection_id"])]
        node = api[("Node@launch", kernel["args"]["connection_id"])]
        left = Decimal(start_api["ts"]) + start_api["dur"]
        right = Decimal(stop_api["ts"])
        alloc = [row for row in allocations if row["tid"] == int(call["pid"]) and
                 left <= Decimal(row["ts"]) and Decimal(row["ts"]) + row["dur"] <= right]
        free = [row for row in releases if row["tid"] == int(call["pid"]) and
                left <= Decimal(row["ts"]) and Decimal(row["ts"]) + row["dur"] <= right]
        assert len(alloc) == len(free) == 1
        assert left <= Decimal(node["ts"]) <= Decimal(node["ts"]) + node["dur"] <= right
        sync_candidates = [row for row in synchronizations if row["tid"] == int(call["pid"])
                           and Decimal(row["ts"]) >= Decimal(stop_api["ts"]) + stop_api["dur"]]
        assert sync_candidates
        sync = min(sync_candidates, key=lambda row: Decimal(row["ts"]))
        assert sync["tid"] == int(call["pid"])
        assert right + stop_api["dur"] <= Decimal(sync["ts"])
        call.update({
            "start_connection_id": start["args"]["connection_id"],
            "kernel_connection_id": kernel["args"]["connection_id"],
            "stop_connection_id": stop["args"]["connection_id"],
            "host_start_record_start_us": start_api["ts"], "host_start_record_us": float(start_api["dur"]),
            "host_stop_record_start_us": stop_api["ts"], "host_stop_record_us": float(stop_api["dur"]),
            "host_allocation_start_us": alloc[0]["ts"], "host_allocation_us": float(alloc[0]["dur"]),
            "host_free_start_us": free[0]["ts"], "host_free_us": float(free[0]["dur"]),
            "host_node_launch_start_us": node["ts"], "host_node_launch_us": float(node["dur"]),
            "host_event_sync_start_us": sync["ts"], "host_event_sync_us": float(sync["dur"]),
            "host_allocation_count_between_records": 1, "host_free_count_between_records": 1,
        })
        fields = ("host_start_record_us", "host_stop_record_us", "host_allocation_us", "host_free_us",
              "host_node_launch_us", "host_event_sync_us")
    return {
        "source": str(paths[0].relative_to(root)), "trace_event_count": len(trace),
        "matched_timed_calls": len(calls), "allocations_between_host_records": len(calls),
        "frees_between_host_records": len(calls),
        "allocation_bytes": "UNKNOWN", "allocation_object": "UNKNOWN",
        "interpretation": "Calls are inside the host RecordEvent interval. Nested and overlapping host/device durations are not added together.",
        "by_width": {str(width): {field: task_stats([row[field] for row in calls if int(row["width"]) == width])
                                 for field in fields if any(int(row["width"]) == width for row in calls)}
                     for width in (16384, 32768)},
    }


def bootstrap_paired(values, seed=20261008, rounds=5000):
    import random
    randomizer = random.Random(seed)
    by_group = {}
    for row in values:
        by_group.setdefault(row["group"], {}).setdefault(row["cycle"], []).append(row)
    result = []
    groups = sorted(by_group)
    for _ in range(rounds):
        sampled_groups = [randomizer.choice(groups) for _ in groups]
        sample = []
        for group in sampled_groups:
            cycles = sorted(by_group[group])
            for cycle in (randomizer.choice(cycles) for _ in cycles):
                sample.extend(by_group[group][cycle])
        result.append(statistics.median(row["delta_us"] for row in sample))
    return {"low95_us": task_percentile(result, .025), "high95_us": task_percentile(result, .975),
            "seed": seed, "rounds": rounds}


def paired_summary(rows):
    values = [row["delta_us"] for row in rows]
    return {"count": len(rows), "median_delta_us": statistics.median(values),
            "median_delta_percent": statistics.median(row["delta_percent"] for row in rows),
            "interval95": bootstrap_paired(rows)}


def analyze_task_comparison(root):
    def read_rows(path, delimiter=","):
        with path.open() as handle:
            return list(csv.DictReader((line for line in handle if not line.startswith("#")), delimiter=delimiter))

    op_paths = list(root.glob("comparison-profile/**/op_summary_*.csv"))
    task_paths = list(root.glob("comparison-profile/**/task_time_*.csv"))
    assert len(op_paths) == len(task_paths) == 1
    operators = read_rows(op_paths[0])
    operators.sort(key=lambda row: Decimal(row["Task Start Time(us)"].strip()))
    tasks_rows = read_rows(task_paths[0])
    tasks = {(row["Device_id"], row["stream_id"], row["task_id"]): row for row in tasks_rows}
    assert len(tasks) == len(tasks_rows) and len(operators) == 2600
    assert {row["Device_id"] for row in operators} == {"0"}
    assert {row["Task Type"] for row in operators} == {"AI_VECTOR_CORE"}
    assert {row["Block Dim"] for row in operators} == {"8"}
    for op in operators:
        task = tasks[(op["Device_id"], op["Stream ID"], op["Task ID"])]
        assert Decimal(op["Task Start Time(us)"].strip()) == Decimal(task["task_start(us)"].strip())
        assert Decimal(op["Task Duration(us)"]) == Decimal(task["task_time(us)"])
        assert op["Op Name"] == task["kernel_name"]

    log = (root / "comparison-profile.log").read_text()
    session = None
    contexts, refs, correctness = {}, [], []
    moment = "UNKNOWN"
    for line in log.splitlines():
        fields = dict(re.findall(r"(\w+)=([^\s]+)", line))
        if line.startswith("STUDY_SESSION "):
            session = fields
            assert fields["candidate_dispatched"] == "1"
        elif line.startswith("TIMING_CONTEXT "):
            assert fields["pid"] == session["pid"]
            assert fields["parent_function"] != fields["second_function"]
            assert fields["output_P"] != fields["output_C"]
            contexts[int(fields["width"])] = fields
        elif line.startswith("REFERENCE_PHASE="):
            moment = fields["REFERENCE_PHASE"]
        elif line.startswith("REFERENCE "):
            fields["phase"] = moment
            assert fields["result"] == "PASS" and fields["nonfinite"] == "0"
            assert fields["strict_all_elements"] == "FAIL"
            assert int(fields["tolerance_failures"]) == (1 if fields["width"] == "16384" else 3)
            refs.append(fields)
        elif line.startswith("CORRECTNESS "):
            fields["phase"] = moment
            assert fields["result"] == "PASS" and fields["bit_differences"] == "0"
            correctness.append(fields)
    assert len(refs) == 8 and len(correctness) == 4 and set(contexts) == {16384, 32768}

    metadata = {}
    for width in (16384, 32768):
        path = root / f"pcstudy-r16-d{width}-calls.tsv"
        rows = read_rows(path, "\t")
        assert len(rows) == 1300 and {row["pid"] for row in rows} == {session["pid"]}
        metadata[width] = rows
    assert all(row["x"] == metadata[int(row["width"])][0]["x"] and
               row["residual"] == metadata[int(row["width"])][0]["residual"] and
               row["gamma"] == metadata[int(row["width"])][0]["gamma"] and
               row["bias"] == metadata[int(row["width"])][0]["bias"] for width in metadata for row in metadata[width])
    for width_rows in metadata.values():
      for index, call in enumerate(width_rows):
        if index == 0:
            assert call["previous_function_address"] in ("0", "0x0")
            assert call["previous_output_address"] in ("0", "0x0")
            assert call["previous_slot"] == "?"
        else:
            previous = width_rows[index - 1]
            assert call["previous_function_address"] == previous["function_address"]
            assert call["previous_output_address"] == previous["output_address"]
            assert call["previous_slot"] == previous["slot"]

    pp_rows, pc_rows, all_calls, raw_by_stage = [], [], [], {}
    for width in (16384, 32768):
        case_index = 0 if width == 16384 else 1
        ops = operators[case_index * 1300:(case_index + 1) * 1300]
        calls = metadata[width]
        assert len(ops) == len(calls)
        timed_by_phase = {}
        for index, (op, call) in enumerate(zip(ops, calls)):
            key = (op["Device_id"], op["Stream ID"], op["Task ID"])
            task = tasks[key]
            assert int(call["ordinal"]) == index + 1
            function = call["function_address"]
            expected_function = contexts[width]["parent_function"] if call["slot"] == "P" or call["comparison"] == "PP" else contexts[width]["second_function"]
            assert function == expected_function
            output_slot = call["slot"]
            if call["swapped"] == "1":
                output_slot = "C" if output_slot == "P" else "P"
            expected_output = contexts[width]["output_" + output_slot]
            assert call["output_address"] == expected_output
            assert op["Op Name"] == task["kernel_name"]
            row = {**call, "kernel_us": float(task["task_time(us)"]), "task_id": key[2],
                   "stream_id": key[1], "kernel_start_us": task["task_start(us)"],
                   "kernel_stop_us": task["task_stop(us)"], "kernel_name": task["kernel_name"]}
            if call["phase"] == "timed":
                task_id = int(key[2])
                start, stop = tasks[(key[0], key[1], str(task_id - 1))], tasks[(key[0], key[1], str(task_id + 1))]
                assert start["kernel_type"] == stop["kernel_type"] == "EVENT_RECORD"
                begin, end = Decimal(start["task_start(us)"].strip()), Decimal(stop["task_start(us)"].strip())
                kbegin, kend = Decimal(task["task_start(us)"].strip()), Decimal(task["task_stop(us)"].strip())
                assert begin <= kbegin <= kend <= end
                event = float(end - begin)
                row.update({"start_record_task_id": str(task_id - 1), "stop_record_task_id": str(task_id + 1),
                            "event_start_us": str(begin), "event_stop_us": str(end),
                            "event_us": event, "trace_event_start_us": str(begin), "trace_event_stop_us": str(end),
                            "trace_event_interval_us": event, "event_minus_task_us": event - row["kernel_us"]})
                raw = root / f"pcstudy-r16-d{width}-g{call['group']}-{call['comparison'].lower()}.tsv"
                rawrows = raw_by_stage.setdefault(str(raw), read_rows(raw, "\t"))
                sample = rawrows[int(call["pair"]) % 32]
                side = "parent" if call["slot"] == "P" else "candidate"
                row["event_us"] = float(sample[side + "_device_us"])
                row["wall_us"] = float(sample[side + "_wall_us"])
                timed_by_phase.setdefault((int(call["group"]), call["comparison"], int(call["pair"])), {})[call["slot"]] = row
            all_calls.append(row)
        for (group, comparison, pair), sides in timed_by_phase.items():
            assert set(sides) == {"P", "C"}
            pairrow = {"width": width, "group": group, "pair": pair, "comparison": comparison,
                       "cycle": int(sides["P"]["cycle"]), "previous_slot": sides["P"]["previous_slot"],
                       "position": int(sides["P"]["position"]), "swapped": sides["P"]["swapped"] == "1",
                       "P_task_us": sides["P"]["kernel_us"], "C_task_us": sides["C"]["kernel_us"],
                       "P_event_us": sides["P"]["event_us"], "C_event_us": sides["C"]["event_us"],
                       "P_wall_us": sides["P"]["wall_us"], "C_wall_us": sides["C"]["wall_us"]}
            pairrow["delta_us"] = pairrow["C_task_us"] - pairrow["P_task_us"]
            pairrow["delta_percent"] = 100 * pairrow["delta_us"] / pairrow["P_task_us"]
            (pp_rows if comparison == "PP" else pc_rows).append(pairrow)

    assert len(all_calls) == 2600 and sum(row["phase"] == "timed" for row in all_calls) == 1024
    transitions = {}
    for row in metadata[16384]:
        if row["phase"] == "timed":
            key = f"{row['comparison']}:{row['previous_slot']}{row['slot']}:pos{row['position']}:map{'AB' if row['swapped']=='0' else 'BA'}"
            transitions[key] = transitions.get(key, 0) + 1
    for comparison in ("PP", "PC"):
        for previous in "PC":
            for slot in "PC":
                for position in (1, 2):
                    for mapping in ("AB", "BA"):
                        key = f"{comparison}:{previous}{slot}:pos{position}:map{mapping}"
                        assert transitions.get(key, 0) == 16, (key, transitions.get(key))

    host_trace = task_host_trace(root, [row for row in all_calls if row["phase"] == "timed"], "comparison")
    with (root / "all-calls.tsv").open("w", newline="") as handle:
        columns = list(dict.fromkeys(k for row in all_calls for k in row))
        writer = csv.DictWriter(handle, fieldnames=columns, delimiter="\t")
        writer.writeheader()
        writer.writerows(all_calls)
    by_width = {}
    for width in (16384, 32768):
        pps = [row for row in pp_rows if row["width"] == width]
        pcs = [row for row in pc_rows if row["width"] == width]
        adjusted = []
        pc_index = {(row["group"], row["cycle"], row["previous_slot"], row["position"]): row for row in pcs}
        pp_index = {(row["group"], row["cycle"], row["previous_slot"], row["position"]): row for row in pps}
        for key in sorted(pc_index):
            pc, pp = pc_index[key], pp_index[key]
            adj = dict(pc)
            adj["delta_us"] = pc["delta_us"] - pp["delta_us"]
            adj["delta_percent"] = 100 * adj["delta_us"] / pp["P_task_us"]
            adjusted.append(adj)
        def side_stats(rows, field):
            vals = [row[field] for row in rows]
            median = statistics.median(vals)
            group_medians = {str(g): statistics.median(row[field] for row in rows if row["group"] == g)
                             for g in sorted({row["group"] for row in rows})}
            return {**task_stats(vals), "group_medians": group_medians,
                    "group_drift_ratio": max(abs(value-median) for value in group_medians.values())/median}
        def stability_pass(rows):
            return all(side_stats(rows, field)["group_drift_ratio"] <= .10 for field in ("P_task_us", "C_task_us"))
        def mad_groups_pass(rows):
            return all(task_stats([row[field] for row in rows if row["group"] == group])["mad_over_median"] <= .10
                       for group in sorted({row["group"] for row in rows})
                       for field in ("P_task_us", "C_task_us"))
        ppd = [{**row, "delta_us": row["C_task_us"]-row["P_task_us"],
                "delta_percent": 100*(row["C_task_us"]-row["P_task_us"])/row["P_task_us"]} for row in pps]
        pcd = [{**row, "delta_us": row["C_task_us"]-row["P_task_us"],
                "delta_percent": 100*(row["C_task_us"]-row["P_task_us"])/row["P_task_us"]} for row in pcs]
        by_width[str(width)] = {
            "shape": [16,width], "dtype": "fp16", "comparison_samples_per_side": {"PP": len(pps), "PC": len(pcs)},
            "PP": {"P": side_stats(pps,"P_task_us"), "C": side_stats(pps,"C_task_us"),
                   "side_median_delta_us": statistics.median([r["C_task_us"] for r in pps])-statistics.median([r["P_task_us"] for r in pps]),
                   "side_median_delta_percent": 100*(statistics.median([r["C_task_us"] for r in pps])/statistics.median([r["P_task_us"] for r in pps])-1),
                   "paired_delta": paired_summary(ppd), "stability_pass": stability_pass(pps)},
            "PC": {"P": side_stats(pcs,"P_task_us"), "C": side_stats(pcs,"C_task_us"),
                   "side_median_delta_us": statistics.median([r["C_task_us"] for r in pcs])-statistics.median([r["P_task_us"] for r in pcs]),
                   "side_median_delta_percent": 100*(statistics.median([r["C_task_us"] for r in pcs])/statistics.median([r["P_task_us"] for r in pcs])-1),
                   "paired_delta": paired_summary(pcd), "stability_pass": stability_pass(pcs)},
            "PP_device_event": {"P": task_stats([row["P_event_us"] for row in pps]),
                                "C": task_stats([row["C_event_us"] for row in pps]),
                                "paired_delta": paired_summary([{**row, "delta_us": row["C_event_us"]-row["P_event_us"],
                                    "delta_percent": 100*(row["C_event_us"]-row["P_event_us"])/row["P_event_us"]} for row in pps])},
            "PC_device_event": {"P": task_stats([row["P_event_us"] for row in pcs]),
                                "C": task_stats([row["C_event_us"] for row in pcs]),
                                "paired_delta": paired_summary([{**row, "delta_us": row["C_event_us"]-row["P_event_us"],
                                    "delta_percent": 100*(row["C_event_us"]-row["P_event_us"])/row["P_event_us"]} for row in pcs])},
            "PP_wall": {"P": task_stats([row["P_wall_us"] for row in pps]),
                        "C": task_stats([row["C_wall_us"] for row in pps]),
                        "paired_delta": paired_summary([{**row, "delta_us": row["C_wall_us"]-row["P_wall_us"],
                            "delta_percent": 100*(row["C_wall_us"]-row["P_wall_us"])/row["P_wall_us"]} for row in pps])},
            "PC_wall": {"P": task_stats([row["P_wall_us"] for row in pcs]),
                        "C": task_stats([row["C_wall_us"] for row in pcs]),
                        "paired_delta": paired_summary([{**row, "delta_us": row["C_wall_us"]-row["P_wall_us"],
                            "delta_percent": 100*(row["C_wall_us"]-row["P_wall_us"])/row["P_wall_us"]} for row in pcs])},
            "adjusted_PC_minus_PP": paired_summary(adjusted),
            "adjusted_by_stratum": {
                f"prev{prev}_pos{pos}_map{mapping}_stage{stage}": statistics.median(
                    r["delta_us"] for r in adjusted if r["previous_slot"] == prev and r["position"] == pos
                    and ("BA" if r["swapped"] else "AB") == mapping and
                    ((r["group"] in (1,4)) == (stage == "PP_first")))
                for prev in "PC" for pos in (1,2) for mapping in ("AB","BA")
                for stage in ("PP_first","PC_first")},
            "PC_by_group": {str(g): paired_summary([r for r in pcd if r["group"] == g]) for g in range(1,5)},
            "PC_by_stage_order": {"PP_first": paired_summary([r for r in pcd if r["group"] in (1,4)]), "PC_first": paired_summary([r for r in pcd if r["group"] in (2,3)])},
            "PC_by_mapping": {"AB": paired_summary([r for r in pcd if not r["swapped"]]), "BA": paired_summary([r for r in pcd if r["swapped"]])},
            "PC_by_first_cycle": {"cycle0": paired_summary([r for r in pcd if r["cycle"] == 0]),
                                  "cycles1to7": paired_summary([r for r in pcd if r["cycle"] != 0])},
            "parent_group_paired_shift_PC_minus_PP_us": {
                str(g): statistics.median(r["P_task_us"] for r in pcs if r["group"] == g)
                       - statistics.median(r["P_task_us"] for r in pps if r["group"] == g)
                for g in (1,2,3,4)},
            "PP_MAD_limit_pass": mad_groups_pass(pps) and stability_pass(pps),
            "PC_MAD_limit_pass": mad_groups_pass(pcs) and stability_pass(pcs),
            "PP_PC_interval_contains_zero": (lambda ci: ci["low95_us"] <= 0 <= ci["high95_us"])(bootstrap_paired(ppd)),
            "PC_raw_interval_contains_zero": (lambda ci: ci["low95_us"] <= 0 <= ci["high95_us"])(bootstrap_paired(pcd)),
            "PC_adjusted_interval_contains_zero": (lambda ci: ci["low95_us"] <= 0 <= ci["high95_us"])(bootstrap_paired(adjusted)),
        }
    resources = {}
    for prefix in ("compile-pre", "comparison-pre", "comparison-post"):
        lines = (root / f"{prefix}.context.txt").read_text().splitlines()
        usage = (root / f"{prefix}.usages.txt").read_text()
        resources[prefix] = {"utc":lines[0],"host":lines[1],"loadavg":lines[2],"free_hbm_mb":int(lines[3].split("=")[1]),
            "aicore_percent":int(re.search(r"Aicore Usage Rate\(%\)\s*:\s*(\d+)",usage)[1]),
            "aivector_percent":int(re.search(r"Aivector Usage Rate\(%\)\s*:\s*(\d+)",usage)[1])}
    for case in by_width.values():
        pp_ok = case["PP_MAD_limit_pass"] and case["PP_PC_interval_contains_zero"]
        pc_ok = case["PC_MAD_limit_pass"]
        raw_ci = case["PC_raw_interval_contains_zero"]
        adjusted_ci = case["PC_adjusted_interval_contains_zero"]
        raw_direction = case["PC"]["paired_delta"]["median_delta_us"]
        adjusted_direction = case["adjusted_PC_minus_PP"]["median_delta_us"]
        strata = list(case["adjusted_by_stratum"].values())
        case["PP_qualification"] = pp_ok
        case["PC_measurement_qualification"] = pc_ok
        case["effective_local_direction"] = bool(
            pp_ok and pc_ok and not raw_ci and not adjusted_ci and raw_direction < 0
            and adjusted_direction < 0 and all(value < 0 for value in strata))
    result = {"route":"W4-R01","revision":"V001","event_kind":"SAME_REVISION_PP_PC_TASK_STUDY",
        "source_commit":"7f8a335f451d5d7226c38add93d17e5a9b6ad15e","direct_parent":"R31B/V011",
        "branch":"w4/r01-selective-param-pipeline-x",
        "worktree":"/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R01-selective-param-pipeline-x",
        "rule_source_commit":"9f91895506023d917637f707bb3f61cd9d9f8765",
        "shared_state_source_commit":"07662d7b96e9beaaa0f56d97c9cf346b86081eb3",
        "candidate_source_changed":False,"parent_source_changed":False,"device":0,"blocks":8,
        "compile":"HOST_PASS_EXISTING_KERNEL_LIBRARIES_REUSED","correctness":"PASS_TEMPLATE_REFERENCE; STRICT_DIFFERENCES_RETAINED",
        "reference_results":refs,"paired_correctness":correctness,"profile_counts":{"kernel_rows":len(operators),"task_rows":len(tasks_rows),"mapped_calls":len(all_calls),"timed_calls":1024},
        "transitions":transitions,"resources":resources,"host_api_trace":host_trace,"cases":by_width,
        "local_score":None,"local_delta":None,"local_best":"NONE","official_score":None,"online_state":"PAUSED","push":"NO",
        "status":"MEASUREMENT_BLOCKED","valid_local":0,"new_performance_revisions":0,
        "local_score":None,"local_delta":None,"current_local_best":"NONE",
        "official_score":None,"online_state":"PAUSED","push":"NO",
        "duplicate_audit":{"mechanism":"V001 FP16 batchRows>=2 pass2 parameter start slot B; no kernel edit",
            "searched_history":"R31/R31A/R31B; MIX; STORE/EPILOGUE; W3 R2/V040, R4/V031, R5/V028; R01 old call order; R07 bca492a3 timing method",
            "match_found":"V001 exists; same-process task-range real Candidate comparison was not in earlier R01 capture",
            "why_new_or_duplicate":"same-revision measurement supplement; no new performance mechanism"},
        "limitations":["P/P and P/C phases are sequential, not simultaneous; stage order is balanced by group.","Profiler changes timing conditions.","Event and wall values are diagnostic and are not added to nested host API durations.","One planned collection; no follow-up sampling after observing the result."],
        "raw_files":[p.name for p in root.glob("pcstudy-r16-d*-g*-*.tsv")],"all_calls_file":"all-calls.tsv"}
    with (root/"result.json").open("w") as f: json.dump(result,f,ensure_ascii=False,indent=2,allow_nan=False); f.write("\n")
    print(json.dumps({"status":result["status"],"profile_counts":result["profile_counts"],"cases":{
        w:{"PP_parent_median":v["PP"]["P"]["median_us"],"PP_second_median":v["PP"]["C"]["median_us"],
           "PC_parent_median":v["PC"]["P"]["median_us"],"PC_candidate_median":v["PC"]["C"]["median_us"],
           "PP_delta":v["PP"]["paired_delta"],"PC_delta":v["PC"]["paired_delta"],
           "adjusted":v["adjusted_PC_minus_PP"],"PP_qualified":v["PP_qualification"],
           "PC_qualified":v["PC_measurement_qualification"],"effective_local_direction":v["effective_local_direction"]}
        for w,v in by_width.items()}},ensure_ascii=False,indent=2))


def analyze_task_study(root):
    def read_rows(path, delimiter=","):
        with path.open() as handle:
            return list(csv.DictReader((line for line in handle if not line.startswith("#")), delimiter=delimiter))

    op_paths = list(root.glob("pp-profile/**/op_summary_*.csv"))
    task_paths = list(root.glob("pp-profile/**/task_time_*.csv"))
    assert len(op_paths) == len(task_paths) == 1
    operators = read_rows(op_paths[0])
    operators.sort(key=lambda row: Decimal(row["Task Start Time(us)"].strip()))
    task_rows = read_rows(task_paths[0])
    tasks = {(row["Device_id"], row["stream_id"], row["task_id"]): row for row in task_rows}
    assert len(tasks) == len(task_rows)
    assert len(operators) == 1052
    assert {row["Device_id"] for row in operators} == {"0"}
    assert {row["Task Type"] for row in operators} == {"AI_VECTOR_CORE"}
    assert {row["Block Dim"] for row in operators} == {"8"}
    for op in operators:
        task = tasks[(op["Device_id"], op["Stream ID"], op["Task ID"])]
        assert Decimal(op["Task Start Time(us)"].strip()) == Decimal(task["task_start(us)"].strip())
        assert Decimal(op["Task Duration(us)"]) == Decimal(task["task_time(us)"])
        assert op["Op Name"] == task["kernel_name"]

    contexts, references, pair_results = {}, [], []
    moment = "UNKNOWN"
    log = (root / "pp-profile.log").read_text()
    for line in log.splitlines():
        fields = dict(re.findall(r"(\w+)=([^\s]+)", line))
        if line.startswith("STUDY_SESSION "):
            session = fields
            assert fields["candidate_dispatched"] == "0"
        elif line.startswith("TIMING_CONTEXT "):
            assert fields["parent_function"] == fields["second_function"]
            assert fields["output_P"] != fields["output_C"]
            assert fields["pid"] == session["pid"]
            contexts[int(fields["width"])] = fields
        elif line.startswith("REFERENCE_PHASE="):
            moment = fields["REFERENCE_PHASE"]
        elif line.startswith("REFERENCE "):
            fields["phase"] = moment
            assert fields["result"] == "PASS" and fields["nonfinite"] == "0"
            assert fields["strict_all_elements"] == "FAIL"
            assert int(fields["tolerance_failures"]) == (1 if fields["width"] == "16384" else 3)
            assert fields["allowed_mismatch_fraction"] == "0.001"
            references.append(fields)
        elif line.startswith("CORRECTNESS "):
            fields["phase"] = moment
            assert fields["result"] == "PASS" and fields["bit_differences"] == "0"
            pair_results.append(fields)
    assert len(references) == 8 and len(pair_results) == 4
    assert set(contexts) == {16384, 32768}
    assert {(int(row["width"]), row["phase"]) for row in references} == {
        (width, phase) for width in contexts for phase in ("PRE", "POST")}

    all_calls, timed_calls, all_pairs = [], [], []
    for case_index, width in enumerate((16384, 32768)):
        case_ops = operators[case_index * 526:(case_index + 1) * 526]
        assert len({row["Stream ID"] for row in case_ops}) == 1
        sequence = [("correctness", 0, i // 2, "PC", i % 2 + 1, "PC"[i % 2], None) for i in range(4)]
        for group in (1, 2, 3):
            for pair in range(45):
                order = "PC" if pair % 2 == 0 else "CP"
                sequence.extend(("warmup", group, pair, order, position, slot, None)
                                for position, slot in enumerate(order, 1))
            raw_path = root / f"pp-r16-d{width}-b{group}.tsv"
            raw = read_rows(raw_path, "\t")
            assert len(raw) == 42
            for pair, sample in enumerate(raw):
                order = "PC" if pair % 2 == 0 else "CP"
                assert sample["sample"] == str(pair) and sample["order"] == order
                assert sample["parent_completed"] == sample["candidate_completed"] == "1"
                entry = {"width": width, "group": group, "pair": pair, "order": order,
                         "raw_file": raw_path.name}
                all_pairs.append(entry)
                sequence.extend(("timed", group, pair, order, position, slot, (sample, entry))
                                for position, slot in enumerate(order, 1))
        assert len(sequence) == len(case_ops)
        for index, (op, item) in enumerate(zip(case_ops, sequence)):
            phase, group, pair, order, position, slot, sample_entry = item
            key = (op["Device_id"], op["Stream ID"], op["Task ID"])
            task = tasks[key]
            task_start = Decimal(task["task_start(us)"].strip())
            task_stop = Decimal(task["task_stop(us)"].strip())
            entry = {
                "launch_ordinal": case_index * 526 + index + 1,
                "pid": session["pid"], "rows": 16, "width": width, "dtype": "fp16",
                "phase": phase, "group": group, "pair": pair, "order": order,
                "position": position, "slot": slot, "function": "Parent",
                "function_address": contexts[width]["parent_function"],
                "output_address": contexts[width]["output_" + slot],
                "device": 0, "stream_id": key[1], "task_id": key[2], "block_dim": 8,
                "kernel_name": op["Op Name"], "kernel_start_us": str(task_start),
                "kernel_stop_us": str(task_stop), "kernel_us": float(task["task_time(us)"]),
            }
            if phase == "timed":
                sample, pair_entry = sample_entry
                side = "parent" if slot == "P" else "candidate"
                event_us = float(sample[side + "_device_us"])
                wall_us = float(sample[side + "_wall_us"])
                assert math.isfinite(event_us) and event_us > 0 and math.isfinite(wall_us)
                before = tasks[(key[0], key[1], str(int(key[2]) - 1))]
                after = tasks[(key[0], key[1], str(int(key[2]) + 1))]
                assert before["kernel_type"] == after["kernel_type"] == "EVENT_RECORD"
                event_start = Decimal(before["task_start(us)"].strip())
                event_stop = Decimal(after["task_start(us)"].strip())
                assert event_start <= task_start <= task_stop <= event_stop
                entry.update({
                    "timed_ordinal": len(timed_calls) + 1, "event_us": event_us, "wall_us": wall_us,
                    "start_record_task_id": before["task_id"], "stop_record_task_id": after["task_id"],
                    "event_start_us": str(event_start), "event_stop_us": str(event_stop),
                    "trace_interval_us": float(event_stop - event_start),
                    "trace_interval_error_us": float(event_stop - event_start) - event_us,
                    "start_to_kernel_us": float(task_start - event_start),
                    "kernel_to_stop_us": float(event_stop - task_stop),
                    "event_minus_kernel_us": event_us - entry["kernel_us"],
                })
                for field in ("event_us", "kernel_us", "wall_us"):
                    pair_entry[slot + "_" + field] = entry[field]
                timed_calls.append(entry)
            all_calls.append(entry)
    assert len(timed_calls) == 504 and len(all_pairs) == 252
    host_trace = task_host_trace(root, timed_calls)

    with (root / "all-calls.tsv").open("w", newline="") as handle:
        columns = list(dict.fromkeys(key for row in all_calls for key in row))
        writer = csv.DictWriter(handle, fieldnames=columns, delimiter="\t")
        writer.writeheader()
        writer.writerows(all_calls)
    cases = {}
    for width in (16384, 32768):
        pairs = [row for row in all_pairs if row["width"] == width]
        mapped = [row for row in timed_calls if row["width"] == width]
        cases[str(width)] = {
            "shape": [16, width], "dtype": "fp16", "addresses": contexts[width],
            "event": task_scope(pairs, mapped, "event_us"),
            "kernel_task": task_scope(pairs, mapped, "kernel_us"),
            "wall": task_scope(pairs, mapped, "wall_us"),
            "event_minus_kernel_median_us": statistics.median(row["event_minus_kernel_us"] for row in mapped),
            "start_to_kernel_median_us": statistics.median(row["start_to_kernel_us"] for row in mapped),
            "kernel_to_stop_median_us": statistics.median(row["kernel_to_stop_us"] for row in mapped),
            "max_trace_interval_error_us": max(abs(row["trace_interval_error_us"]) for row in mapped),
            "first_sample_each_group": [next(row for row in mapped if row["group"] == group) for group in (1, 2, 3)],
            "largest_kernel_samples": sorted(mapped, key=lambda row: row["kernel_us"], reverse=True)[:5],
        }
    resources = {}
    for prefix in ("compile-pre", "pp-pre", "pp-post"):
        lines = (root / f"{prefix}.context.txt").read_text().splitlines()
        usages = (root / f"{prefix}.usages.txt").read_text()
        resources[prefix] = {
            "utc": lines[0], "host": lines[1], "loadavg": lines[2],
            "free_hbm_mb": int(lines[3].split("=")[1]),
            "aicore_percent": int(re.search(r"Aicore Usage Rate\(%\)\s*:\s*(\d+)", usages)[1]),
            "aivector_percent": int(re.search(r"Aivector Usage Rate\(%\)\s*:\s*(\d+)", usages)[1]),
        }
    result = {
        "route": "W4-R01", "revision": "V001", "event_kind": "SAME_REVISION_TASK_TIME_RESEARCH",
        "source_commit": "93f15d9bab643fa6e8dbf6ed8833a144672d9fb3",
        "rule_source_commit": "9f91895506023d917637f707bb3f61cd9d9f8765",
        "shared_state_source_commit": "07662d7b96e9beaaa0f56d97c9cf346b86081eb3",
        "direct_parent": "R31B/V011", "session": session,
        "compile": "PASS_HOST_ONLY_EXISTING_KERNELS_REUSED",
        "parent_source_changed": False, "candidate_source_changed": False,
        "reference": "unchanged CPU FP64 formula, final FP16; atol=rtol=.001; allowed fraction=.001",
        "reference_results": references, "paired_correctness": pair_results,
        "old_four_input_strict_counts_per_side": [1, 0, 3, 0],
        "old_correctness_source": "93f15d9b:本地实验/W4-R01/V001/requal-device2-20261008/result.json",
        "candidate_execution": "NOT_EXECUTED", "local_score": None, "local_delta": None,
        "current_local_best": "R31B/V011; W4-R01 has no accepted Local Best",
        "new_performance_revisions": 0, "valid_local_results": 0, "stagnation_3_increment": 0,
        "official_score": None, "online_state": "PAUSED", "push": "NO",
        "status": "PARENT_ONLY_RESOLUTION_SUPPORTED" if cases["16384"]["kernel_task"]["resolves_old_target_gap"] else "MEASUREMENT_BLOCKED",
        "counts": {"all_kernel_calls": len(all_calls), "reference_calls": 8, "warmup_calls": 540,
                   "timed_calls": len(timed_calls), "paired_samples": len(all_pairs),
                   "all_task_rows": len(tasks), "all_op_task_pairs_match": True,
                   "all_timed_calls_have_event_brackets": True},
        "mapping_sources": [str(path.relative_to(root)) for path in (op_paths[0], task_paths[0])],
        "host_api_attribution": host_trace,
        "additional_method_source": "1bc84959fbc5dea190ad06a3dcef8b8b3005cae1:本地实验/W4-R08/V001/RETEST-20261008.md",
        "quantile_method": "linear interpolation between adjacent sorted observations",
        "resources": resources, "cases": cases,
        "limitations": [
            "P/P uses one process, both loaded libraries and fixed per-input output addresses; no P/C was dispatched in this process.",
            "Older P/P and P/C were separate processes 529 seconds apart; their task durations and output addresses were not captured.",
            "Profiler affects measurement conditions; task duration is not an isolated active-cycle count.",
            "All tails are retained; event-minus-task is not assigned to a single host API or device cause.",
            "Every timed call contains one runtime allocation and one ACL free between host RecordEvent calls; their object and size remain unknown.",
        ],
    }
    with (root / "result.json").open("w") as handle:
        json.dump(result, handle, ensure_ascii=False, indent=2, allow_nan=False)
        handle.write("\n")
    print(json.dumps({"status": result["status"], "counts": result["counts"],
                      "cases": {width: {scope: {
                          "P": item[scope]["P"]["median_us"], "C": item[scope]["C"]["median_us"],
                          "P_mad_ratio": item[scope]["P"]["mad_over_median"],
                          "C_mad_ratio": item[scope]["C"]["mad_over_median"],
                          "drift": item[scope]["group_drift_over_joint_median"],
                          "floor_us": item[scope]["measurement_floor_us"],
                          "original_center_limits": item[scope]["both_sides_all_groups_within_original_limits"],
                      } for scope in ("event", "kernel_task")} for width, item in cases.items()}}, indent=2))


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
    if len(sys.argv) == 3 and sys.argv[1] == "--task-compare":
        analyze_task_comparison(Path(sys.argv[2]))
    elif len(sys.argv) == 3 and sys.argv[1] == "--task-study":
        analyze_task_study(Path(sys.argv[2]))
    else:
        main()
