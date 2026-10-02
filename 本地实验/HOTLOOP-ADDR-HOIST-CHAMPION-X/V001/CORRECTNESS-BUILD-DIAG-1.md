# V001 Correctness Diagnostic Harness Build 1 — PASS

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
SUPPORT_COMMIT=cd0e3a5a8d9c8ccbab21b869fb34f1b3fb4f0312
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
HOST=hwnput3
DEVICE_PLATFORM=Ascend910B3
DATE_UTC=2026-10-02
CANN=8.5.0.alpha002
BUILD_DIR=本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness
```

## Result

```text
CMAKE_CONFIGURE=PASS (existing support configuration)
ASC_COMPILE=PASS
LINK=PASS
BUILD_DIAG1_RC=0
CORRECTNESS=NOT_RUN_AT_BUILD_RECORD
LOCAL_PERFORMANCE=NOT_STARTED
```

This support-only diagnostic adds launch labels and an optional case filter;
it does not change inputs, golden arithmetic, tolerances, parent/candidate
source, or the H3 kernel.

Executable identity:

```text
PATH=本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness/addr_v001_correctness
SHA256=267cf5e730a75e6d6eef41660db05e4d6057d7acc8d0352114d7e27a4abc3759
FORMAT=ELF 64-bit LSB pie executable, ARM aarch64, dynamically linked, not stripped
SIZE_BYTES=572144
```

## Exact command and evidence

```text
set +u; source /usr/local/Ascend/ascend-toolkit/set_env.sh; set -u
cmake --build 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness --parallel 1 --verbose
```

Log:

- `logs/correctness-build-diag1-local-hwnput3-20261002.txt`

The next event is a support-diagnostic D40960 correctness run using this exact
executable and system-first runtime library path.
