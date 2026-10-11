# N02-CODE-FOOTPRINT-ICACHE-X V006

- ROUTE: N02-CODE-FOOTPRINT-ICACHE-X
- REVISION: V006
- PARENT: R31B V011 / Official 45.16
- PARENT_SOURCE_COMMIT: 7132c86d32e8fa1d57e2ae8edb7846e3b4775b2a
- CANDIDATE_SOURCE_COMMIT: 8eefd778133b449e4e10ca3c35bb6da0d708d099
- PAIRED_BIND_COMMIT: 5bb7d4ec884ed9da20e96c6db9d8884a6ded4e42
- FULL_LINK_BIND_COMMIT: c02b1c5f8884339a60d5042fa651cdd76be38c75
- BRANCH: route/n02-code-footprint-icache-x-20261011
- REMOTE_HOST: hwnput3
- SOC: Ascend910B3
- DEVICE: d6 / NPU 6
- TOOLCHAIN: CANN 8.5.0.alpha002
- REMOTE_RUN: `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V006/8eefd778133b449e4e10ca3c35bb6da0d708d099`

## Change

V006 starts from V011 and removes one of two adjacent, identical BF16 gamma/bias `ToFloat` conversion blocks and its vector barrier in pass 2. The remaining conversion still prepares both parameter vectors before the row loop. Arithmetic, data movement, scheduling, tile order, output semantics, shapes, dtypes, and launch ABI are unchanged. The single source change targets generated instruction count and placement in the BF16 hot function.

## Compile And Device Artifacts

The first attempt did not compile: the environment script initially saw unset `LD_LIBRARY_PATH`; after that, the remote support tree lacked two probe sources, and the paired compile lacked the GCC C++ include path. These logs are retained as attempt 1. After supplying the existing V016 probe sources, selecting the CANN 8.5.0.alpha002 environment, and using the host GCC 11 include paths, Parent/Candidate configure and `full_link`, plus the existing paired runner configure/build, all returned 0. Final status is `results/compile/compile-status-attempt-02.txt`.

| Artifact | Parent | Candidate | Difference |
|---|---:|---:|---:|
| `device.cpp.o` | 334040 B | 333912 B | -128 B |
| `device.alink` | 183144 B | 183144 B | 0 B |
| FP16 `ProcessWideLowPrecision` | 0x1234 B | 0x1234 B | 0 B |
| BF16 `ProcessWideLowPrecision` | 0x1248 B | 0x1214 B | -0x34 B |

Parent and Candidate object files and linked device files compare as different byte streams. `llvm-objdump` completed for both objects, and the disassembly differs. Objects, linked files, section tables, symbol tables, disassembly, and diff are retained under `results/artifacts/` and `results/compile/`.

## Correctness

On d6, Parent V011 and Candidate V006 each passed all four FP16/BF16 cases at widths 12288 and 32768, with zero failures. Runner return code: 0. Raw output and status are under `results/correctness/paired-correctness-attempt-02.*`.

The unsuffixed `paired-correctness-status.txt` is from the earlier launch and correctly records return code 1; its paired log shows the dynamic loader failure for `GLIBCXX_3.4.29`, before any case began. The later attempt 2 used the system GCC 11 runtime library; its separate status is 0 and its complete output ends with `RUNNER_COMPLETE mode=CORRECTNESS correctness=PASS cases=4`. Both attempts are retained. The successful paired Local log independently records PASS for the same four cases.

## Local

The existing paired runner compared V011 and V006 on d6 with 45 warmups and 21 interleaved pairs per case. Runner return code: 0; all 84 pairs completed.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta p10 / p90 (us) |
|---|---:|---:|---:|---:|
| FP16, width 12288 | 6.140 | 6.240 | +0.180 | -0.340 / +9.400 |
| FP16, width 32768 | 10.340 | 10.240 | -0.160 | -133.060 / +0.580 |
| BF16, width 12288 | 6.740 | 6.720 | -0.060 | -109.100 / +107.140 |
| BF16, width 32768 | 13.640 | 13.180 | -0.480 | -1.220 / +99.360 |

Large outliers dominate three cases: paired deltas range from -32.880 to +156.600 us, -137.680 to +14.980 us, -184.040 to +131.520 us, and -242.840 to +221.780 us in table order. Before Local, d6 used 59876/65536 MB HBM (5660 MB available), AICore was 0%, and an existing `VLLMWorker_TP` used 56355 MB. The same process remained after the run. The Local medians are diagnostic only.

LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
LOAD_QUALITY=LOW

The complete raw samples and summaries are in `results/local/paired-local-attempt-02.log`; device and process snapshots are under `results/snapshots/`.

## Earlier Unstarted Attempts

Compile attempt 1 and the first Correctness launch failed before device execution; their original logs and status files remain in `results/compile/` and `results/correctness/`. The first Local launch also stopped before Runner invocation because `results/local/` did not exist. Its shell error is retained in `results/local/paired-local-attempt-01-launch.log`; no samples came from that attempt. The successful attempt 2 evidence above is separate and was not rerun after completion.
