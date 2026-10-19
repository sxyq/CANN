#!/usr/bin/env python3
import argparse
import hashlib
import json
import math
import random
import statistics
import struct
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

V034_SHA = "75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33"
V033_SHA = "0df982248c80c4f2e5541dc730b0a25ba97544efc3b3e3e3409c24deed2d2eff"

CASES = [
    ("T01", 1, (1, 64)),
    ("T02", 1, (4, 96)),
    ("T03", 1, (2, 3, 576)),
    ("T04", 1, (2, 1, 2, 1024)),
    ("T05", 1, (1, 67)),
    ("T06", 2, (1, 64)),
    ("T07", 2, (3, 192)),
    ("T08", 2, (2, 2, 1000)),
    ("T09", 2, (2, 1, 2, 4096)),
    ("T10", 2, (1, 129)),
    ("T11", 0, (1, 64)),
    ("T12", 0, (3, 576)),
    ("T13", 0, (2, 3, 1024)),
    ("T14", 0, (2, 1, 2, 32768)),
    ("T15", 0, (96, 8192)),
]

DTYPE_NAMES = {0: "fp32", 1: "fp16", 2: "bf16"}
EPSILON = 1.0e-5


def round_f32(value):
    return struct.unpack("<f", struct.pack("<f", value))[0]


def encode_value(dtype, value):
    value = round_f32(value)
    if dtype == 0:
        return struct.pack("<f", value), value
    if dtype == 1:
        raw = struct.pack("<e", value)
        return raw, struct.unpack("<e", raw)[0]
    bits = struct.unpack("<I", struct.pack("<f", value))[0]
    if math.isnan(value):
        rounded = (bits >> 16) | 0x40
    else:
        rounded = bits + 0x7FFF + ((bits >> 16) & 1)
    upper = (rounded >> 16) & 0xFFFF
    return struct.pack("<H", upper), struct.unpack("<f", struct.pack("<I", upper << 16))[0]


def decode_values(dtype, data):
    if dtype == 0:
        return [item[0] for item in struct.iter_unpack("<f", data)]
    if dtype == 1:
        return [item[0] for item in struct.iter_unpack("<e", data)]
    return [struct.unpack("<f", struct.pack("<I", item[0] << 16))[0]
            for item in struct.iter_unpack("<H", data)]


def write_values(path, dtype, values):
    with path.open("wb") as stream:
        for value in values:
            stream.write(encode_value(dtype, value)[0])


def generate_case(directory, case_id, dtype, shape, seed):
    directory.mkdir(parents=True, exist_ok=True)
    rng = random.Random(seed)
    width = shape[-1]
    elements = math.prod(shape)
    x = [encode_value(dtype, rng.uniform(-2.0, 2.0)) for _ in range(elements)]
    residual = [encode_value(dtype, rng.uniform(-2.0, 2.0)) for _ in range(elements)]
    gamma = [encode_value(dtype, rng.uniform(0.25, 1.5)) for _ in range(width)]
    bias = [encode_value(dtype, rng.uniform(-0.5, 0.5)) for _ in range(width)]

    x_path = directory / "x.bin"
    residual_path = directory / "residual.bin"
    gamma_path = directory / "gamma.bin"
    bias_path = directory / "bias.bin"
    golden_path = directory / "golden.bin"
    for path, values in ((x_path, x), (residual_path, residual),
                         (gamma_path, gamma), (bias_path, bias)):
        with path.open("wb") as stream:
            for raw, _ in values:
                stream.write(raw)

    x_values = [value for _, value in x]
    residual_values = [value for _, value in residual]
    gamma_values = [value for _, value in gamma]
    bias_values = [value for _, value in bias]
    golden = []
    for row in range(elements // width):
        offset = row * width
        y = [x_values[offset + col] + residual_values[offset + col]
             for col in range(width)]
        mean_square = math.fsum(value * value for value in y) / width
        inverse_rms = 1.0 / math.sqrt(mean_square + EPSILON)
        for col, value in enumerate(y):
            output = value * inverse_rms * gamma_values[col] + bias_values[col]
            golden.append(output)
    write_values(golden_path, dtype, golden)

    return {
        "case_id": case_id,
        "dtype": dtype,
        "dtype_name": DTYPE_NAMES[dtype],
        "shape": list(shape),
        "epsilon": EPSILON,
        "seed": seed,
        "files": {"x": str(x_path), "residual": str(residual_path),
                  "gamma": str(gamma_path), "bias": str(bias_path),
                  "golden": str(golden_path)},
    }


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def runner_command(runner, case, output, device, warmup, repeat):
    files = case["files"]
    return [str(runner), "--device", str(device), "--dtype", str(case["dtype"]),
            "--shape", ",".join(str(value) for value in case["shape"]),
            "--epsilon", str(case["epsilon"]), "--warmup", str(warmup),
            "--repeat", str(repeat), "--x", files["x"],
            "--residual", files["residual"], "--gamma", files["gamma"],
            "--bias", files["bias"], "--output", str(output)]


def invoke(runner, case, output, log_path, expected_sha, device, warmup, repeat):
    command = runner_command(runner, case, output, device, warmup, repeat)
    completed = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT, check=False)
    log_path.write_text("COMMAND=" + json.dumps(command) + "\n" + completed.stdout,
                        encoding="utf-8")
    if completed.returncode != 0:
        raise RuntimeError(f"runner returned {completed.returncode}; see {log_path}")
    identity = f"KERNEL_SOURCE_SHA256={expected_sha}"
    if identity not in completed.stdout:
        raise RuntimeError(f"runner identity mismatch; see {log_path}")
    runner_file = Path(runner).resolve()
    runner_path_line = f"RUNNER_EXECUTABLE_PATH={runner_file}"
    if runner_path_line not in completed.stdout:
        raise RuntimeError(f"runner executable identity mismatch; see {log_path}")
    if not runner_file.is_file():
        raise RuntimeError(f"runner executable is missing: {runner_file}")
    runner_identity = {"path": str(runner_file),
                       "sha256": hashlib.sha256(runner_file.read_bytes()).hexdigest()}
    latencies = [float(line.split("=", 1)[1]) for line in completed.stdout.splitlines()
                 if line.startswith("LATENCY_US=")]
    if len(latencies) != repeat:
        raise RuntimeError(f"expected {repeat} timing samples, got {len(latencies)}")
    return latencies, runner_identity


def compare(dtype, actual_path, golden_path):
    actual_bytes = actual_path.read_bytes()
    golden_bytes = golden_path.read_bytes()
    actual = decode_values(dtype, actual_bytes)
    golden = decode_values(dtype, golden_bytes)
    if len(actual) != len(golden):
        raise RuntimeError("output and golden element counts differ")
    tol = 1.0e-4 if dtype == 0 else 1.0e-3
    max_abs = 1.0e-2 if dtype == 0 else (1.0e-1 if dtype == 1 else 2.5e-1)
    matched = 0
    max_error = 0.0
    first_bad = None
    for index, (got, want) in enumerate(zip(actual, golden)):
        error = abs(got - want)
        max_error = max(max_error, error)
        if math.isfinite(got) and math.isfinite(want) and error <= tol + tol * abs(want):
            matched += 1
        elif first_bad is None:
            first_bad = {"index": index, "actual": got, "golden": want, "abs_error": error}
    ratio = matched / max(1, len(golden))
    passed = ratio >= 0.999 and max_error <= max_abs
    return {"passed": passed, "elements": len(golden), "matched_ratio": ratio,
            "required_matched_ratio": 0.999, "max_abs_error": max_error,
            "max_abs_error_limit": max_abs, "atol": tol, "rtol": tol,
            "first_mismatch": first_bad}


def prepare_output_dir(path):
    path.mkdir(parents=True, exist_ok=False)


def run_correctness(args):
    out = Path(args.out_dir).resolve()
    prepare_output_dir(out)
    cases_dir = out / "cases"
    results = []
    source_sha = V033_SHA if args.revision == "V033" else V034_SHA
    selected_cases = [(index, case) for index, case in enumerate(CASES)
                      if args.case is None or case[0] == args.case]
    for case_index, (case_id, dtype, shape) in selected_cases:
        case_dir = cases_dir / case_id
        case = generate_case(case_dir, case_id, dtype, shape, 341034 + case_index)
        output = case_dir / "actual.bin"
        log = out / f"{case_id}.runner.log"
        samples, runner_identity = invoke(Path(args.runner).resolve(), case, output, log, source_sha,
                                          args.device, args.warmup, 1)
        comparison = compare(dtype, output, Path(case["files"]["golden"]))
        result = {**case, **comparison, "runner_executable": runner_identity,
                  "latency_us_diagnostic": samples[0],
                  "runner_log": str(log), "output": str(output)}
        results.append(result)
        print(f"{case_id} {case['dtype_name']} {case['shape']} "
              f"{'PASS' if comparison['passed'] else 'FAIL'} "
              f"matched={comparison['matched_ratio']:.6f} "
              f"max_abs={comparison['max_abs_error']:.8g}")
    summary = {
        "route": "STAGING-LIVENESS-X",
        "revision": args.revision,
        "status": "PASS" if all(item["passed"] for item in results) else "FAIL",
        "suite": "route-local synthetic coverage; not the unpublished official T01-T15 mapping",
        "selection": args.case or "all route-local cases",
        "tested_source_sha256": source_sha,
        "parent_source_sha256": V033_SHA,
        "candidate_source_sha256": V034_SHA,
        "device": args.device,
        "case_count": len(results),
        "cases": results,
        "precision_rule": "fp32 atol=rtol=1e-4; fp16/bf16 atol=rtol=1e-3; matched ratio >=0.999; dtype hard error limit",
    }
    write_json(out / "correctness.json", summary)
    print("CORRECTNESS_STATUS=" + summary["status"])
    print("EVIDENCE=" + str(out))
    if summary["status"] != "PASS":
        raise SystemExit(1)


def snapshot(device, path):
    command = ["npu-smi", "info", "-t", "usages", "-i", str(device)]
    result = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, check=False)
    path.write_text("COMMAND=" + json.dumps(command) + "\n" + result.stdout,
                    encoding="utf-8")
    if result.returncode != 0:
        raise RuntimeError(f"npu-smi failed; see {path}")
    return result.stdout


def statistics_for(values):
    ordered = sorted(values)
    median = statistics.median(ordered)
    mean = statistics.fmean(ordered)
    stdev = statistics.stdev(ordered) if len(ordered) > 1 else 0.0
    deviations = [abs(value - median) for value in ordered]
    return {"count": len(ordered), "raw_us": values, "median_us": median,
            "mean_us": mean, "stdev_us": stdev,
            "cv": stdev / mean if mean else 0.0,
            "min_us": ordered[0], "max_us": ordered[-1],
            "mad_us": statistics.median(deviations),
            "p10_us": ordered[int((len(ordered) - 1) * 0.10)],
            "p90_us": ordered[int((len(ordered) - 1) * 0.90)]}


def parse_load(text):
    values = {}
    for line in text.splitlines():
        if ":" not in line:
            continue
        key, value = line.split(":", 1)
        key = key.strip()
        if key in ("HBM Usage Rate(%)", "Aicore Usage Rate(%)", "Aivector Usage Rate(%)"):
            values[key] = int(value.strip().split()[0])
    return values


def run_local(args):
    out = Path(args.out_dir).resolve()
    prepare_output_dir(out)
    case_dir = out / "case-T15"
    case = generate_case(case_dir, "T15", 0, (96, 8192), 341049)
    parent_runner = Path(args.parent_runner).resolve()
    candidate_runner = Path(args.candidate_runner).resolve()
    blocks = []
    all_parent = []
    all_candidate = []
    order = ("P", "C", "C", "P") * args.rounds
    for index, kind in enumerate(order):
        role = "parent" if kind == "P" else "candidate"
        runner = parent_runner if kind == "P" else candidate_runner
        source_sha = V033_SHA if kind == "P" else V034_SHA
        before_path = out / f"block-{index:02d}-{role}-pre.npu.txt"
        after_path = out / f"block-{index:02d}-{role}-post.npu.txt"
        before = snapshot(args.device, before_path)
        output = case_dir / f"{index:02d}-{role}.bin"
        log = out / f"block-{index:02d}-{role}.runner.log"
        samples, runner_identity = invoke(runner, case, output, log, source_sha, args.device,
                                          args.warmup, args.samples_per_block)
        after = snapshot(args.device, after_path)
        block = {"index": index, "role": role, "samples": samples,
                 "runner_executable": runner_identity,
                 "pre_load": parse_load(before), "post_load": parse_load(after),
                 "pre_snapshot": str(before_path), "post_snapshot": str(after_path),
                 "runner_log": str(log)}
        blocks.append(block)
        (all_parent if kind == "P" else all_candidate).extend(samples)
        print(f"BLOCK={index:02d} ROLE={role} MEDIAN_US={statistics.median(samples):.6f} "
              f"CV={statistics_for(samples)['cv']:.5f}")

    parent_stats = statistics_for(all_parent)
    candidate_stats = statistics_for(all_candidate)
    parent_block_medians = [statistics.median(block["samples"])
                            for block in blocks if block["role"] == "parent"]
    candidate_block_medians = [statistics.median(block["samples"])
                               for block in blocks if block["role"] == "candidate"]
    parent_block_spread = ((max(parent_block_medians) - min(parent_block_medians)) /
                           statistics.median(parent_block_medians))
    candidate_block_spread = ((max(candidate_block_medians) - min(candidate_block_medians)) /
                              statistics.median(candidate_block_medians))
    delta = (candidate_stats["median_us"] / parent_stats["median_us"] - 1.0) * 100.0
    loads = [load for block in blocks for load in (block["pre_load"], block["post_load"])]
    load_clean = all(load.get("Aicore Usage Rate(%)", 100) == 0 and
                     load.get("Aivector Usage Rate(%)", 100) == 0 for load in loads)
    load_stable = all(load.get("HBM Usage Rate(%)") == loads[0].get("HBM Usage Rate(%)")
                      for load in loads)
    noise_pass = parent_block_spread <= 0.05 and candidate_block_spread <= 0.05
    load_quality = "PASS" if load_clean and load_stable and noise_pass else "DEGRADED"
    summary = {
        "route": "STAGING-LIVENESS-X", "revision": "V034", "parent_revision": "V033",
        "shape": list(case["shape"]), "dtype": case["dtype_name"],
        "method": "ACL device-event timing; P-C-C-P interleaving by block",
        "device": args.device, "warmup_per_block": args.warmup,
        "samples_per_block": args.samples_per_block, "rounds": args.rounds,
        "parent_source_sha256": V033_SHA, "candidate_source_sha256": V034_SHA,
        "parent": {**parent_stats, "block_medians_us": parent_block_medians,
                   "same_binary_block_spread": parent_block_spread},
        "candidate": {**candidate_stats, "block_medians_us": candidate_block_medians,
                      "same_binary_block_spread": candidate_block_spread},
        "local_delta_percent_candidate_vs_parent": delta,
        "load_quality": load_quality,
        "load_notes": {"aicore_and_aivector_zero": load_clean,
                       "hbm_usage_stable": load_stable,
                       "vllm_resident_process_not_modified": True},
        "noise_floor_pass": noise_pass,
        "blocks": blocks,
        "interpretation": "Numeric local comparison only; acceptance depends on stable noise/load and repeatable direction.",
    }
    write_json(out / "local-result.json", summary)
    print(f"PARENT_MEDIAN_US={parent_stats['median_us']:.6f}")
    print(f"CANDIDATE_MEDIAN_US={candidate_stats['median_us']:.6f}")
    print(f"LOCAL_DELTA_PERCENT={delta:.4f}")
    print(f"LOAD_QUALITY={load_quality}")
    print("EVIDENCE=" + str(out))


def main():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)
    correctness = subparsers.add_parser("correctness")
    correctness.add_argument("--runner", required=True)
    correctness.add_argument("--revision", choices=("V033", "V034"), default="V034")
    correctness.add_argument("--out-dir", required=True)
    correctness.add_argument("--device", type=int, default=1)
    correctness.add_argument("--warmup", type=int, default=1)
    correctness.add_argument("--case", choices=[case[0] for case in CASES])
    correctness.set_defaults(func=run_correctness)

    local = subparsers.add_parser("local")
    local.add_argument("--parent-runner", required=True)
    local.add_argument("--candidate-runner", required=True)
    local.add_argument("--out-dir", required=True)
    local.add_argument("--device", type=int, default=1)
    local.add_argument("--warmup", type=int, default=10)
    local.add_argument("--samples-per-block", type=int, default=5)
    local.add_argument("--rounds", type=int, default=3)
    local.set_defaults(func=run_local)
    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
