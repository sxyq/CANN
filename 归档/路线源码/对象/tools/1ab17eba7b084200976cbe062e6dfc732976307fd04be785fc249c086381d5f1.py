#!/usr/bin/env python3
"""Recompute the pooled fields of local-result.json from the raw per-block TSV.

This script performs a post-RESULT evidence correction.  It changes no
Candidate source and introduces no new performance change.  Every pooled
number it writes is derived from the permanent raw samples under
results/, and the values it replaces are kept side by side so the
correction stays auditable.

Method, matching 归档/历史控制文件/local-timing-protocol.md:
  pooled median      median over all raw device-event samples of the blocks
  pooled delta       pooled candidate median - pooled parent median
  pooled percent     pooled delta / pooled parent median
  paired delta       median over the per-sample delta column
  block MAD/median   median(|x - median|) / median, full sample set
  block drift        max |block median - pooled median| / pooled median,
                     taken over the Parent and Candidate block medians
                     together
"""

import json
import os
import statistics
import sys


def median(values):
    return statistics.median(values)


def mad_over_median(values):
    m = median(values)
    if m == 0:
        return 0.0
    return median([abs(x - m) for x in values]) / m


def read_block(path):
    parent, candidate, delta = [], [], []
    with open(path) as handle:
        for line in handle:
            if line.startswith("#") or line.startswith("sample"):
                continue
            fields = line.rstrip("\n").split("\t")
            if len(fields) < 5:
                continue
            parent.append(float(fields[2]))
            candidate.append(float(fields[3]))
            delta.append(float(fields[4]))
    return parent, candidate, delta


def recompute_shape(shape, blocks, per_block):
    pooled_parent, pooled_candidate, pooled_delta = [], [], []
    favor_candidate = favor_parent = ties = 0
    block_parent_medians = []
    block_candidate_medians = []
    block_parent_mad = []
    block_candidate_mad = []
    gates = []

    for block in blocks:
        path = per_block[block["block"]]["file"]
        parent, candidate, delta = read_block(path)
        pooled_parent.extend(parent)
        pooled_candidate.extend(candidate)
        pooled_delta.extend(delta)
        favor_candidate += sum(1 for x in delta if x < 0)
        favor_parent += sum(1 for x in delta if x > 0)
        ties += sum(1 for x in delta if x == 0)
        parent_median = median(parent)
        candidate_median = median(candidate)
        block_parent_medians.append(parent_median)
        block_candidate_medians.append(candidate_median)
        parent_mad = mad_over_median(parent)
        candidate_mad = mad_over_median(candidate)
        block_parent_mad.append(parent_mad)
        block_candidate_mad.append(candidate_mad)
        gates.append(
            {
                "block": block["block"],
                "parent_median_us": parent_median,
                "parent_mad_over_median": parent_mad,
                "parent_gate": "PASS" if parent_mad <= 0.10 else "FAIL",
                "candidate_median_us": candidate_median,
                "candidate_mad_over_median": candidate_mad,
                "candidate_gate": "PASS" if candidate_mad <= 0.10 else "FAIL",
            }
        )

    pooled_parent_median = median(pooled_parent)
    pooled_candidate_median = median(pooled_candidate)
    pooled_delta_us = pooled_candidate_median - pooled_parent_median
    all_block_medians = block_parent_medians + block_candidate_medians
    all_pooled_medians = [pooled_parent_median, pooled_candidate_median]
    drift_reference = median(all_block_medians + all_pooled_medians)
    block_drift = (
        max(abs(x - drift_reference) for x in all_block_medians + all_pooled_medians)
        / drift_reference
    )

    original = {
        "pooled_parent_median_us": shape.get("pooled_parent_median_us"),
        "pooled_candidate_median_us": shape.get("pooled_candidate_median_us"),
        "pooled_local_delta_us": shape.get("pooled_local_delta_us"),
        "pooled_local_delta_pct": shape.get("pooled_local_delta_pct"),
        "pooled_median_paired_delta_us": shape.get("pooled_median_paired_delta_us"),
        "pooled_favor_candidate": shape.get("pooled_favor_candidate"),
        "pooled_favor_parent": shape.get("pooled_favor_parent"),
        "block_median_drift_over_median": shape.get("block_median_drift_over_median"),
    }

    corrected = {
        "pooled_parent_median_us": pooled_parent_median,
        "pooled_candidate_median_us": pooled_candidate_median,
        "pooled_local_delta_us": pooled_delta_us,
        "pooled_local_delta_pct": 100.0 * pooled_delta_us / pooled_parent_median,
        "pooled_median_paired_delta_us": median(pooled_delta),
        "pooled_favor_candidate": favor_candidate,
        "pooled_favor_parent": favor_parent,
        "pooled_ties": ties,
        "sample_count": len(pooled_parent),
        "block_count": len(blocks),
        "parent_block_medians_us": block_parent_medians,
        "candidate_block_medians_us": block_candidate_medians,
        "parent_block_mad_over_median": block_parent_mad,
        "candidate_block_mad_over_median": block_candidate_mad,
        "block_median_drift_over_median": block_drift,
        "block_mad_gate": gates,
        "block_mad_gate_note": (
            "gate = full-sample MAD/median <= 0.10 from "
            "归档/历史控制文件/local-timing-protocol.md"
        ),
        "provenance": {
            "status": "RECOMPUTED_FROM_RAW_SAMPLES",
            "source_files": [per_block[b["block"]]["file"] for b in blocks],
            "method": "median over all raw device-event samples of the listed blocks",
            "superseded_values": original,
        },
    }

    # Preserve descriptive fields the previous record wrote by hand.
    for key, value in shape.items():
        if key not in corrected and key not in ("blocks",):
            corrected[key] = value
    corrected["blocks"] = blocks
    return corrected


def main():
    result_path = sys.argv[1]
    with open(result_path) as handle:
        record = json.load(handle)

    per_block = record["per_block"]
    for shape_name, shape in record["shape_results"].items():
        record["shape_results"][shape_name] = recompute_shape(
            shape, shape["blocks"], per_block
        )

    with open(result_path, "w") as handle:
        json.dump(record, handle, indent=2, ensure_ascii=False)
        handle.write("\n")

    for shape_name, shape in record["shape_results"].items():
        old = shape["provenance"]["superseded_values"]
        print(
            f"{shape_name}: "
            f"{old['pooled_local_delta_pct']:+.4f}% -> "
            f"{shape['pooled_local_delta_pct']:+.4f}%  "
            f"P={shape['pooled_parent_median_us']:.6f} "
            f"C={shape['pooled_candidate_median_us']:.6f}"
        )


if __name__ == "__main__":
    main()
