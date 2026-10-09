"""Join R10 event samples to device tasks and host calls without changing inputs."""

import csv
import json
import statistics
import sys
from collections import Counter
from collections import defaultdict
from decimal import Decimal
from pathlib import Path

from analyze import compare, grouped, quantile, read_table, stats, timestamp


def number(value):
    return Decimal(str(value).replace("\\t", "").strip())


def dump_json(path, value):
    with path.open("x") as output:
        json.dump(value, output, ensure_ascii=False, indent=2, allow_nan=False)
        output.write("\n")


def write_table(path, rows):
    with path.open("x") as output:
        writer = csv.DictWriter(output, fieldnames=list(rows[0]), delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def join_calls(rows, export):
    ops = read_table(next(export.glob("op_summary_*.csv")))
    tasks = read_table(next(export.glob("task_time_*.csv")))
    timeline = json.loads(next(export.glob("msprof_*.json")).read_text(), parse_float=Decimal)
    ops.sort(key=lambda row: timestamp(row["Task Start Time(us)"]))
    task_index = {(r["Device_id"], r["stream_id"], r["task_id"]): r for r in tasks}
    hardware = {(str(e["tid"]), str(e["args"]["Task Id"])): e for e in timeline
                if e.get("ph") == "X" and e.get("name", "").startswith("_Z24add_rms")}
    host = sorted((e for e in timeline if e.get("ph") == "X" and
                   e.get("name", "").startswith(("Node@", "Runtime@", "AscendCL@"))),
                  key=lambda e: number(e["ts"]))
    nodes = {e["args"]["connection_id"]: e for e in host if e["name"] == "Node@launch"}
    records = [e for e in host if e["name"] == "AscendCL@aclrtRecordEvent"]
    syncs = [e for e in host if e["name"] == "AscendCL@aclrtSynchronizeEvent"]
    assert len(records) == 2 * len(rows) and len(syncs) == len(rows)
    assert len(ops) == len(hardware) == len(nodes)
    assert len(task_index) == len(tasks)
    for op in ops:
        key = (op["Device_id"], op["Stream ID"], op["Task ID"])
        task = task_index[key]
        assert timestamp(op["Task Start Time(us)"]) == timestamp(task["task_start(us)"])
        assert number(op["Task Duration(us)"]) == number(task["task_time(us)"])
    mapped = []
    for index, row in enumerate(rows):
        op = ops[int(row["launch_ordinal"]) - 1]
        key = (op["Device_id"], op["Stream ID"], op["Task ID"])
        task = task_index[key]
        start = task_index[(key[0], key[1], str(int(key[2]) - 1))]
        stop = task_index[(key[0], key[1], str(int(key[2]) + 1))]
        assert start["kernel_type"] == stop["kernel_type"] == "EVENT_RECORD"
        begin, end = timestamp(task["task_start(us)"]), timestamp(task["task_stop(us)"])
        begin_event, end_event = timestamp(start["task_start(us)"]), timestamp(stop["task_start(us)"])
        assert begin_event <= begin <= end <= end_event
        hw = hardware[(key[1], key[2])]
        node = nodes[hw["args"]["connection_id"]]
        assert number(hw["ts"]) == begin
        before, after = records[2 * index:2 * index + 2]
        assert number(before["ts"]) <= number(node["ts"]) <= number(after["ts"])
        apis = [e for e in host if number(before["ts"]) <= number(e["ts"]) <= number(after["ts"])]
        allocations = [e for e in apis if e["name"] == "Runtime@DevMalloc"]
        releases = [e for e in apis if e["name"] == "AscendCL@aclrtFree"]
        launches = [e for e in apis if e["name"] == "Runtime@KernelLaunch"]
        assert len(launches) == 1
        runtime_launch = launches[0]
        previous_op = ops[int(row["launch_ordinal"]) - 2] if int(row["launch_ordinal"]) > 1 else None
        merged = dict(row)
        merged.update(
            device_id=key[0], stream_id=key[1], task_id=key[2], block_dim=op["Block Dim"],
            kernel_start_us=str(begin), kernel_stop_us=str(end), kernel_us=float(number(task["task_time(us)"])),
            start_record_task_id=start["task_id"], stop_record_task_id=stop["task_id"],
            start_record_us=str(begin_event), stop_record_us=str(end_event),
            start_record_duration_us=float(number(start["task_time(us)"])),
            stop_record_duration_us=float(number(stop["task_time(us)"])),
            start_record_to_kernel_us=float(begin - begin_event),
            kernel_to_stop_record_us=float(end_event - end),
            trace_interval_us=float(end_event - begin_event),
            interval_error_us=float(end_event - begin_event) - float(row["device_us"]),
            event_minus_kernel_us=float(row["device_us"]) - float(end - begin),
            trace_event_outside_kernel_us=float((end_event - begin_event) - (end - begin)),
            host_connection_id=hw["args"]["connection_id"], host_node_start_us=str(node["ts"]),
            host_node_us=float(node["dur"]), host_runtime_launch_us=float(runtime_launch["dur"]),
            host_record_start_us=float(before["dur"]), host_record_stop_us=float(after["dur"]),
            host_sync_us=float(syncs[index]["dur"]),
            host_alloc_count=len(allocations), host_free_count=len(releases),
            host_alloc_us=sum(float(e["dur"]) for e in allocations),
            host_free_us=sum(float(e["dur"]) for e in releases),
            device_start_minus_host_node_us=float(begin - number(node["ts"])),
            device_start_minus_host_runtime_us=float(begin - number(runtime_launch["ts"])),
            previous_task_id=previous_op["Task ID"] if previous_op else "NONE",
            previous_kernel_to_current_us=float(begin - timestamp(previous_op["Task Start Time(us)"])
                                                - number(previous_op["Task Duration(us)"])) if previous_op else 0,
        )
        mapped.append(merged)
    ordered_nodes = sorted(nodes.values(), key=lambda e: number(e["ts"]))
    registrations = [dict(host_start_us=str(e["ts"]), duration_us=float(e["dur"]),
                          before_launch_ordinal=1 + sum(number(n["ts"]) < number(e["ts"]) for n in ordered_nodes))
                     for e in host if e["name"] == "Runtime@DevBinaryRegister"]
    return mapped, dict(kernel_calls=len(ops), task_rows=len(tasks), timeline_items=len(timeline),
                        timed_calls=len(rows), all_op_task_pairs_match=True,
                        all_timed_host_connection_ids_match=True, binary_registrations=registrations,
                        device_before_host_node=sum(r["device_start_minus_host_node_us"] < 0 for r in mapped),
                        device_before_runtime_launch=sum(r["device_start_minus_host_runtime_us"] < 0 for r in mapped),
                        allocation_count_distribution=dict(Counter(r["host_alloc_count"] for r in mapped)),
                        free_count_distribution=dict(Counter(r["host_free_count"] for r in mapped)),
                        max_event_interval_error_us=max(abs(r["interval_error_us"]) for r in mapped),
                        export=str(export))


def metric_medians(rows):
    columns = ("kernel_us", "device_us", "wall_us", "start_record_to_kernel_us", "kernel_to_stop_record_us",
               "trace_event_outside_kernel_us", "host_alloc_us", "host_free_us", "host_node_us",
               "host_runtime_launch_us", "host_record_start_us", "host_record_stop_us", "host_sync_us")
    return {key: statistics.median(float(r[key]) for r in rows) for key in columns}


def old_analysis(root, output):
    result = dict(route="W4-R10", revision="V002", source_commit="1441fc7297b37d37a4af67d0775cb49f37f5956d",
                  shapes={}, clocks="Host/device absolute ordering is not treated as causal proof.")
    for name in ("target", "control"):
        rows = read_table(root / f"profile-{name}.task-map.tsv", "\t")
        export = next((root / f"profile-{name}-capture").glob("PROF*/mindstudio_profiler_output"))
        mapped, evidence = join_calls(rows, export)
        libraries = {i: "P" for i in range(1, 185)}
        libraries.update({i: "P" if i % 2 else "C" for i in range(185, 305)})
        libraries.update({int(r["launch_ordinal"]): r["side"] for r in rows if r["phase"] == "PC"})
        for row in mapped:
            library = "P" if row["phase"] == "PP" else row["side"]
            previous = libraries[int(row["launch_ordinal"]) - 1]
            row.update(actual_library=library, previous_library=previous,
                       repeats_previous_library=library == previous, output_slot="SHARED")
        write_table(output / f"old-{name}-attribution.tsv", mapped)
        phases = {}
        for phase in ("PP", "PC"):
            part = [r for r in mapped if r["phase"] == phase]
            phases[phase] = dict(medians=metric_medians(part),
                positions={str(position): dict(n=sum(int(r["position"]) == position for r in part),
                    repeats_previous_library=sum(int(r["position"]) == position and r["repeats_previous_library"] for r in part),
                    medians=metric_medians([r for r in part if int(r["position"]) == position])) for position in (0, 1)},
                comparison=compare(part, "kernel_us", ("P1", "P2") if phase == "PP" else ("P", "C")),
                largest_kernel_tasks=sorted(part, key=lambda r: r["kernel_us"], reverse=True)[:5],
                largest_outside_kernel_intervals=sorted(part, key=lambda r: r["trace_event_outside_kernel_us"], reverse=True)[:5])
        result["shapes"][name] = dict(evidence=evidence, phases=phases)
        print(json.dumps(dict(shape=name, evidence=evidence,
                              phase_medians={k: v["medians"] for k, v in phases.items()},
                              positions={k: v["positions"] for k, v in phases.items()}), ensure_ascii=False))
    dump_json(output / "old-attribution-summary.json", result)


def differences(values):
    return dict(n=len(values), median_us=statistics.median(values), mean_us=statistics.mean(values),
                min_us=min(values), max_us=max(values), p10_us=quantile(values, 0.1),
                p90_us=quantile(values, 0.9), positive=sum(v > 0 for v in values),
                negative=sum(v < 0 for v in values), zero=sum(v == 0 for v in values))


def diagnostic_pairs(rows, phase):
    labels = ("P1", "P2") if phase == "PP" else ("P", "C")
    groups = defaultdict(dict)
    for row in rows:
        groups[(row["block"], row["pair"])][row["side"]] = row
    pairs = []
    for (block, pair), group in groups.items():
        p, c = (group[label] for label in labels)
        assert p["cell"] == c["cell"]
        result = dict(phase=phase, block=block, pair=pair, cell=p["cell"],
                      p_ordinal=p["launch_ordinal"], c_ordinal=c["launch_ordinal"],
                      condition_library=p["condition_library"], condition_other_output=p["condition_other_output"],
                      c_output=c["output_slot"], c_slot=c["logical_slot"],
                      first_side=p["side"] if int(p["position"]) == 0 else c["side"],
                      address_swap=p["address_swap"], kernel_swap=p["kernel_swap"])
        for metric in ("kernel_us", "device_us", "wall_us"):
            left, right = float(p[metric]), float(c[metric])
            result.update({f"p_{metric}": left, f"c_{metric}": right,
                           f"difference_{metric}": right - left,
                           f"delta_percent_{metric}": (right / left - 1) * 100})
        pairs.append(result)
    return pairs


def factorial_contrasts(rows, phase):
    axes = ("condition_library", "condition_other_output", "output_slot", "position", "logical_slot")
    contrasts = []
    for axis in axes:
        held = [name for name in axes if name != axis]
        groups = defaultdict(list)
        for row in rows:
            key = (row["block"], row["side"], *(row[name] for name in held))
            groups[key].append(row)
        for key, pair in groups.items():
            assert len(pair) == 2, (axis, key, len(pair))
            low, high = sorted(pair, key=lambda row: (row[axis] == "C") if axis == "condition_library" else row[axis])
            assert low[axis] != high[axis]
            result = dict(phase=phase, axis=axis, block=key[0], side=key[1],
                          low_level=low[axis], high_level=high[axis],
                          low_ordinal=low["launch_ordinal"], high_ordinal=high["launch_ordinal"])
            result.update({f"held_{name}": low[name] for name in axes})
            result[f"held_{axis}"] = "VARIED"
            for metric in ("kernel_us", "device_us", "wall_us"):
                result[f"low_{metric}"] = float(low[metric])
                result[f"high_{metric}"] = float(high[metric])
                result[f"difference_{metric}"] = float(high[metric]) - float(low[metric])
            contrasts.append(result)
    return contrasts


def diagnostic_analysis(root):
    result = dict(route="W4-R10", revision="V002", new_performance_revisions=0,
                  accepted_local_score=None, accepted_local_delta=None, shapes={})
    for name in ("target", "control"):
        original_rows = []
        for phase in ("PP", "PC"):
            rows = read_table(root / f"{name}.{phase.lower()}.tsv", "\t")
            assert len(rows) == 256
            for row in rows:
                row["phase"] = phase
            original_rows.extend(rows)
        assert [int(r["launch_ordinal"]) for r in original_rows] == list(range(125, 381)) + list(range(501, 757))
        export = next((root / f"{name}-capture").glob("PROF*/mindstudio_profiler_output"))
        mapped, evidence = join_calls(original_rows, export)
        assert evidence["kernel_calls"] == 756
        assert [r["before_launch_ordinal"] for r in evidence["binary_registrations"]] == [1, 3]
        for row in mapped:
            expected = "32" if name == "target" and row["actual_library"] == "C" else "40"
            assert row["block_dim"] == expected
        by_ordinal = {int(row["launch_ordinal"]): row for row in mapped}
        for row in mapped:
            if row["role"] != "probe":
                continue
            previous = by_ordinal[int(row["previous_ordinal"])]
            assert int(row["previous_ordinal"]) == int(row["launch_ordinal"]) - 1
            assert previous["role"] == "conditioner"
            assert row["previous_library"] == row["condition_library"] == previous["actual_library"]
            assert row["previous_output_slot"] == previous["output_slot"]
            assert (row["output_slot"] != previous["output_slot"]) == bool(int(row["condition_other_output"]))
            assert row["previous_task_id"] == previous["task_id"]
            if row["phase"] == "PP":
                assert row["actual_library"] == "P"
        addresses = {label: {r["output_address"] for r in mapped if r["output_slot"] == label} for label in ("A", "B")}
        assert all(len(values) == 1 for values in addresses.values())
        assert addresses["A"] != addresses["B"]
        reference = read_table(root / f"{name}.reference.tsv", "\t")
        assert len(reference) == (128 if name == "target" else 48) * 6
        assert all(int(r["failures"]) == int(r["nonfinite"]) == 0 for r in reference)
        reference_summary = {
            label: dict(elements=sum(int(r["elements"]) for r in reference if r["side"] == label),
                        failures=0, nonfinite=0,
                        max_abs=max(float(r["max_abs"]) for r in reference if r["side"] == label))
            for label in sorted({r["side"] for r in reference})}
        write_table(root / f"{name}-task-map.tsv", mapped)
        phases, all_pairs, all_contrasts = {}, [], []
        for phase in ("PP", "PC"):
            probes = [r for r in mapped if r["phase"] == phase and r["role"] == "probe"]
            assert len(probes) == 128
            coverage = Counter((r["block"], r["cell"]) for r in probes)
            assert len(coverage) == 64 and set(coverage.values()) == {2}
            labels = ("P1", "P2") if phase == "PP" else ("P", "C")
            pairs = diagnostic_pairs(probes, phase)
            contrasts = factorial_contrasts(probes, phase)
            all_pairs.extend(pairs)
            all_contrasts.extend(contrasts)
            metrics = {}
            for metric in ("kernel_us", "device_us", "wall_us"):
                comparison = compare(probes, metric, labels)
                strata = {axis: {level: grouped([r for r in probes if r[axis] == level], metric)
                                  for level in sorted({r[axis] for r in probes})}
                          for axis in ("logical_slot", "output_slot", "position", "condition_library", "condition_other_output")}
                required = [comparison, *comparison["sides"].values(),
                            *strata["logical_slot"].values(), *strata["output_slot"].values()]
                qualified = all(part["stability"] == "PASS" for part in required)
                pair_groups = {axis: {level: dict(
                    n=sum(p[axis] == level for p in pairs),
                    difference_median_us=statistics.median(p[f"difference_{metric}"] for p in pairs if p[axis] == level),
                    delta_median_percent=statistics.median(p[f"delta_percent_{metric}"] for p in pairs if p[axis] == level))
                    for level in sorted({p[axis] for p in pairs})}
                    for axis in ("condition_library", "condition_other_output", "c_output", "c_slot", "first_side", "block")}
                contrast_summary = {}
                for axis in sorted({r["axis"] for r in contrasts}):
                    contrast_summary[axis] = {}
                    for label in labels:
                        part = [r for r in contrasts if r["axis"] == axis and r["side"] == label]
                        contrast_summary[axis][label] = dict(
                            **differences([r[f"difference_{metric}"] for r in part]),
                            by_block={str(b): differences([r[f"difference_{metric}"] for r in part if int(r["block"]) == b]) for b in (0, 1)})
                metrics[metric] = dict(comparison=comparison, strata=strata, qualified=qualified,
                                       pair_groups=pair_groups, contrasts=contrast_summary)
            phases[phase] = dict(metrics=metrics, medians=metric_medians(probes),
                                 largest_kernel_tasks=sorted(probes, key=lambda r: r["kernel_us"], reverse=True)[:5],
                                 largest_outside_kernel_intervals=sorted(probes, key=lambda r: r["trace_event_outside_kernel_us"], reverse=True)[:5])
        write_table(root / f"{name}-pairs.tsv", all_pairs)
        write_table(root / f"{name}-contrasts.tsv", all_contrasts)
        result["shapes"][name] = dict(shape=[128 if name == "target" else 48, 12288], dtype="BF16",
            evidence=evidence, addresses={key: sorted(values) for key, values in addresses.items()},
            reference=reference_summary, phases=phases)
        print(json.dumps(dict(shape=name, evidence=evidence, addresses=result["shapes"][name]["addresses"],
            pp=phases["PP"]["metrics"]["kernel_us"]["comparison"],
            pc=phases["PC"]["metrics"]["kernel_us"]["comparison"],
            pp_qualified=phases["PP"]["metrics"]["kernel_us"]["qualified"],
            pc_qualified=phases["PC"]["metrics"]["kernel_us"]["qualified"]), ensure_ascii=False))
    reused = []
    for path in sorted(root.parent.glob("correctness-*.reference.tsv")):
        rows = read_table(path, "\t")
        assert all(int(r["failures"]) == int(r["nonfinite"]) == 0 for r in rows)
        reused.append(dict(path=str(path), sides=sorted({r["side"] for r in rows}),
                           element_comparisons=sum(int(r["elements"]) for r in rows),
                           failures=0, nonfinite=0, max_abs=max(float(r["max_abs"]) for r in rows)))
    assert len(reused) == 8
    result["reused_original_reference"] = reused
    dump_json(root / "diagnostic-summary.json", result)


if __name__ == "__main__":
    if sys.argv[1] == "old":
        old_analysis(Path(sys.argv[2]), Path(sys.argv[3]))
    elif sys.argv[1] == "diagnostic":
        diagnostic_analysis(Path(sys.argv[2]))
    else:
        raise SystemExit("expected: old RESULTS OUTPUT | diagnostic RESULTS")
