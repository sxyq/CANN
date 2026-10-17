# N02-CODE-FOOTPRINT-ICACHE-X V003

ROUTE: N02-CODE-FOOTPRINT-ICACHE-X  
REVISION: V003  
SOURCE_PARENT: 12a7676ad947dfaf05fabb8e96651b0bbbb59006  
V003_COMMIT: 7132c86d32e8fa1d57e2ae8edb7846e3b4775b2a  
SUPPORT_BIND_COMMIT: c414a9ce  
SOURCE_SHA256: fa85bdc5b686d9dd49cdc6f0a6e6f02f25753ebd921cb31e12c5253fe829b22d  
DEVICE: d5 / NPU 5 / hwnput3  
REMOTE_RUN: ~/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V003/7132c86d32e8fa1d57e2ae8edb7846e3b4775b2a  

## Change

Added one `always_inline` attribute to `ComputeWideLowPrecisionInvRmsPipelined`, the hot wide low-precision reduction helper. Relative to V002, this is the only source change. Input handling, arithmetic, synchronization, shapes, dtypes, and launch ABI are unchanged. No dispatch policy or noinline attribute was added.

## Similarity Fact

Main's current audit reports `FULL_SOURCE_SIMILARITY` for the visible route set at approximately 0.982-0.999, above the 80% upper limit. This record keeps that fact unchanged; no names, comments, formatting, or empty wrappers were added to reduce it. Effective-change similarity is a separate Main-side statistic and is not filled with an invented value here.

## Compile

- Parent and Candidate CANN 8.5 `full_link`: PASS, both return code 0.
- Existing paired CMake configure: PASS, return code 0.
- Existing `paired_runner_v016` build: PASS, return code 0.
- Probe targets were not built or used.

## Correctness

The existing paired runner ran `--correctness` on d5 for FP16/BF16 with widths 12288/32768 and rows=2. Parent and V003 Candidate passed all four cases with zero failures. Return code: 0.

## Device Artifact Result

| artifact | Parent | V003 Candidate | difference |
|---|---:|---:|---:|
| `device.cpp.o` bytes | 334040 | 333912 | -128 |
| executable section bytes | 77340 | 77288 | -52 |
| `device.alink` bytes | 183144 | 183144 | 0 |

The V003 Candidate object SHA is `8bfa93d106b610f7e5499d846c9b46d71c107c61e1c6ada7b7b5f1506c2d0a42`, equal to V001 and V002. The `.alink` SHA values and 12437 differing byte positions are also unchanged. `llvm-objdump` completed for both objects; the raw disassembly and diff are retained.

Conclusion: CANN produced no new device code structure for this `always_inline` change. V003 is a negative code-generation result and does not establish a new executable-layout improvement.

## Local

The existing paired runner ran `--paired` with 45 warmups and 21 interleaved pairs per public case and returned 0. Raw samples are in `results/local/paired-local.log`.

Paired device median deltas (V003 minus V011) were +0.440 us, +0.100 us, -0.260 us, and -0.860 us for FP16-tail, FP16-wide, BF16-tail, and BF16-wide. Variation is too high for a score: FP16-wide included device samples up to 161.300/152.040 us and wall samples up to 352.745/1350.739 us; BF16-tail reached 85.840/150.100 us. Therefore `LOCAL_SCORE=NONE` and `LOCAL_DELTA=NONE`.

V001's four 21-pair raw blocks were read and summarized in V002's `RESULTS.md`; their spikes likewise did not support a score. V002's helper-sharing change and V003's inline attribute both produced the same Candidate object as V001.

The V002 raw 21-pair blocks were also read here. Their paired device median deltas and p10/p90 ranges were: FP16-tail -0.140 us (-39.580, +0.540), FP16-wide +0.060 us (-0.340, +0.500), BF16-tail -0.300 us (-0.660, +6.780), and BF16-wide -0.840 us (-1.160, -0.600). The Parent/Candidate device maxima were 78.380/15.320 us, 121.120/161.520 us, 75.240/90.520 us, and 142.560/13.560 us. These isolated maxima and wide p10/p90 spans make V002 medians unsuitable for a performance claim; `LOCAL_SCORE` remains `NONE`.

## Device Snapshot

Pre-Local d5 snapshots record HBM use, AICore, existing process load, and post-run state. The route did not change any existing process. The snapshots are under `results/snapshots/`.

## Evidence Files

`results/compile/` contains full_link and paired build logs, object/link size and SHA evidence, raw object disassembly, and assembly diff. `results/correctness/` and `results/local/` contain raw runner output and return codes. `results/artifacts/` contains the four Parent/V003 device artifacts. No probe executable or full build directory was copied into the repository.
