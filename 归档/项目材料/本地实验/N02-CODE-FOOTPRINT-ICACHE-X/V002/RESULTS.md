# N02-CODE-FOOTPRINT-ICACHE-X V002

ROUTE: N02-CODE-FOOTPRINT-ICACHE-X  
REVISION: V002  
SOURCE_PARENT: aac8b47ab640902ef2ed4302650c69c7247aac0d  
V002_COMMIT: 12a7676ad947dfaf05fabb8e96651b0bbbb59006  
SUPPORT_BIND_COMMIT: 38d5570116cd02fc97ae04692105fa2f38a9b2bd  
SOURCE_SHA256: d100688377f62b9307ea4372327336d89f937fbe310bb0b8de39531cb1f6a9c8  
DEVICE: d5 / NPU 5 / hwnput3  
REMOTE_RUN: ~/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V002/12a7676ad947dfaf05fabb8e96651b0bbbb59006  

## Change

Added one class-local helper, `PrepareWideLowPrecisionValue`, to share the existing input conversion, barrier, and add sequence between `ComputeWideLowPrecisionInvRms` and its double-buffered variant. The input path, arithmetic order, synchronization sequence, shapes, dtypes, and launch ABI are unchanged. No dispatch policy was added.

## Compile

- The first Parent/Candidate `full_link` attempt returned 2 because the existing `device.cpp` and `adapter.cpp` still included V001. The raw failure is retained.
- After the two existing support includes were bound to V002 in `38d55701`, Parent and Candidate `full_link` both returned 0.
- Existing paired CMake configure returned 0; existing `paired_runner_v016` build returned 0.
- The separate probe targets were not built or used.

## Correctness

The existing paired runner ran `--correctness` on d5 for FP16/BF16 with widths 12288/32768 and rows=2. Parent and V002 Candidate passed all four cases with zero failures. Return code: 0.

## Device Artifact Result

| artifact | Parent | V002 Candidate | difference |
|---|---:|---:|---:|
| `device.cpp.o` bytes | 334040 | 333912 | -128 |
| executable section bytes | 77340 | 77288 | -52 |
| `device.alink` bytes | 183144 | 183144 | 0 |

The V002 Candidate object SHA is `8bfa93d106b610f7e5499d846c9b46d71c107c61e1c6ada7b7b5f1506c2d0a42`; it is byte-identical to the V001 Candidate object. The Parent object, both `.alink` SHA values, differing byte counts, and raw section evidence are in `results/compile/artifact-comparison.txt`. `llvm-objdump` completed for both objects; `device-objdump.diff` is retained.

Conclusion: the new helper was fully inlined by the CANN compiler and produced no new device code layout or size change relative to V001. V002 is a negative code-generation result.

## Local

The same paired runner ran `--paired` with 45 warmups and 21 interleaved pairs for each of the four cases and returned 0. Raw samples are in `results/local/paired-local.log`.

I read all 21 Parent/Candidate pairs for each V001 case before selecting the next hypothesis. V001 had timing spikes in every case: FP16-tail reached 113.880/146.960 us, FP16-wide reached 148.300/81.660 us, BF16-tail reached 124.480/119.360 us, and BF16-wide reached 96.180/152.860 us. Its paired median deltas were +0.060, -0.060, -0.160, and -0.860 us, with wide p10/p90 intervals reaching -21.600/+0.500 us and -35.820/+6.320 us.

V002 showed the same issue. Its paired median deltas were -0.140, +0.060, -0.300, and -0.840 us for FP16-tail, FP16-wide, BF16-tail, and BF16-wide. Several cases still had outliers up to 161.520 us or parent-side 142.560 us. Therefore `LOCAL_SCORE=NONE` and `LOCAL_DELTA=NONE`; medians remain diagnostic only.

## Device Snapshot

Before V002 Local, d5 reported 59876/65536 MB HBM in use, approximately 5660 MB free, and AICore 0%. Existing server load and process listings were recorded and left unchanged. The full pre/post snapshots are under `results/snapshots/`.

## Evidence Files

`results/compile/` contains both failed and successful full_link logs, paired build logs, artifact size/SHA evidence, raw object disassembly, and assembly diff. `results/correctness/` and `results/local/` contain raw paired-runner output and return codes. `results/artifacts/` contains the four Parent/V002 device artifacts. No probe executable or full build directory was copied into the repository.

