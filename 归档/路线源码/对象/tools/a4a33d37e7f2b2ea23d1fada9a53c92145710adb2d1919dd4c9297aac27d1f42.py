"""Summarize all Parent-only samples; no Candidate score is produced."""

import argparse
import csv
import json
import math
import statistics
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
    stdev = statistics.stdev(values) if len(values) > 1 else 0.0
    mad = statistics.median(abs(value - median) for value in values)
    return dict(n=len(values), median=median, mean=mean, stdev=stdev,
                cv=stdev / mean, min=min(values), max=max(values), mad=mad,
                mad_over_median=mad / median,
                p10=quantile(values, 0.1), p90=quantile(values, 0.9))


def analyze(rows, metric):
    all_stats = stats([float(row[metric]) for row in rows])
    blocks = {str(block): stats([float(row[metric]) for row in rows if int(row["block"]) == block])
              for block in sorted({int(row["block"]) for row in rows})}
    sides = {side: stats([float(row[metric]) for row in rows if row["side"] == side])
             for side in ("P1", "P2")}
    pairs = {}
    for row in rows:
        pairs.setdefault((row["block"], row["pair"]), {})[row["side"]] = float(row[metric])
    differences = [pair["P2"] - pair["P1"] for pair in pairs.values()]
    block_medians = [block["median"] for block in blocks.values()]
    spread = max(block_medians) - min(block_medians)
    drift = spread / all_stats["median"]
    qualifies = all_stats["mad_over_median"] <= 0.1 and drift <= 0.1
    return dict(all=all_stats, blocks=blocks, sides=sides,
                block_drift=drift,
                p2_over_p1_percent=(sides["P2"]["median"] / sides["P1"]["median"] - 1) * 100,
                paired_difference_median_us=statistics.median(differences),
                paired_abs_difference_p90_us=quantile([abs(value) for value in differences], 0.9),
                measurement_floor_us=max(quantile([abs(value) for value in differences], 0.9), spread),
                same_binary_qualification="PASS" if qualifies else "MEASUREMENT_BLOCKED")


def attach_tasks(rows, export_dir, map_path):
    op_paths = list(export_dir.glob("op_summary_*.csv"))
    task_paths = list(export_dir.glob("task_time_*.csv"))
    assert len(op_paths) == len(task_paths) == 1
    with op_paths[0].open() as source:
        ops = list(csv.DictReader(source))
    with task_paths[0].open() as source:
        tasks = list(csv.DictReader(source))
    timestamp = lambda value: Decimal(value.replace("\\t", "").strip())
    ops.sort(key=lambda op: timestamp(op["Task Start Time(us)"]))
    assert len(ops) == 185
    assert {op["Device_id"] for op in ops} == {"4"}
    assert {op["Block Dim"] for op in ops} == {"1"}
    assert {op["Task Type"] for op in ops} == {"AI_VECTOR_CORE"}
    assert {op["Op Name"] for op in ops} == {"_Z24add_rms_norm_bias_customIfEvPhS0_S0_S0_S0_mmjff"}
    task_index = {(task["Device_id"], task["stream_id"], task["task_id"]): task for task in tasks}
    assert len(task_index) == len(tasks)
    for op in ops:
        task = task_index[(op["Device_id"], op["Stream ID"], op["Task ID"])]
        assert timestamp(op["Task Start Time(us)"]) == timestamp(task["task_start(us)"])
        assert Decimal(op["Task Duration(us)"]) == Decimal(task["task_time(us)"])

    mapped = []
    for row in rows:
        op = ops[int(row["launch_ordinal"]) - 1]
        key = (op["Device_id"], op["Stream ID"], op["Task ID"])
        task = task_index[key]
        start_record = task_index[(key[0], key[1], str(int(key[2]) - 1))]
        stop_record = task_index[(key[0], key[1], str(int(key[2]) + 1))]
        assert start_record["kernel_type"] == stop_record["kernel_type"] == "EVENT_RECORD"
        event_start = timestamp(start_record["task_start(us)"])
        event_stop = timestamp(stop_record["task_start(us)"])
        task_start = timestamp(task["task_start(us)"])
        task_stop = timestamp(task["task_stop(us)"])
        assert event_start <= task_start <= task_stop <= event_stop
        trace_interval = float(event_stop - event_start)
        interval_error = trace_interval - float(row["device_us"])
        # ACL elapsed time and task timestamps have distinct rounding; retain the difference.
        mapped.append(dict(row, device_id=key[0], stream_id=key[1], task_id=key[2],
                           kernel_start_us=str(task_start), kernel_us=float(task["task_time(us)"]),
                           start_record_task_id=start_record["task_id"],
                           stop_record_task_id=stop_record["task_id"],
                           trace_interval_us=trace_interval, interval_error_us=interval_error,
                           event_minus_task_us=float(row["device_us"]) - float(task["task_time(us)"]),
                           record_to_kernel_start_us=float(task_start - event_start),
                           kernel_end_to_record_us=float(event_stop - task_stop)))
    with map_path.open("x") as target:
        writer = csv.DictWriter(target, fieldnames=list(mapped[0]), delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(mapped)
    return dict(op_summary=str(op_paths[0]), task_time=str(task_paths[0]),
                complete_kernel_calls=len(ops), correctness_calls=1, warmup_calls=60,
                timed_kernel_calls=len(mapped), op_task_matches=185, event_record_brackets=124,
                stream_ids=sorted({op["Stream ID"] for op in ops}), block_dim=1,
                max_event_interval_error_us=max(abs(row["interval_error_us"]) for row in mapped),
                task_map=str(map_path), kernel_task_us=analyze(mapped, "kernel_us"),
                event_minus_task_us=stats([row["event_minus_task_us"] for row in mapped]),
                record_to_kernel_start_us=stats([row["record_to_kernel_start_us"] for row in mapped]),
                kernel_end_to_record_us=stats([row["kernel_end_to_record_us"] for row in mapped]))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("raw", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--profile-export", type=Path)
    args = parser.parse_args()
    with args.raw.open() as source:
        rows = list(csv.DictReader(source, delimiter="\t"))
    assert len(rows) == 124, len(rows)
    assert [int(row["launch_ordinal"]) for row in rows] == list(range(62, 186))
    for row in rows:
        assert math.isfinite(float(row["device_us"])) and float(row["device_us"]) > 0
        assert math.isfinite(float(row["wall_us"])) and float(row["wall_us"]) > 0
    result = dict(route="W4-R12", event_kind="ROUTE_RESEARCH_EVENT", shape=[1, 64], dtype="FP32",
                  candidate=None, local_score=None, local_delta=None,
                  rule="pooled MAD/median <= 0.10 and block-median drift <= 0.10",
                  all_samples_retained=True, raw=str(args.raw),
                  device_event_us=analyze(rows, "device_us"), wall_us=analyze(rows, "wall_us"))
    if args.profile_export:
        result["profile"] = attach_tasks(rows, args.profile_export, args.output.with_suffix(".task-map.tsv"))
    with args.output.open("x") as target:
        json.dump(result, target, indent=2, ensure_ascii=False, allow_nan=False)
        target.write("\n")
    print(json.dumps(result, indent=2, ensure_ascii=False, allow_nan=False))


if __name__ == "__main__":
    main()
