#!/usr/bin/env python3
import csv
import hashlib
import json
import os
import re
import struct
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parent
BUILD = ROOT / "build"
RESULT = ROOT / "logs" / "correctness-run-002-repro"
DEVICE = os.environ.get("DEVICE_ID", "2")
ROWS = 2
WIDTH = 16384
DTYPE = 0


def f32(value):
    return struct.unpack("<f", struct.pack("<f", value))[0]


def input_value(index, multiplier, offset):
    numerator = f32(float((index * multiplier + offset) % 997))
    quotient = f32(numerator / f32(498.0))
    return f32(quotient - f32(1.0))


def packed_floats(values):
    return b"".join(struct.pack("<f", value) for value in values)


def make_inputs():
    elements = ROWS * WIDTH
    x = packed_floats(input_value(i, 37, 11) for i in range(elements))
    residual = packed_floats(input_value(i, 17, 3) for i in range(elements))
    gamma = packed_floats(
        f32(f32(0.75) + f32(f32(float((i * 13) % 100)) / f32(200.0)))
        for i in range(WIDTH)
    )
    bias = packed_floats(
        f32(f32(f32(float((i * 7) % 100)) / f32(400.0)) - f32(0.125))
        for i in range(WIDTH)
    )
    return {"x": x, "residual": residual, "gamma": gamma, "bias": bias}


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def file_sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def result_fields(path):
    with path.open(newline="") as source:
        rows = list(csv.DictReader(source, delimiter="\t"))
    if len(rows) != 1:
        raise RuntimeError(f"expected one result row in {path}")
    return rows[0]


def main():
    RESULT.mkdir(parents=True, exist_ok=False)
    buffers = make_inputs()
    parent_source = ROOT.parent / "parent.asc"
    candidate_source = ROOT.parent / "submission.asc"
    parent_sidecar = Path(
        "/home/data4t2/lelinfeng/cann/线上结果/R31B/V011/submission.sha256"
    )
    parent_object_deps = BUILD / "CMakeFiles" / "selective_v001_parent_correctness.dir" / "correctness_parent.asc.o.d"
    candidate_object_deps = BUILD / "CMakeFiles" / "selective_v001_candidate_correctness.dir" / "correctness_candidate.asc.o.d"
    parent_dep_text = parent_object_deps.read_text(encoding="utf-8")
    candidate_dep_text = candidate_object_deps.read_text(encoding="utf-8")
    parent_source_sha = file_sha256(parent_source)
    candidate_source_sha = file_sha256(candidate_source)
    sidecar_sha = parent_sidecar.read_text(encoding="utf-8").split()[0]
    if parent_source_sha != sidecar_sha:
        raise RuntimeError(
            f"Parent source/sidecar mismatch: {parent_source_sha} != {sidecar_sha}"
        )
    parent_includes = sorted({
        str(Path(path).resolve())
        for path in re.findall(r"/[^\\\s]+/parent\.asc", parent_dep_text)
    })
    candidate_includes = sorted({
        str(Path(path).resolve())
        for path in re.findall(r"/[^\\\s]+/submission\.asc", candidate_dep_text)
    })
    if str(parent_source) not in parent_includes or str(candidate_source) not in candidate_includes:
        raise RuntimeError("compile dependency files do not name the staged exact sources")

    with (RESULT / "input-sha256.tsv").open("w", newline="") as target:
        writer = csv.writer(target, delimiter="\t", lineterminator="\n")
        writer.writerow(["buffer", "bytes", "sha256"])
        for name, data in buffers.items():
            writer.writerow([name, len(data), sha256(data)])
        concatenated = b"".join(buffers[name] for name in ("x", "residual", "gamma", "bias"))
        writer.writerow(["x+residual+gamma+bias", len(concatenated), sha256(concatenated)])

    executables = {
        "parent": BUILD / "selective_v001_parent_correctness",
        "candidate": BUILD / "selective_v001_candidate_correctness",
    }
    for name, path in executables.items():
        if not path.is_file() or not os.access(path, os.X_OK):
            raise RuntimeError(f"missing executable: {path}")

    manifest = {
        "device": int(DEVICE),
        "shape": [ROWS, WIDTH],
        "dtype": "FP32",
        "source_commit": "fe4f61166c5fd356b216b32d1a8ac55b5d2f3275",
        "candidate_source_sha256": candidate_source_sha,
        "parent_source_sha256": parent_source_sha,
        "parent_sidecar_sha256": sidecar_sha,
        "parent_source_include_paths": parent_includes,
        "candidate_source_include_paths": candidate_includes,
        "candidate_dispatch_resolution": {
            "rank": 2,
            "shape": [2, 16384],
            "dtype_code": DTYPE,
            "donor_condition_matches": False,
            "selected_entry": "add_rms_norm_bias_custom<float>",
            "selector_source_sha256": candidate_source_sha,
        },
        "candidate_executable_sha256": file_sha256(executables["candidate"]),
        "parent_executable_sha256": file_sha256(executables["parent"]),
        "runs": [],
    }

    with (RESULT / "repeat-summary.tsv").open("w", newline="") as target:
        writer = csv.writer(target, delimiter="\t", lineterminator="\n")
        writer.writerow([
            "attempt", "variant", "device", "rows", "width", "dtype",
            "rc", "max_abs", "bad", "matched_ratio", "output_bytes",
            "output_sha256", "log",
        ])
        for variant, attempt in (("parent", 1), ("candidate", 1),
                                 ("parent", 2), ("candidate", 2)):
            executable = executables[variant]
            tag = f"{variant}_r{ROWS}_d{WIDTH}_t{DTYPE}_rep{attempt}"
            output = RESULT / f"{tag}.bin"
            result = RESULT / f"{tag}.tsv"
            log = RESULT / f"{tag}.log"
            command = [
                str(executable), DEVICE, str(ROWS), str(WIDTH), str(DTYPE),
                str(output), str(result),
            ]
            completed = subprocess.run(command, capture_output=True, text=True, check=False)
            log.write_text(
                "COMMAND=" + " ".join(command) + "\n"
                + "--- stdout ---\n" + completed.stdout
                + "--- stderr ---\n" + completed.stderr
                + f"RC={completed.returncode}\n",
                encoding="utf-8",
            )
            fields = result_fields(result) if result.is_file() else {}
            output_hash = file_sha256(output) if output.is_file() else "MISSING"
            output_bytes = output.stat().st_size if output.is_file() else 0
            writer.writerow([
                attempt, variant, DEVICE, ROWS, WIDTH, "FP32", completed.returncode,
                fields.get("max_abs", "NA"), fields.get("bad", "NA"),
                fields.get("matched_ratio", "NA"), output_bytes, output_hash,
                log.name,
            ])
            manifest["runs"].append({
                "attempt": attempt,
                "variant": variant,
                "rc": completed.returncode,
                "max_abs": fields.get("max_abs", "NA"),
                "bad": fields.get("bad", "NA"),
                "matched_ratio": fields.get("matched_ratio", "NA"),
                "output_bytes": output_bytes,
                "output_sha256": output_hash,
                "log": log.name,
            })

    (RESULT / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    print((RESULT / "input-sha256.tsv").read_text(encoding="utf-8"), end="")
    print((RESULT / "repeat-summary.tsv").read_text(encoding="utf-8"), end="")
    print(f"manifest={RESULT / 'manifest.json'}")
    return 0 if all(run["rc"] in (0, 3) for run in manifest["runs"]) else 1


if __name__ == "__main__":
    raise SystemExit(main())
