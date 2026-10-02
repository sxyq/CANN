# V001 Correctness Harness Build Fix 1 — PASS

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V001
HARNESS_FIX_COMMIT=72dac4a0c91d53717abc2196bdb205f83b179799
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
CMAKE_CONFIGURE=PASS
ASC_COMPILE=PASS
LINK=PASS
BUILD_FIX1_RC=0
CORRECTNESS=NOT_RUN_AT_BUILD_RECORD
LOCAL_PERFORMANCE=NOT_STARTED
```

The only fix was a support-only C-style cast at the correctness runner GM
address call sites. `submission.asc` was not modified and the H3 source
identity is unchanged.

Executable identity:

```text
PATH=本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness/addr_v001_correctness
SHA256=fe4607229e2a962e5d2d39dcc4b510cb13d7cf0b6ce265bf875c470e4eb3f770
FORMAT=ELF 64-bit LSB pie executable, ARM aarch64, dynamically linked, not stripped
SIZE_BYTES=571304
```

## Exact command and evidence

```text
set +u; source /usr/local/Ascend/ascend-toolkit/set_env.sh; set -u
cmake --build 本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/build-correctness --parallel 1 --verbose
```

Log:

- `logs/correctness-build-fix1-local-hwnput3-20261002.txt`

The next event is the exact Parent/Candidate NPU correctness matrix on the
same runner and device. No qualification or performance was run here.
