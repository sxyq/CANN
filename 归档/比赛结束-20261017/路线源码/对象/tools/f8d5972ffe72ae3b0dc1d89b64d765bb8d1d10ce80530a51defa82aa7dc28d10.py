"""Retain all R10 samples and align each timed launch with its kernel task."""

import csv
import json
import math
import statistics
import sys
from decimal import Decimal
from pathlib import Path


def quantile(values, fraction):
    ordered = sorted(values)
    position = (len(ordered) - 1) * fraction
    lower = int(position)
    upper = min(lower + 1, len(ordered) - 1)
    weight = position - lower
    return ordered[lower] * (1 - weight) + ordered[upper] * weight


def stats(values):
    median = statistics.median(values)
    mean = statistics.mean(values)
    stdev = statistics.stdev(values)
    mad = statistics.median(abs(value - median) for value in values)
    return dict(n=len(values), median_us=median, mean_us=mean, stdev_us=stdev,
                cv=stdev / mean, min_us=min(values), max_us=max(values), mad_us=mad,
                mad_over_median=mad / median,
                p10_us=quantile(values, 0.1), p90_us=quantile(values, 0.9))


def grouped(rows, metric):
    pooled = stats([float(row[metric]) for row in rows])
    blocks = {str(block): stats([float(row[metric]) for row in rows if int(row["block"]) == block])
              for block in (0, 1)}
    drift = abs(blocks["1"]["median_us"] - blocks["0"]["median_us"]) / pooled["median_us"]
    return dict(pooled=pooled, blocks=blocks, block_drift=drift,
                stability="PASS" if pooled["mad_over_median"] <= 0.1 and drift <= 0.1 else "MEASUREMENT_BLOCKED")


def compare(rows, metric, labels):
    pooled = grouped(rows, metric)
    sides = {label: grouped([row for row in rows if row["side"] == label], metric) for label in labels}
    pairs = {}
    for row in rows:
        pair = pairs.setdefault((row["block"], row["pair"]), {})
        pair[row["side"]] = float(row[metric])
        if int(row["position"]) == 0:
            pair["first"] = row["side"]
    differences = [pair[labels[1]] - pair[labels[0]] for pair in pairs.values()]
    deltas = [(pair[labels[1]] / pair[labels[0]] - 1) * 100 for pair in pairs.values()]
    order = {label: {
        "difference_median_us": statistics.median(pair[labels[1]] - pair[labels[0]] for pair in pairs.values() if pair["first"] == label),
        "paired_delta_median_percent": statistics.median((pair[labels[1]] / pair[labels[0]] - 1) * 100 for pair in pairs.values() if pair["first"] == label),
    } for label in labels}
    block_differences = {
        str(block): statistics.median(pair[labels[1]] - pair[labels[0]] for key, pair in pairs.items() if int(key[0]) == block)
        for block in (0, 1)
    }
    spread = abs(pooled["blocks"]["1"]["median_us"] - pooled["blocks"]["0"]["median_us"])
    return dict(**pooled, sides=sides,
                median_ratio_delta_percent=(sides[labels[1]]["pooled"]["median_us"] / sides[labels[0]]["pooled"]["median_us"] - 1) * 100,
                paired_delta_median_percent=statistics.median(deltas),
                paired_difference_median_us=statistics.median(differences),
                paired_abs_difference_p90_us=quantile([abs(value) for value in differences], 0.9),
                pp_resolution_us=max(quantile([abs(value) for value in differences], 0.9), spread) if labels[0] == "P1" else None,
                by_first_side=order, block_difference_medians_us=block_differences)


def read_table(path, delimiter=","):
    with path.open() as source:
        return list(csv.DictReader(source, delimiter=delimiter))


def timestamp(value):
    return Decimal(value.replace("\\t", "").strip())


def map_tasks(pp, pc, export, destination, target):
    op_paths = list(export.glob("op_summary_*.csv"))
    task_paths = list(export.glob("task_time_*.csv"))
    assert len(op_paths) == len(task_paths) == 1
    ops = read_table(op_paths[0])
    tasks = read_table(task_paths[0])
    ops.sort(key=lambda op: timestamp(op["Task Start Time(us)"]))
    assert len(ops) == 428, len(ops)
    assert {op["Device_id"] for op in ops} == {"4"}
    assert {op["Task Type"] for op in ops} == {"AI_VECTOR_CORE"}
    assert all("add_rms_norm_bias_custom" in op["Op Name"] for op in ops)
    task_index = {(task["Device_id"], task["stream_id"], task["task_id"]): task for task in tasks}
    assert len(task_index) == len(tasks)
    for op in ops:
        task = task_index[(op["Device_id"], op["Stream ID"], op["Task ID"])]
        assert timestamp(op["Task Start Time(us)"]) == timestamp(task["task_start(us)"])
        assert Decimal(op["Task Duration(us)"]) == Decimal(task["task_time(us)"])
    mapped = []
    for phase, rows in (("PP", pp), ("PC", pc)):
        for row in rows:
            op = ops[int(row["launch_ordinal"]) - 1]
            key = (op["Device_id"], op["Stream ID"], op["Task ID"])
            task = task_index[key]
            start = task_index[(key[0], key[1], str(int(key[2]) - 1))]
            stop = task_index[(key[0], key[1], str(int(key[2]) + 1))]
            assert start["kernel_type"] == stop["kernel_type"] == "EVENT_RECORD"
            event_start, event_stop = timestamp(start["task_start(us)"]), timestamp(stop["task_start(us)"])
            task_start, task_stop = timestamp(task["task_start(us)"]), timestamp(task["task_stop(us)"])
            assert event_start <= task_start <= task_stop <= event_stop
            expected_blocks = "32" if target and row["side"] == "C" else "40"
            assert op["Block Dim"] == expected_blocks, (row, op["Block Dim"])
            mapped.append(dict(row, phase=phase, device_id=key[0], stream_id=key[1], task_id=key[2],
                               block_dim=op["Block Dim"], kernel_start_us=str(task_start),
                               kernel_us=float(task["task_time(us)"]),
                               start_record_task_id=start["task_id"], stop_record_task_id=stop["task_id"],
                               trace_interval_us=float(event_stop - event_start),
                               interval_error_us=float(event_stop - event_start) - float(row["device_us"]),
                               event_minus_kernel_us=float(row["device_us"]) - float(task["task_time(us)"])))
    with destination.open("x") as output:
        writer = csv.DictWriter(output, fieldnames=list(mapped[0]), delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(mapped)
    return mapped, dict(complete_kernel_calls=len(ops), task_rows=len(tasks),
                        all_op_task_pairs_match=True, timed_calls=len(mapped),
                        event_brackets=len(mapped), warmup_calls=180,
                        actual_block_dims=sorted({op["Block Dim"] for op in ops}),
                        max_event_interval_error_us=max(abs(row["interval_error_us"]) for row in mapped),
                        event_minus_kernel_median_us=statistics.median(row["event_minus_kernel_us"] for row in mapped),
                        op_summary=str(op_paths[0]), task_time=str(task_paths[0]), task_map=str(destination))


def main():
    root = Path(sys.argv[1])
    result = dict(route="W4-R10", revision="V002", agent_id="01a119de-51ab-7810-9515-c9c388f70ac3",
                  all_samples_retained=True, primary_metric="kernel_task_us", shapes={})
    for name, rows in (("target", 128), ("control", 48)):
        prefix = root / f"profile-{name}"
        pp = read_table(prefix.with_suffix(".pp.tsv"), "\t")
        pc = read_table(prefix.with_suffix(".pc.tsv"), "\t")
        assert len(pp) == len(pc) == 124
        assert [int(row["launch_ordinal"]) for row in pp] == list(range(61, 185))
        assert [int(row["launch_ordinal"]) for row in pc] == list(range(305, 429))
        for row in pp + pc:
            assert all(math.isfinite(float(row[key])) and float(row[key]) > 0 for key in ("device_us", "wall_us"))
        exports = list((root / f"profile-{name}-capture").glob("PROF*/mindstudio_profiler_output"))
        assert len(exports) == 1, exports
        mapped, evidence = map_tasks(pp, pc, exports[0], root / f"profile-{name}.task-map.tsv", name == "target")
        task_pp = [row for row in mapped if row["phase"] == "PP"]
        task_pc = [row for row in mapped if row["phase"] == "PC"]
        result["shapes"][name] = dict(shape=[rows, 12288], dtype="BF16", profile=evidence,
            pp=dict(kernel_task=compare(task_pp, "kernel_us", ("P1", "P2")),
                    event=compare(pp, "device_us", ("P1", "P2")), wall=compare(pp, "wall_us", ("P1", "P2"))),
            pc=dict(kernel_task=compare(task_pc, "kernel_us", ("P", "C")),
                    event=compare(pc, "device_us", ("P", "C")), wall=compare(pc, "wall_us", ("P", "C"))))
    with (root / "local-summary.json").open("x") as output:
        json.dump(result, output, ensure_ascii=False, indent=2, allow_nan=False)
        output.write("\n")
    for name, shape in result["shapes"].items():
        print(json.dumps(dict(name=name, pp=shape["pp"]["kernel_task"], pc=shape["pc"]["kernel_task"], profile=shape["profile"]), ensure_ascii=False))


if __name__ == "__main__":
    main()
