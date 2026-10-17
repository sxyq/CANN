#!/usr/bin/env python3
"""Read committed R1 evidence and replay the FP16 wide allocation expressions."""

import difflib
import re
import statistics
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
R1_REF = "64e32f535348cdde1828cd8a8fd89fad100aaa33"
PARENT_REF = "de70b634813dea80783fc57716d6e95c158edeec"
PARENT_PATH = "线上结果/R31B/V011/submission.asc"
R1_ROOT = "本地实验/UB-BANK-LAYOUT-CHAMPION-X"


def git_text(*args):
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True)


def source(ref, path):
    return git_text("show", f"{ref}:{path}")


def body(text, marker):
    start = text.index("{", text.index(marker))
    depth = 1
    end = start + 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    return text[start + 1:end - 1]


def constant(text, name):
    expr = re.search(r"static constexpr int32_t " + name + r"\s*=\s*([^;]+);", text)[1]
    return eval(expr, {"__builtins__": {}}, {})


def bank_location(address):
    """Formula transcribed from the 2201 official diagram, not a device probe."""
    assert 0 <= address < 192 * 1024
    group = address // 32 % 16
    return 16 * (address // 65536) + group, group, address % 65536 // 512


def wide_half_plan(text, width=32768):
    tile = constant(text, "kWideFullYTileElems")
    stride = constant(text, "kWideFullYReduceStride")
    budget = constant(text, "kWideFullYBudgetBytes")
    max_rows = constant(text, "kWideFullYMaxRows")
    rows = 1
    for trial in range(max_rows, 1, -1):
        if width * 2 * trial + 4 * tile * 2 + 3 * tile * 4 + trial * stride * 4 <= budget:
            rows = trial
            break
    if rows == 1:
        while tile > 2048:
            if width * 2 + 4 * tile * 2 + 3 * tile * 4 + stride * 4 <= budget:
                break
            tile -= 512
    init = body(text, "__aicore__ inline void Init(")
    half_init = body(init, "else if constexpr (std::is_same<T, half>::value)")
    half_init = re.sub(r"//[^\n]*", "", half_init)
    expressions = re.findall(r"pipe_\.InitBuffer\((\w+),\s*([\s\S]*?)\);", half_init)
    values = {"tileElems": tile, "reduceStride": stride,
              "wideFullYRows_": rows, "yElemsPerRow": width}
    allocations = {}
    cursor = 0
    for name, expr in expressions:
        expr = " ".join(expr.split()).replace("sizeof(half)", "2").replace("sizeof(float)", "4")
        requested = eval(expr, {"__builtins__": {}}, values)
        allocated = (requested + 31) // 32 * 32
        allocations[name] = (cursor, requested, allocated)
        cursor += allocated
    assert len(allocations) == 8
    assert cursor <= 192 * 1024
    return allocations, tile, rows, cursor


def fields(line):
    return dict(re.findall(r"(\w+)=([^\s]+)", line))


def print_old_samples(log, path, revision):
    samples = {}
    summaries = {}
    for line in log.splitlines():
        if line.startswith("SAMPLE "):
            row = fields(line)
            samples.setdefault(row["case"], []).append(
                (float(row["parent_device_us"]), float(row["candidate_device_us"])))
        if line.startswith("SUMMARY ") and "metric=device_us " in line:
            row = fields(line)
            summaries[(row["case"], row["binary"])] = row
    assert len(samples) == 2
    print("RAW_SOURCE=" + R1_REF + ":" + path)
    for case, pairs in samples.items():
        assert len(pairs) == 21
        parent = [x[0] for x in pairs]
        candidate = [x[1] for x in pairs]
        pm, cm = statistics.median(parent), statistics.median(candidate)
        ps = summaries[(case, "V011")]
        cs = summaries[(case, "UB_BANK_" + revision)]
        assert abs(pm - float(ps["median"])) < 0.000001
        assert abs(cm - float(cs["median"])) < 0.000001
        delta = (cm / pm - 1) * 100
        paired = statistics.median(c - p for p, c in pairs)
        print(f"OLD_LOCAL revision={revision} case={case} pairs=21 "
              f"parent_median_us={pm:.3f} candidate_median_us={cm:.3f} "
              f"median_ratio_delta_pct={delta:+.6f} paired_delta_median_us={paired:+.3f} "
              f"parent_cv={ps['cv']} candidate_cv={cs['cv']} "
              f"parent_mad_us={ps['mad']} candidate_mad_us={cs['mad']}")
    for line in log.splitlines():
        if line.startswith(("CORRECTNESS ", "RUNNER_COMPLETE ")):
            print("OLD_RUNNER " + line)


def main():
    print("MODE=SOURCE_MODEL_AND_ARCHIVED_SAMPLE_RECOMPUTATION")
    print("DEVICE_EXECUTION=NONE; NEW_PERFORMANCE_REVISIONS=0; LOCAL_SCORE=NONE")
    print("PARENT=" + PARENT_REF + ":" + PARENT_PATH)
    examples = {0x00000: 0, 0x00020: 1, 0x10000: 16, 0x10020: 17,
                0x10E20: 17, 0x1FE00: 16, 0x1FF00: 24, 0x20000: 32,
                0x20020: 33, 0x20200: 32}
    for address, expected_bank in examples.items():
        bank, group, row = bank_location(address)
        assert bank == expected_bank
        print(f"DOC_MODEL address={address:#07x} bank={bank} group={group} row={row}")
    print("DOC_MODEL_EXAMPLES=PASS count=10; not empirical hardware validation")

    parent = source(PARENT_REF, PARENT_PATH)
    assert parent == source(R1_REF, PARENT_PATH)
    ref_check = body(source(R1_REF, f"{R1_ROOT}/V001/support/paired_runner.cpp"), "void CheckOutput(")
    paths = git_text("-c", "core.quotePath=false", "ls-tree", "-r", "--name-only",
                     R1_REF, "--", R1_ROOT).splitlines()
    parent_plan, _, _, _ = wide_half_plan(parent)
    plans = {}
    for number in range(11):
        revision = "PARENT" if number == 0 else f"V{number:03d}"
        text = parent if number == 0 else source(R1_REF, f"{R1_ROOT}/{revision}/submission.asc")
        plan, tile, rows, size = wide_half_plan(text)
        plans[revision] = plan
        print(f"SOURCE_LAYOUT revision={revision} dtype=FP16 width=32768 "
              f"tile_elems={tile} allocated_y_rows={rows} total_ub_bytes={size}")
        for name, (address, requested, allocated) in plan.items():
            bank, group, row = bank_location(address)
            print(f"  {name} address={address:#07x} requested={requested} allocated={allocated} "
                  f"bank={bank} group={group} bank_row={row}")
        x, r = plan["xBuf_"][0], plan["residualBuf_"][0]
        overlaps = []
        for slot in (0, 1):
            for repeat in range(tile * 2 // 256):
                offset = slot * tile * 2 + repeat * 256
                xg = {bank_location(x + offset + 32 * i)[1] for i in range(8)}
                rg = {bank_location(r + offset + 32 * i)[1] for i in range(8)}
                overlaps.append(len(xg & rg))
        print(f"SOURCE_READ_GROUP_OVERLAP revision={revision} "
              f"per_256B_repeat_unique_counts={sorted(set(overlaps))}; no cycle prediction")
        if number:
            print("EXACT_SOURCE_DIFF " + revision)
            print("".join(difflib.unified_diff(parent.splitlines(True), text.splitlines(True),
                                               fromfile="R31B-V011", tofile=revision, n=2)))
            runner = source(R1_REF, f"{R1_ROOT}/{revision}/support/paired_runner.cpp")
            assert body(runner, "void CheckOutput(") == ref_check
            local_path = next(p for p in paths if f"/{revision}/support/logs/local-" in p)
            print_old_samples(source(R1_REF, local_path), local_path, revision)
    assert all(plans["V008"][k][0] == v[0] for k, v in parent_plan.items())
    assert bank_location(parent_plan["xBuf_"][0])[:2] == bank_location(plans["V001"]["xBuf_"][0])[:2]
    print("R1_REFERENCE_FUNCTIONS_EQUAL=PASS count=10")
    print("R1_ARCHIVED_SAMPLE_MEDIANS=PASS cases=20 pairs=420 side_samples=840")
    print("R1_V008_USED_START_ADDRESSES_UNCHANGED=PASS")
    print("RESULT=RESEARCH_ONLY; CURRENT_LOCAL_BEST=NONE; OFFICIAL_SCORE=NONE; ONLINE_STATE=PAUSED")


if __name__ == "__main__":
    main()
