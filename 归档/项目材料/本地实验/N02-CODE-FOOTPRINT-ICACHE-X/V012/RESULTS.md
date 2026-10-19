# N02 CODE-FOOTPRINT-ICACHE-X V012

| Field | Value |
|---|---|
| ROUTE | N02 CODE-FOOTPRINT-ICACHE-X |
| REVISION | V012 |
| PARENT_REVISION | V011 |
| PARENT_SOURCE_COMMIT | `d8b1331eaf91697cc5bd928ff9ac55a9906b88e1` |
| CANDIDATE_SOURCE_COMMIT | `3fba7ee209e56b68f4e813ac0817120b4515e366` |
| SOURCE_SHA256 | `6cdcd84a197061d3d0f81b962309628860cda9fa2892892bdf544d5972c1f27c` |
| SOURCE | `/Users/sunyiyang/Desktop/Project/cann/worktrees/n02-code-footprint-icache/本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V012/submission.asc` |
| REMOTE_HOST | cann-server3 |
| SOC / DEVICE | Ascend910B3 / d6 (NPU 6) |
| TOOLCHAIN | CANN 8.5.0.alpha002 |
| REMOTE_RUN | `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V012/6cdcd84a197061d3d0f81b962309628860cda9fa2892892bdf544d5972c1f27c` |

## Change

V012 moves the final pass-2 tile out of the steady loop. The steady loop now handles only tiles with a successor; the terminal tile is processed in a separate block. Parameter values, load conditions and addresses, DataCopy calls, and event types remain the same. This changes generated control flow and can affect the relative timing of prefetch and compute, so any timing change is a mixed control-flow and overlap result, not an isolated instruction-cache result.

## Compile And Device Code

Parent V011 and Candidate V012 configure/full-link completed under CANN 8.5.0.alpha002 with return code 0. The existing V016 paired Runner build also completed with return code 0 after loading the configured CANN environment. The first Runner build attempt returned 2 because `ASCEND_HOME_PATH` was absent from that compiler process; the failed log is retained. No Parent or Candidate device build was repeated.

| Artifact | Parent V011 | Candidate V012 | Difference |
|---|---:|---:|---:|
| `device.cpp.o` | 334040 B | 343024 B | +8984 B |
| `device.alink` | 183144 B | 191704 B | +8560 B |
| FP16 `ProcessWideLowPrecision` | 0x1234 B | 0x1d4c B | +0xb18 B |
| BF16 `ProcessWideLowPrecision` | 0x1248 B | 0x1cf8 B | +0xab0 B |

The parent/candidate device objects and linked device artifacts compare different at the byte level. Object files, linked artifacts, symbol tables, and disassembly text are saved under `results/artifacts/` and `results/compile/`. `llvm-objdump` marks device instruction words as unavailable, so this confirms generated code size/layout changed but does not establish instruction scheduling or an isolated instruction-cache effect.

## Correctness

The first Runner launch returned 1 before executing a case because its dynamic loader selected the CANN toolchain `libstdc++.so.6`, which lacks `GLIBCXX_3.4.29`. `loader-diagnosis.txt` records that default resolution and the existing system library selected by the corrected environment. The first status and log are retained.

The existing V016 Runner then ran with `/usr/lib/aarch64-linux-gnu` first in `LD_LIBRARY_PATH` and passed all four Parent V011 / Candidate V012 cases on d6.

- CORRECTNESS: PASS 4/4
- CORRECTNESS_RC: 0
- RESULT: `results/correctness/paired-correctness-attempt-02.log`

## Local

The existing V016 Runner used 45 warmups and 21 interleaved pairs per case on d6. Device-event time is primary; wall time is diagnostic. Raw samples and summaries are in `results/local/paired-local.log`.

| Case | Parent median us | Candidate median us | Paired delta median us | Delta p10..p90 us | Delta min..max us | Delta MAD us |
|---|---:|---:|---:|---:|---:|---:|
| FP16 tail D=12288 | 6.040 | 5.800 | -0.080 | -8.680..0.200 | -100.860..173.920 | 0.180 |
| FP16 wide D=32768 | 10.220 | 10.260 | 0.000 | -108.680..10.660 | -145.460..172.220 | 0.400 |
| BF16 tail D=12288 | 6.800 | 6.320 | -0.280 | -0.780..8.480 | -9.860..61.820 | 0.340 |
| BF16 wide D=32768 | 13.800 | 12.980 | -0.800 | -1.220..-0.580 | -122.620..37.380 | 0.140 |

d6 reported 91% HBM use and 0% AICore use before correctness and local measurement. `VLLMWorker_TP` PID 3357697 held 56355 MB before and after sampling. Large outliers and this sustained device occupancy make the medians descriptive only.

- LOAD_QUALITY: LOW
- LOCAL_SCORE: NONE
- LOCAL_DELTA: NONE
- LOCAL_RC: 0
- INTERPRETATION: The paired medians are mixed in size and the raw samples contain large outliers under sustained HBM occupancy. No performance conclusion is supported.

## Evidence

- Compile logs, status, artifact sizes, symbols, disassembly, and byte comparisons: `results/compile/` and `results/artifacts/`.
- Correctness logs, statuses, and loader diagnosis: `results/correctness/`.
- Paired samples and status: `results/local/`.
- d6 snapshots before build, correctness, local sampling, and after sampling: `results/snapshots/`.
