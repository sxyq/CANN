# N02-CODE-FOOTPRINT-ICACHE-X V001

ROUTE: N02-CODE-FOOTPRINT-ICACHE-X  
REVISION: V001  
PARENT: 1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f  
V001_COMMIT: aac8b47ab640902ef2ed4302650c69c7247aac0d  
SOURCE_SHA256: d9297e337f4aee09994ff6d7f41ad3d55ff172a1382ee042b44f791dea114c1c  
CHAMPION_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3  
DEVICE: d5 / NPU 5 / hwnput3  
REMOTE_RUN: ~/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V001/aac8b47ab640902ef2ed4302650c69c7247aac0d  

## Change

In the existing low-precision wide helper, removed the duplicate gamma/bias `ToFloat` pair and its following `PipeBarrier` in the output path. The input path, arithmetic order, shapes, dtypes, and launch ABI are unchanged. Source diff against the Champion is five deleted lines.

## Compile

- Parent and Candidate `support/CMakeLists.txt` `full_link`: PASS, including CANN 8.5 device objects and `.alink` outputs.
- The separate `probe_v016` target link failed with `rtLinkedDevBinaryRegisterDelay` and `__cce_rtKernelLaunch` undefined. This raw failure is retained in `compile/candidate-build.log`; it was not used for runtime testing.
- Existing paired CMake configure: PASS, return code 0.
- Existing `paired_runner_v016` final build with `ASCEND_HOME_PATH`, `CPATH`, CANN `set_env.sh`, and system C++ runtime path: PASS, return code 0.
- Final executable: `results/artifacts/paired_runner_v016` was not copied; the source and raw build logs are retained, while the required Parent/Candidate device artifacts were copied.

## Device Artifact Evidence

| artifact | Parent | Candidate | difference |
|---|---:|---:|---:|
| `device.cpp.o` bytes | 334040 | 333912 | -128 |
| executable section bytes | 77340 | 77288 | -52 |
| `device.alink` bytes | 183144 | 183144 | 0 |

The Parent and Candidate `.o` files differ at 106112 byte positions. The `.alink` files have equal size but differ at 12437 byte positions. `llvm-objdump` completed for both objects; `compile/device-objdump.diff` records the assembly text difference, including the removed instruction slots in the low-precision helper. SHA-256 values and raw section output are in `compile/artifact-comparison.txt`.

## Correctness

The existing paired runner was run with `--correctness` on all four public cases: FP16/BF16 x width 12288/32768, rows=2. Parent and Candidate both passed all four cases with zero failures. Final return code: 0.

## Local

The existing paired runner was run with `--paired` on d5 with 45 warmups and 21 interleaved pairs per case. Raw samples and summaries are in `local/paired-local.log`; the runner returned 0.

The samples contain large timing spikes and high variation, so `LOCAL_SCORE=NONE` and `LOCAL_DELTA=NONE`. Diagnostic paired device medians (Candidate minus Parent) were +0.060 us, -0.060 us, -0.160 us, and -0.860 us for FP16-tail, FP16-wide, BF16-tail, and BF16-wide respectively. These values are retained as raw diagnostics, not a performance score.

## Device Snapshot

Before Local, d5 reported 59877/65536 MB HBM in use, approximately 5659 MB free, and AICore 0%. Existing `VLLMWorker_TP` processes were recorded on d5 and were not changed. The host filesystem reported 43 GB available. Pre/post snapshots and process listings are under `results/snapshots/`.

## Evidence Files

`results/compile/` contains configure/build logs, return-code files, object/link size comparison, raw object disassembly, and the failed probe link. `results/correctness/` and `results/local/` contain raw runner output and return codes. `results/artifacts/` contains the four Parent/Candidate device artifacts needed for local comparison. No `.codegraph` files are included.

