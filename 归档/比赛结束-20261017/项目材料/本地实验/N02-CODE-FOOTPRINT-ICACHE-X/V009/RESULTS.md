# N02-CODE-FOOTPRINT-ICACHE-X V009

| Field | Value |
|---|---|
| ROUTE | N02-CODE-FOOTPRINT-ICACHE-X |
| REVISION | V009 |
| PARENT_REVISION | V008 |
| PARENT_SOURCE_COMMIT | d5739dc618126766d96ab83356858ef2fc82438f |
| PARENT_DEVICE_REFERENCE | R31B V011 / Official 45.16 |
| CANDIDATE_SOURCE_COMMIT | 389db691e67c0941989761e80934168498d831fe |
| SOURCE_SHA256 | 6c46e66c94007f606f3abf3ca5a76744ec6b1865a81081297e1d9b3f862a1e00 |
| SOURCE | `/Users/sunyiyang/Desktop/Project/cann/worktrees/n02-code-footprint-icache/本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V009/submission.asc` |
| REMOTE_HOST | cann-server3 |
| SOC | Ascend910B3 |
| DEVICE_ID | d6 / NPU 6 |
| TOOLCHAIN | CANN 8.5.0.alpha002 |
| REMOTE_RUN | `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V009/6c46e66c94007f606f3abf3ca5a76744ec6b1865a81081297e1d9b3f862a1e00` |

## Change

The Candidate groups two consecutive flattened pass-1 units under one outer loop trip. The flattened `u` value, row/tile calculations, address expressions, DataCopy calls, and per-unit event sequence remain in the same order. The outer/inner loop structure changes control-flow layout and can affect generated instruction scheduling. No host-only layout change was made.

## Compile And Device Code

Parent and Candidate configure and `full_link`, plus the existing V016 paired Runner configure/build, all returned 0. The saved compile status is `results/compile/compile-status.txt`.

| Artifact | Parent V011 | Candidate V009 | Change |
|---|---:|---:|---:|
| `device.cpp.o` | 334040 B | 334152 B | +112 B |
| `device.alink` | 183144 B | 183144 B | same length; contents differ |
| FP16 `ProcessWideLowPrecision` | 0x1234 B | 0x12ac B | +0x78 B |
| BF16 `ProcessWideLowPrecision` | 0x1248 B | 0x128c B | +0x44 B |

Both Candidate device files are byte-distinct from the V006 Candidate files. The object symbol tables establish changed hot-function sizes. `llvm-objdump` recognizes the `elf64-hiipu` file format but prints unavailable instruction words for these device sections; the saved disassembly therefore does not establish the emitted MTE instruction order. Source-level DataCopy and event order is unchanged, while compiler scheduling remains unverified. Treat this as a code-layout/control-flow experiment with possible pipeline interaction, not as an isolated I-cache performance result.

## Correctness

The first launch exited 1 before entering a case: the HCC `libstdc++.so.6` selected at runtime lacked `GLIBCXX_3.4.29`. That log and status are retained. A loader diagnosis confirmed that the system GCC 11 library at `/usr/lib/aarch64-linux-gnu/libstdc++.so.6` contains the required symbol and is selected when its directory is placed first in `LD_LIBRARY_PATH` after loading CANN.

Attempt 2 returned 0. Parent V011 and Candidate V009 passed all four FP16/BF16 cases at widths 12288 and 32768. The raw log and status are under `results/correctness/paired-correctness-attempt-02.*`.

- CORRECTNESS: PASS 4/4
- CORRECTNESS_RC: 0
- INITIAL_LOADER_ATTEMPT_RC: 1 (no case started)

## Local

The existing paired Runner ran on d6 with 45 warmups and 21 paired samples per case. Both binaries passed correctness during the paired run. Raw samples and summaries are in `results/local/paired-local.log`; status is `results/local/paired-local-status.txt`.

| Case | Parent median us | Candidate median us | Paired delta median us | Delta p10..p90 us | Delta min..max us | Delta MAD us |
|---|---:|---:|---:|---:|---:|---:|
| FP16 tail D=12288 | 6.100 | 6.080 | -0.160 | -163.400..0.480 | -178.620..160.040 | 0.300 |
| FP16 wide D=32768 | 10.080 | 10.160 | +0.020 | -5.440..1.280 | -122.460..71.420 | 0.200 |
| BF16 tail D=12288 | 7.380 | 6.700 | -0.440 | -82.060..20.980 | -156.900..100.500 | 4.960 |
| BF16 wide D=32768 | 13.720 | 13.180 | -0.680 | -1.000..5.120 | -83.520..13.440 | 0.320 |

The paired deltas are mixed and contain large outliers, including device latencies tens of times above the usual samples. Before sampling, d6 used 59876/65536 MB HBM (91%) with AICore at 0%; `VLLMWorker_TP` PID 3357697 held 56355 MB. The same process and similar load remained in the post-run snapshot. No process was stopped and the device was not reset.

- LOAD_QUALITY: LOW
- LOCAL_SCORE: NONE
- LOCAL_DELTA: NONE
- INTERPRETATION: Medians are descriptive only; noise and mixed signs do not support a performance conclusion.

## Evidence

- Compile logs, objects, linked files, symbol tables, and disassembly: `results/compile/` and `results/artifacts/`.
- Initial and successful Correctness logs/status, plus loader diagnosis: `results/correctness/`.
- Raw Local samples and return status: `results/local/`.
- d6 load snapshots: `results/snapshots/`. The first full host-process capture was narrowed to the NPU process table before submission so unrelated process arguments are not retained.

V001-V005 Official negative results remain unchanged. V008 remains a separate negative code-generation result because its Candidate device objects matched V006.
