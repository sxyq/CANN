#!/usr/bin/env python3
"""Group the existing FP64 reference comparison by kernel batch and region."""
import argparse
import json
import math
import struct
from pathlib import Path

from analyze_outputs import analyze, f32, wide_geometry


def compare_effect(evidence):
    before = (evidence / "tile-sync-121x9216-output.bin").read_bytes()
    after = (evidence / "batch-sync-121x9216-output.bin").read_bytes()
    assert len(before) == len(after) == 121 * 9216 * 4
    outside_equal = before[3 * 9216 * 4:] == after[3 * 9216 * 4:]
    inside_changed = 0
    model_matches = 0
    model_max_abs = 0.0
    examples = []

    def input_value(index, multiplier, offset):
        return f32(f32(((index * multiplier + offset) % 997) / 498.0) - 1.0)

    for row in range(3):
        start = row * 9216 * 4
        outside_equal &= before[start:start + 8192 * 4] == after[start:start + 8192 * 4]
        values = [input_value(row * 9216 + col, 37, 11) +
                  input_value(row * 9216 + col, 17, 3) for col in range(9216)]
        inv = 1.0 / math.sqrt(math.fsum(v * v for v in values) / 9216 + f32(1.0e-5))
        for col in range(8192, 9216):
            index = row * 9216 + col
            pos = index * 4
            inside_changed += before[pos:pos + 4] != after[pos:pos + 4]
            got = struct.unpack_from("<f", before, pos)[0]
            next_input_index = 3 * 9216 + col - 8192
            reused_x = input_value(next_input_index, 37, 11)
            reused_residual = input_value(next_input_index, 17, 3)
            expected_wrong = values[col] * inv * reused_x + reused_residual
            error = abs(got - expected_wrong)
            model_matches += error <= 1.0e-4 + 1.0e-4 * abs(expected_wrong)
            model_max_abs = max(model_max_abs, error)
            if col == 8192:
                examples.append({"row": row, "column": col, "actual_before": got,
                                 "actual_after": struct.unpack_from("<f", after, pos)[0],
                                 "next_input_index": next_input_index,
                                 "reused_x": reused_x, "reused_residual": reused_residual,
                                 "overwrite_model": expected_wrong, "model_abs_error": error})
    return {"shape": "121x9216", "dtype": "fp32", "device": 3,
            "model": "previous batch tail uses next batch row3 first-tile x/residual as gamma/bias",
            "model_elements": 3072, "model_matches": model_matches,
            "model_max_abs": model_max_abs, "atol": 1.0e-4, "rtol": 1.0e-4,
            "changed_elements_inside_first_batch_tail": inside_changed,
            "outside_first_batch_tail_bitwise_equal": outside_equal,
            "examples": examples, "device_runs_added": 0}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("role", choices=("parent", "tile-sync", "batch-sync", "effect", "reference"))
    args = parser.parse_args()
    evidence = Path(__file__).resolve().parent / "evidence" / "batch-reuse-01"
    if args.role == "effect":
        print(json.dumps(compare_effect(evidence), ensure_ascii=False, indent=2))
        return 0
    if args.role == "reference":
        output = evidence / "reference-fp64-121x9216.bin"
        with output.open("xb") as stream:
            result = analyze(evidence / "batch-sync-121x9216", 121, 9216,
                             reference_stream=stream)
        assert result["pass"] and output.stat().st_size == 121 * 9216 * 8
        print("FP64_REFERENCE_RAW=PASS; little-endian float64; row-major 121x9216;",
              output.stat().st_size, "bytes;", output.name)
        return 0
    prefix = evidence / f"{args.role}-121x9216"
    result = analyze(prefix, 121, 9216, include_rows=True)
    groups = []
    for label, begin, end in (("block0_batch0", 0, 3),
                              ("block0_batch1", 3, 4),
                              ("blocks1to39_single_batch", 4, 121)):
        regions = {}
        for region in ("full", "tail"):
            selected = [row["regions"][region] for row in result["per_row"][begin:end]]
            regions[region] = {
                key: sum(item[key] for item in selected)
                for key in ("elements", "bad", "nonfinite")}
            regions[region]["max_abs"] = max(item["max_abs"] for item in selected)
        groups.append({"group": label, "row_begin": begin, "row_end_exclusive": end,
                       "regions": regions})
    stats = dict(line.split("\t", 1) for line in
                 Path(str(prefix) + "-stats.txt").read_text().splitlines())
    cache_rows, tile = wide_geometry(9216)
    report = {
        "route": "W4-R13", "revision": "DIAG-PARAM-REUSE-01",
        "study": "BATCH-REUSE-01", "role": args.role,
        "classification": "RESEARCH_ONLY", "device": 3,
        "geometry": {"M": 121, "D": 9216, "availableCoreNum": 40,
                     "blockCount": 40, "block0_localRows": 4,
                     "block0_batchRows": [3, 1], "batchLimit": cache_rows,
                     "tileWidth": tile, "tileCount": 3},
        "fp32_runner_bad": int(stats["bad"]),
        "fp32_runner_max_abs": float(stats["max_abs"]),
        "runner_stats": stats, "fp64_reference": result,
        "batch_groups": groups, "local_score": None, "local_delta": None,
        "new_performance_revisions": 0, "official": "NONE", "online": "PAUSED",
    }
    print(json.dumps(report, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
