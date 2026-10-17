#!/usr/bin/env python3
import argparse
import csv
import math
import statistics
from pathlib import Path


WARMUPS = 45
SAMPLES = 21
BLOCKS = 2
WINDOW_REPS = 6
PAIR_COUNT = 4
QUAL_CV_MAX = 0.15
QUAL_MAX_MIN_MAX = 1.30
SAME_BINARY_MAD_MEDIAN_MAX = 0.10
SAME_BINARY_DRIFT_MAX = 0.10


def percentile(sorted_values, fraction):
    if len(sorted_values) == 1:
        return sorted_values[0]
    position = fraction * (len(sorted_values) - 1)
    low = int(position)
    high = min(low + 1, len(sorted_values) - 1)
    part = position - low
    return sorted_values[low] * (1.0 - part) + sorted_values[high] * part


def summarize(values):
    ordered = sorted(values)
    median = statistics.median(ordered)
    mean = statistics.fmean(ordered)
    stdev = math.sqrt(statistics.fmean((value - mean) ** 2 for value in ordered))
    return {
        "n": len(ordered),
        "median": median,
        "mean": mean,
        "stdev": stdev,
        "cv": stdev / mean if mean else math.inf,
        "mad": statistics.median(abs(value - median) for value in ordered),
        "p10": percentile(ordered, 0.10),
        "p90": percentile(ordered, 0.90),
        "min": ordered[0],
        "max": ordered[-1],
        "max_min": ordered[-1] / ordered[0] if ordered[0] else math.inf,
        "span": ordered[-1] - ordered[0],
    }


def read_matrix(path):
    with path.open(newline="") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def read_raw(path):
    blocks = {}
    with path.open(newline="") as stream:
        for row in csv.DictReader(stream, delimiter="\t"):
            value = float(row["device_event_us"])
            if not math.isfinite(value) or value <= 0:
                raise ValueError(f"invalid device event sample in {path}: {value}")
            blocks.setdefault(int(row["block"]), []).append(value)
    return blocks


def write_tsv(path, header, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as stream:
        writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
        writer.writerow(header)
        writer.writerows(rows)


def same_binary(args):
    blocks = read_raw(args.raw)
    with args.raw.open(newline="") as raw_stream:
        raw_rows = list(csv.DictReader(raw_stream, delimiter="\t"))
    matrix = {row["shape_id"]: row for row in read_matrix(args.matrix)}
    if args.shape not in matrix:
        raise ValueError(f"shape not registered: {args.shape}")
    if set(blocks) != {1, 2}:
        raise ValueError(f"expected two blocks in {args.raw}; found {sorted(blocks)}")
    if any(len(blocks[index]) != SAMPLES for index in (1, 2)):
        raise ValueError(f"expected {SAMPLES} event samples in each block: {args.raw}")
    if len(raw_rows) != BLOCKS * SAMPLES or any(
        row["stage"] != "SAME_BINARY" or row["process_rep"] != "1" for row in raw_rows
    ) or len({row["pid"] for row in raw_rows}) != 1:
        raise ValueError(f"same-binary data must come from one process: {args.raw}")

    first = summarize(blocks[1])
    second = summarize(blocks[2])
    all_values = blocks[1] + blocks[2]
    combined = summarize(all_values)
    drift = abs(first["median"] - second["median"]) / combined["median"]
    shape_pass = (
        combined["mad"] / combined["median"] <= SAME_BINARY_MAD_MEDIAN_MAX
        and drift <= SAME_BINARY_DRIFT_MAX
    )
    status = "PASS" if shape_pass else "NOT_QUALIFIED"
    fields = [
        "shape_id", "role", "warmups", "samples_per_block", "blocks",
        "block1_median_us", "block1_cv", "block1_max_min",
        "block2_median_us", "block2_cv", "block2_max_min",
        "all_median_us", "all_mad_over_median", "block_drift",
        "same_binary_qualification",
    ]
    row = [
        args.shape, matrix[args.shape]["role"], WARMUPS, SAMPLES, BLOCKS,
        first["median"], first["cv"], first["max_min"],
        second["median"], second["cv"], second["max_min"],
        combined["median"], combined["mad"] / combined["median"], drift,
        status,
    ]
    write_tsv(args.out, fields, [row])
    print(status)


def parent_window(args):
    entries = []
    with args.index.open(newline="") as stream:
        for row in csv.DictReader(stream, delimiter="\t"):
            if row["shape_id"] == args.shape and row["stage"] == args.stage:
                entries.append(row)
    entries.sort(key=lambda row: int(row["process_rep"]))
    if len(entries) != WINDOW_REPS or [int(row["process_rep"]) for row in entries] != list(range(1, WINDOW_REPS + 1)):
        raise ValueError(f"{args.shape} {args.stage}: expected {WINDOW_REPS} fresh process reps")
    pids = [row["pid"] for row in entries]
    if any(not pid for pid in pids) or len(set(pids)) != WINDOW_REPS:
        raise ValueError(f"{args.shape} {args.stage}: process IDs must be present and distinct")
    process_medians = []
    raw_values = []
    for entry in entries:
        if entry["rc"] != "0":
            raise ValueError(f"runner failed for {args.shape} {args.stage} rep {entry['process_rep']}")
        blocks = read_raw(Path(entry["raw_path"]))
        if set(blocks) != {1} or len(blocks[1]) != SAMPLES:
            raise ValueError(f"invalid one-block sample set: {entry['raw_path']}")
        with Path(entry["raw_path"]).open(newline="") as raw_stream:
            raw_rows = list(csv.DictReader(raw_stream, delimiter="\t"))
        if any(row["stage"] != args.stage or row["process_rep"] != entry["process_rep"]
               or row["pid"] != entry["pid"] for row in raw_rows):
            raise ValueError(f"stage/process identity mismatch: {entry['raw_path']}")
        values = blocks[1]
        process_medians.append(statistics.median(values))
        raw_values.extend(values)
    rep_stats = summarize(process_medians)
    raw_stats = summarize(raw_values)
    status = (
        "PASS"
        if rep_stats["cv"] <= QUAL_CV_MAX and rep_stats["max_min"] <= QUAL_MAX_MIN_MAX
        else "NOT_QUALIFIED"
    )
    write_tsv(
        args.out,
        ["shape_id", "stage", "fresh_processes", "samples_per_process", "process_medians_us",
         "rep_median_us", "rep_mean_us", "rep_stdev_us", "rep_cv", "rep_mad_us",
         "rep_p10_us", "rep_p90_us", "rep_min_us", "rep_max_us", "rep_max_min",
         "rep_abs_span_us", "raw_sample_count", "raw_median_us", "raw_mad_over_median",
         "window_qualification"],
        [[args.shape, args.stage, len(process_medians), SAMPLES,
          ",".join(f"{value:.12g}" for value in process_medians), rep_stats["median"],
          rep_stats["mean"], rep_stats["stdev"], rep_stats["cv"], rep_stats["mad"],
          rep_stats["p10"], rep_stats["p90"], rep_stats["min"], rep_stats["max"],
          rep_stats["max_min"], rep_stats["span"], raw_stats["n"], raw_stats["median"],
          raw_stats["mad"] / raw_stats["median"], status]],
    )
    print(status)


def stable_direction(deltas, noise_floor):
    if len(deltas) != PAIR_COUNT:
        return "INCOMPLETE"
    median_delta = statistics.median(deltas)
    favorable = sum(delta < 0.0 for delta in deltas)
    unfavorable = sum(delta > 0.0 for delta in deltas)
    if favorable >= 3 and median_delta < 0.0 and abs(median_delta) / 100.0 > noise_floor:
        return "IMPROVEMENT"
    if unfavorable >= 3 and median_delta > 0.0 and abs(median_delta) / 100.0 > noise_floor:
        return "REGRESSION"
    return "NO_CLEAR_SIGNAL"


def route_summary(args):
    matrix = read_matrix(args.matrix)
    result_root = Path(args.result_dir)
    pairs_path = result_root / "pairs.tsv"
    with pairs_path.open(newline="") as stream:
        pair_rows = list(csv.DictReader(stream, delimiter="\t"))

    output_rows = []
    shape_results = {}
    for item in matrix:
        shape = item["shape_id"]
        same_binary_path = result_root / "qualification" / f"{shape}.same-binary.tsv"
        same_binary = None
        if same_binary_path.exists():
            with same_binary_path.open(newline="") as stream:
                same_binary = next(csv.DictReader(stream, delimiter="\t"), None)
        same_status = same_binary["same_binary_qualification"] if same_binary else "MISSING"
        window_statuses = []
        for stage in ("PRECHECK-A", "PRECHECK-B"):
            path = result_root / "qualification" / f"{shape}.{stage}.tsv"
            if path.exists():
                with path.open(newline="") as stream:
                    row = next(csv.DictReader(stream, delimiter="\t"), None)
                window_statuses.append(row["window_qualification"] if row else "MISSING")
            else:
                window_statuses.append("MISSING")
        parent_mad_fraction = (
            float(same_binary["all_mad_over_median"])
            if same_binary and same_binary["all_median_us"] not in ("", "0")
            else math.nan
        )
        deltas = []
        for pair in range(1, PAIR_COUNT + 1):
            arms = {}
            for entry in pair_rows:
                if entry["shape_id"] == shape and int(entry["pair"]) == pair and entry["rc"] == "0":
                    arms[entry["arm"]] = entry
            if set(arms) == {"P", "C"}:
                p_median = summarize(read_raw(Path(arms["P"]["raw_path"]))[1])["median"]
                c_median = summarize(read_raw(Path(arms["C"]["raw_path"]))[1])["median"]
                deltas.append((c_median - p_median) / p_median * 100.0)
        eligible = same_status == "PASS" and window_statuses == ["PASS", "PASS"]
        verdict = (
            stable_direction(deltas, parent_mad_fraction)
            if math.isfinite(parent_mad_fraction) and eligible
            else "NOT_MEASURED"
        )
        shape_results[shape] = verdict
        output_rows.append([
            item["order"], shape, item["role"], same_status,
            window_statuses[0], window_statuses[1], len(deltas),
            ",".join(f"{value:.9g}" for value in deltas),
            statistics.median(deltas) if deltas else "",
            parent_mad_fraction if math.isfinite(parent_mad_fraction) else "",
            verdict,
        ])

    primary = next(item["shape_id"] for item in matrix if item["role"] == "PRIMARY")
    complete = all(shape_results[item["shape_id"]] in {"IMPROVEMENT", "REGRESSION", "NO_CLEAR_SIGNAL"}
                   for item in matrix)
    any_regression = any(value == "REGRESSION" for value in shape_results.values())
    if any_regression:
        route_verdict = "LOCAL_REJECTED"
    elif complete and shape_results.get(primary) == "IMPROVEMENT":
        route_verdict = "LOCAL_ACCEPTED"
    else:
        route_verdict = "NEEDS_ONE_MORE_LOCAL"

    write_tsv(
        result_root / "route-summary.tsv",
        ["order", "shape_id", "role", "same_binary", "precheck_a", "precheck_b",
         "complete_pairs", "pair_delta_pct", "median_delta_pct",
         "parent_same_binary_mad_over_median", "shape_result"],
        output_rows,
    )
    write_tsv(result_root / "route-verdict.tsv", ["route_verdict"], [[route_verdict]])
    print(route_verdict)


def main():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)
    q = subparsers.add_parser("same-binary")
    q.add_argument("--matrix", type=Path, required=True)
    q.add_argument("--shape", required=True)
    q.add_argument("--raw", type=Path, required=True)
    q.add_argument("--out", type=Path, required=True)
    q.set_defaults(func=same_binary)
    w = subparsers.add_parser("parent-window")
    w.add_argument("--shape", required=True)
    w.add_argument("--stage", choices=("PRECHECK-A", "PRECHECK-B"), required=True)
    w.add_argument("--index", type=Path, required=True)
    w.add_argument("--out", type=Path, required=True)
    w.set_defaults(func=parent_window)
    s = subparsers.add_parser("summarize")
    s.add_argument("--matrix", type=Path, required=True)
    s.add_argument("--result-dir", type=Path, required=True)
    s.set_defaults(func=route_summary)
    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
