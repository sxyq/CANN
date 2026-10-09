#!/usr/bin/env python3
"""Align the unchanged R14 Parent's complete call sequence and retain all samples."""

import csv
import io
import json
import math
import re
import statistics
from decimal import Decimal
from pathlib import Path


ROOT = Path(__file__).resolve().parent
CAPTURE = ROOT / "task-time-20261008"
CASES = ((80, 8192), (120, 6144))


def read_rows(path, delimiter=","):
    with path.open(newline="") as stream:
        return list(csv.DictReader(stream, delimiter=delimiter))


def timestamp(value):
    return Decimal(value.replace("\\t", "").strip())


def quantile(values, fraction):
    ordered = sorted(values)
    position = (len(ordered) - 1) * fraction
    lower, upper = math.floor(position), math.ceil(position)
    return ordered[lower] + (ordered[upper] - ordered[lower]) * (position - lower)


def stats(values):
    center = statistics.median(values)
    mean = statistics.mean(values)
    deviation = statistics.stdev(values)
    mad = statistics.median(abs(value - center) for value in values)
    return dict(n=len(values), median_us=center, mean_us=mean, stdev_us=deviation,
                cv=deviation / abs(mean) if mean else None,
                mad_us=mad, mad_over_median=mad / abs(center) if center else None,
                p10_us=quantile(values, 0.1), p90_us=quantile(values, 0.9),
                min_us=min(values), max_us=max(values))


def summarize(samples, metric):
    pooled = stats([row[metric] for row in samples])
    sides = {side: stats([row[metric] for row in samples if row["side"] == side])
             for side in ("P1", "P2")}
    blocks = {str(block): stats([row[metric] for row in samples if row["block"] == block])
              for block in (0, 1)}
    pairs = {}
    for row in samples:
        pair = pairs.setdefault((row["block"], row["pair"]), {})
        pair[row["side"]] = row[metric]
        pair["order"] = row["order"]
    differences = [pair["P2"] - pair["P1"] for pair in pairs.values()]
    spread = abs(blocks["1"]["median_us"] - blocks["0"]["median_us"])
    absolute_p90 = quantile([abs(value) for value in differences], 0.9)
    drift = spread / pooled["median_us"]
    return dict(pooled=pooled, sides=sides, blocks=blocks, block_drift=drift,
                block_median_spread_us=spread,
                paired_difference=stats(differences), paired_abs_difference_p90_us=absolute_p90,
                measurement_floor_us=max(absolute_p90, spread),
                p2_over_p1_median_percent=100 * (sides["P2"]["median_us"] / sides["P1"]["median_us"] - 1),
                order_difference_median_us={
                    order: statistics.median(pair["P2"] - pair["P1"] for pair in pairs.values()
                                             if pair["order"] == order)
                    for order in ("P1-P2", "P2-P1")},
                same_binary_qualification="PASS" if pooled["mad_over_median"] <= 0.1 and drift <= 0.1
                else "MEASUREMENT_BLOCKED")


def load_context(directory, phase):
    text = (directory / f"{phase}.usages.txt").read_text()
    capacity = int(re.search(r"HBM Capacity\(MB\)\s*:\s*(\d+)", text)[1])
    usage = int(re.search(r"^\s*HBM Usage Rate\(%\)\s*:\s*(\d+)", text, re.M)[1])
    return dict(free_hbm_mb=capacity * (100 - usage) // 100,
                aicore_usage_percent=int(re.search(r"Aicore Usage Rate\(%\)\s*:\s*(\d+)", text)[1]),
                aivector_usage_percent=int(re.search(r"Aivector Usage Rate\(%\)\s*:\s*(\d+)", text)[1]),
                loadavg=(directory / f"{phase}.load.txt").read_text().strip(),
                processes=(directory / f"{phase}.processes.txt").read_text().strip())


def map_case(rows, width, prior):
    directory = CAPTURE / f"{rows}x{width}_fp32"
    log = (directory / "profile.log").read_text()
    references = re.findall(r"^REFERENCE .*$", log, re.M)
    assert len(references) == 1
    reference = dict(item.split("=", 1) for item in references[0].split()[1:])
    assert reference["mismatches"] == "0"
    assert (reference["rows"], reference["width"], reference["device"]) == (str(rows), str(width), "3")
    assert reference["physical_cores"] == reference["available_cores"] == reference["block_count"] == "40"
    assert reference["reference"] == "cpu_fp64" and reference["atol"] == "2e-5" and reference["rtol"] == "1e-4"
    assert reference["seed"] == str(314159 + width)
    assert (directory / "exit.txt").read_text().strip() == "PROFILE_RC=0"
    header = "block\tpair\torder\tp1_us\tp2_us\tp1_wall_us\tp2_wall_us"
    assert log.splitlines().count(header) == 1
    sample_lines = [line for line in log.splitlines() if re.match(r"^[01]\t\d+\tP[12]-P[12]\t", line)]
    assert len(sample_lines) == 42, len(sample_lines)
    raw_text = header + "\n" + "\n".join(sample_lines) + "\n"
    raw_pairs = list(csv.DictReader(io.StringIO(raw_text), delimiter="\t"))
    assert [(int(row["block"]), int(row["pair"])) for row in raw_pairs] == [
        (block, pair) for block in range(2) for pair in range(21)]

    exports = list(directory.glob("capture/PROF*/mindstudio_profiler_output"))
    assert len(exports) == 1
    op_paths = list(exports[0].glob("op_summary_*.csv"))
    task_paths = list(exports[0].glob("task_time_*.csv"))
    assert len(op_paths) == len(task_paths) == 1
    ops = read_rows(op_paths[0])
    tasks = read_rows(task_paths[0])
    ops.sort(key=lambda op: timestamp(op["Task Start Time(us)"]))
    assert len(ops) == 130, len(ops)
    assert {op["Device_id"] for op in ops} == {"3"}
    assert {op["Block Dim"] for op in ops} == {"40"}
    assert {op["Task Type"] for op in ops} == {"AI_VECTOR_CORE"}
    assert len({op["Op Name"] for op in ops}) == 1
    assert all("add_rms_norm_bias_custom" in op["Op Name"] for op in ops)
    assert len({op["Stream ID"] for op in ops}) == 1
    assert sum(task["kernel_type"] == "AI_VECTOR_CORE" for task in tasks) == 130
    task_index = {(task["Device_id"], task["stream_id"], task["task_id"]): task for task in tasks}
    assert len(task_index) == len(tasks)
    calls = []
    for ordinal, op in enumerate(ops, 1):
        key = (op["Device_id"], op["Stream ID"], op["Task ID"])
        task = task_index[key]
        assert timestamp(op["Task Start Time(us)"]) == timestamp(task["task_start(us)"])
        assert Decimal(op["Task Duration(us)"]) == Decimal(task["task_time(us)"])
        phase = "correctness" if ordinal == 1 else "warmup" if ordinal <= 46 else "timed"
        calls.append(dict(shape=f"{rows}x{width}", dtype="FP32", launch_ordinal=ordinal, phase=phase,
                          block="", pair="", order="", side="", position="",
                          device_id=key[0], stream_id=key[1], task_id=key[2], block_dim=op["Block Dim"],
                          kernel_start_us=str(timestamp(task["task_start(us)"])),
                          kernel_stop_us=str(timestamp(task["task_stop(us)"])),
                          kernel_us=float(task["task_time(us)"]),
                          device_event_us="", wall_us="", start_record_task_id="", stop_record_task_id="",
                          trace_interval_us="", interval_error_us="", event_minus_kernel_us="",
                          record_to_kernel_start_us="", kernel_end_to_record_us=""))

    timed = calls[46:]
    for pair_index, raw in enumerate(raw_pairs):
        block, pair = int(raw["block"]), int(raw["pair"])
        expected_order = "P1-P2" if (block + pair) % 2 == 0 else "P2-P1"
        assert raw["order"] == expected_order
        for position, side in enumerate(expected_order.split("-")):
            call = timed[2 * pair_index + position]
            device_us, wall_us = float(raw[side.lower() + "_us"]), float(raw[side.lower() + "_wall_us"])
            assert math.isfinite(device_us) and device_us > 0 and math.isfinite(wall_us) and wall_us > 0
            key = (call["device_id"], call["stream_id"], call["task_id"])
            start_record = task_index[(key[0], key[1], str(int(key[2]) - 1))]
            stop_record = task_index[(key[0], key[1], str(int(key[2]) + 1))]
            assert start_record["kernel_type"] == stop_record["kernel_type"] == "EVENT_RECORD"
            event_start, event_stop = timestamp(start_record["task_start(us)"]), timestamp(stop_record["task_start(us)"])
            task_start, task_stop = Decimal(call["kernel_start_us"]), Decimal(call["kernel_stop_us"])
            assert event_start <= task_start <= task_stop <= event_stop
            interval = float(event_stop - event_start)
            call.update(block=block, pair=pair, order=expected_order, side=side, position=position,
                        device_event_us=device_us, wall_us=wall_us,
                        start_record_task_id=start_record["task_id"], stop_record_task_id=stop_record["task_id"],
                        trace_interval_us=interval, interval_error_us=interval - device_us,
                        event_minus_kernel_us=device_us - call["kernel_us"],
                        record_to_kernel_start_us=float(task_start - event_start),
                        kernel_end_to_record_us=float(event_stop - task_stop))

    with (directory / "event.raw.tsv").open("x") as output:
        output.write(raw_text)
    with (directory / "all-calls.tsv").open("x", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=list(calls[0]), delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(calls)
    kernel = summarize(timed, "kernel_us")
    event = summarize(timed, "device_event_us")
    old = next(item for item in prior["profiles"] if item["origin"] == "THIS_RUN" and item["shape"] == [rows, width])
    saving = old["optimistic_command_share_saving_proxy_us"]
    floor = kernel["measurement_floor_us"]
    return dict(shape=[rows, width], dtype="FP32", reference=reference,
                call_mapping=dict(complete_kernel_calls=130, correctness_calls=1, warmup_calls=45,
                                  timed_calls=84, paired_samples=42, all_op_task_pairs_match=True,
                                  all_event_brackets_match=True,
                                  max_event_interval_error_us=max(abs(call["interval_error_us"]) for call in timed),
                                  op_summary=str(op_paths[0].relative_to(ROOT)),
                                  task_time=str(task_paths[0].relative_to(ROOT)),
                                  all_calls=str((directory / "all-calls.tsv").relative_to(ROOT))),
                kernel_task=kernel, device_event=event, wall=summarize(timed, "wall_us"),
                event_minus_kernel=stats([call["event_minus_kernel_us"] for call in timed]),
                record_to_kernel_start=stats([call["record_to_kernel_start_us"] for call in timed]),
                kernel_end_to_record=stats([call["kernel_end_to_record_us"] for call in timed]),
                PARAM_MTE2_COMMAND_COUNT=dict(per_core=4, per_launch=160, source="fcbd1814 source attribution; unchanged Parent"),
                PARAM_MTE2_TIME_OR_PROXY=dict(direct_us=None, proxy_us=old["param_mte2_time_proxy_mean_us"],
                                             source_device=0, new_capture=False, source=old["source"]),
                TOTAL_KERNEL_TIME=kernel["pooled"]["median_us"],
                EXPECTED_SAVING=dict(measured_us=None, prior_optimistic_proxy_us=saving,
                                     note="Bytes are unchanged; issue cost and overlap are not isolated; not an upper bound."),
                MEASUREMENT_FLOOR=dict(kernel_task_us=floor, event_us=event["measurement_floor_us"]),
                SIGNAL_ABOVE_NOISE="NO_PROXY_BELOW_FLOOR" if saving <= floor else "UNPROVEN_PARAMETER_TIME",
                proxy_to_kernel_floor=saving / floor,
                resources={phase: load_context(directory, phase) for phase in ("pre", "post")})


def main():
    prior = json.loads((ROOT / "results/analysis.json").read_text())
    result = dict(route="W4-R14", event_kind="ROUTE_RESEARCH_EVENT",
                  event_id="W4-R14-KERNEL-TASK-20261008", parent_event_id="W4-R14-PARAM-MTE2-20261008",
                  direct_parent="R31B V011", kernel_change=None, host_runner_change=None,
                  candidate=None, performance_revision=None, new_performance_revisions=0,
                  valid_local=0, stagnation_contribution=0, current_local_best=None,
                  local_score=None, local_delta=None, official_score=None, online="PAUSED", push="NO",
                  all_samples_retained=True, primary_metric="kernel_task_us",
                  floor_definition="max(p90(abs(P2-P1)), range(block medians))",
                  stability_definition="pooled MAD/median <= 0.10 and block-median drift <= 0.10",
                  method_sources=["fcbd1814", "4243f4e9", "a018f702", "1441fc72"],
                  cases=[map_case(rows, width, prior) for rows, width in CASES])
    with (CAPTURE / "analysis.json").open("x") as output:
        json.dump(result, output, indent=2, ensure_ascii=False, allow_nan=False)
        output.write("\n")
    for case in result["cases"]:
        print(json.dumps({key: case[key] for key in (
            "shape", "call_mapping", "kernel_task", "device_event", "event_minus_kernel",
            "PARAM_MTE2_TIME_OR_PROXY", "EXPECTED_SAVING", "MEASUREMENT_FLOOR", "SIGNAL_ABOVE_NOISE")},
            ensure_ascii=False, allow_nan=False))


if __name__ == "__main__":
    main()
