#!/usr/bin/env python3
"""Compare saved FP32 probe output with an independent double reference."""
import argparse
import json
import math
import struct
from pathlib import Path


def f32(value):
    return struct.unpack("<f", struct.pack("<f", value))[0]


def wide_geometry(width):
    tile = 4096
    for rows in range(8, 1, -1):
        if 4 * width * rows + 8 * tile + rows * 16 * 4 <= 176 * 1024:
            return rows, tile
    while tile > 2048:
        if 4 * width + 8 * tile + 16 * 4 <= 176 * 1024:
            return 1, tile
        tile -= 512
    return 1, tile


def analyze(prefix, rows, width):
    data = Path(str(prefix) + "-output.bin").read_bytes()
    assert len(data) == rows * width * 4
    actual = struct.unpack("<" + "f" * (rows * width), data)
    cache_rows, tile = wide_geometry(width) if width > 8192 else (None, 4096)
    tail = width % tile
    gamma = [f32(0.75 + f32(((j * 13) % 100) / 200.0)) for j in range(width)]
    bias = [f32(f32(((j * 7) % 100) / 400.0) - 0.125) for j in range(width)]
    regions = {name: {"elements": 0, "bad": 0, "nonfinite": 0,
                      "max_abs": 0.0, "max_abs_fp32_add_reference": 0.0,
                      "worst_examples": []} for name in ("full", "tail")}
    epsilon = f32(1.0e-5)
    for row in range(rows):
        values = []
        for j in range(width):
            i = row * width + j
            x = f32(f32(((i * 37 + 11) % 997) / 498.0) - 1.0)
            residual = f32(f32(((i * 17 + 3) % 997) / 498.0) - 1.0)
            values.append(x + residual)
        inv = 1.0 / math.sqrt(math.fsum(v * v for v in values) / width + epsilon)
        rounded_inv = 1.0 / math.sqrt(math.fsum(f32(v) ** 2 for v in values) / width + epsilon)
        for j, value in enumerate(values):
            index = row * width + j
            region = regions["tail" if tail and j >= width - tail else "full"]
            region["elements"] += 1
            got = actual[index]
            if not math.isfinite(got):
                region["nonfinite"] += 1
                region["bad"] += 1
                continue
            expected = value * inv * gamma[j] + bias[j]
            rounded_expected = f32(value) * rounded_inv * gamma[j] + bias[j]
            error = abs(got - expected)
            region["max_abs"] = max(region["max_abs"], error)
            region["max_abs_fp32_add_reference"] = max(region["max_abs_fp32_add_reference"], abs(got - rounded_expected))
            region["bad"] += error > 1.0e-4 + 1.0e-4 * abs(expected)
            if error > 1.0e-4 + 1.0e-4 * abs(expected):
                example = {"index": index, "actual": got, "reference": expected, "abs_error": error}
                if tail and j < width - tail and j % tile < tail:
                    last_panel_column = width - tail + j % tile
                    last_panel_expected = value * inv * gamma[last_panel_column] + bias[last_panel_column]
                    example.update({"last_panel_column": last_panel_column,
                                    "last_panel_expected": last_panel_expected,
                                    "last_panel_abs_error": abs(got - last_panel_expected)})
                region["worst_examples"].append(example)
                region["worst_examples"].sort(key=lambda v: v["abs_error"], reverse=True)
                del region["worst_examples"][5:]
    return {"prefix": str(prefix), "rows": rows, "width": width, "dtype": "fp32",
            "path_basis": "canonical source and runtime arguments; no device field instrumentation",
            "path": "ProcessWideFp32FullCacheRows" if width > 8192 else "narrow",
            "source_derived_cache_rows": cache_rows, "source_derived_tile": tile,
            "source_derived_tail": tail, "atol": 1.0e-4, "rtol": 1.0e-4,
            "regions": regions, "pass": all(r["bad"] == 0 for r in regions.values())}


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("evidence", type=Path)
    args = parser.parse_args()
    cases = [(1, 8192), (1, 9216), (1, 10240)]
    results = [analyze(args.evidence / f"parent-{rows}x{width}-run02", rows, width)
               for rows, width in cases]
    cache_rows, tile = wide_geometry(40000)
    print(json.dumps({"parent": "R31B V011", "results": results,
                      "not_executed": {"1x36736": "outside unchanged runner width limit",
                                       "1x40000": {"reason": "source-derived partial count exceeds allocated slots",
                                                    "cache_rows": cache_rows, "tile": tile,
                                                    "partial_count": math.ceil(40000 / tile),
                                                    "allocated_partials_per_row": 16}},
                      "official": "NONE", "online": "PAUSED"}, ensure_ascii=False, indent=2))
