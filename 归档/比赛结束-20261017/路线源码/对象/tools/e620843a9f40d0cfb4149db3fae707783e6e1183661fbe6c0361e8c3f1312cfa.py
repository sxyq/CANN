#!/usr/bin/env python3
"""Audit committed input views without running a device or changing a kernel."""

import re

from ub_bank_audit import PARENT_PATH, PARENT_REF, body, git_text, source


SOURCES = [
    (PARENT_REF, None),
    ("64e32f535348cdde1828cd8a8fd89fad100aaa33", "本地实验/UB-BANK-LAYOUT-CHAMPION-X/"),
    ("6321ad4427b1819d172c719ebde190eb447ce9ae", "本地实验/ADAPTIVE-CORE-OWNERSHIP-CHAMPION-X/"),
    ("ce6c6dc568256ac8b80b674096bd7ca081b897cb", "本地实验/MULTIROW-PANEL-RMS-CHAMPION-X/"),
    ("1efa0863611f1a9b76ab13dd361eba161b32b46d", "本地实验/CROSSROW-FULL-PIPELINE-CHAMPION-X/"),
    ("93f15d9b", "本地实验/W4-R01/"),
    ("60ca277d", "本地实验/W4-R02/"),
    ("fa9b19bb", "本地实验/W4-R08/"),
    ("96044629", "本地实验/W4-R09/"),
    ("5b7b9215", "本地实验/W4-R10/"),
    ("4243f4e9", "本地实验/W4-R11/"),
]
MARKER = "__aicore__ inline void ProcessNarrowMidOverlap("


def selected_paths(ref, prefix):
    paths = git_text("-c", "core.quotePath=false", "ls-tree", "-r", "--name-only", ref).splitlines()
    if prefix is None:
        return [p for p in paths if p.endswith("submission.asc") and
                re.search(r"/(?:R031|R31A|R31B|MIX[^/]*|STORE[^/]*|EPILOGUE[^/]*|UB-LIVENESS[^/]*|COEFF[^/]*)/", p)]
    return [p for p in paths if p.startswith(prefix) and
            p.endswith(("/submission.asc", "/Candidate.asc", "/candidate.asc"))]


def main():
    print("MODE=COMMITTED_NARROW_INPUT_SOURCE_AUDIT; DEVICE_EXECUTION=NONE")
    parent = source(PARENT_REF, PARENT_PATH)
    parent_body = body(parent, MARKER)
    assert "AscendC::LocalTensor<T> residualLocal = residualBuf_.Get<T>();" in parent_body
    assert "Load(residualLocal, residualGm_, rowOffset, valid);" in parent_body
    assert "AscendC::Add(valueLocal, xLocal, residualLocal, valid);" in parent_body
    print("PARENT_INPUT_LOAD_AND_FP32_ADD_USE_SAME_VIEW=PASS")
    total = 0
    for ref, prefix in SOURCES:
        paths = selected_paths(ref, prefix)
        print(f"SOURCE={ref}; PREFIX={prefix}; FILE_COUNT={len(paths)}")
        assert paths, (ref, prefix)
        for path in paths:
            text = source(ref, path)
            total += 1
            if MARKER not in text:
                print(f"  FILE={path}; NARROW_MID_PRESENT=NO")
                continue
            function = body(text, MARKER)
            view_lines = [line.strip() for line in function.splitlines()
                          if re.search(r"\b(?:xLocal|residualLocal)\s*=|xBuf_|residualBuf_", line)]
            print(f"  FILE={path}; SAME_NARROW_MID_AS_PARENT={function == parent_body}")
            for line in view_lines:
                print("    VIEW=" + line)
    print(f"SOURCE_FILES_READ={total}")
    print("MODEL: input capacity=4096 float; positive aligned offset s requires s+round_up(D,8)<=4096")
    print("MODEL: x_start=0; residual_start=0x4000; group phase difference=(s/8) mod 16")
    print("MODEL: unchanged allocations preserve every other buffer start")
    print("LIMIT: a source audit does not establish compiled addresses, device accuracy, or timing")


if __name__ == "__main__":
    main()
