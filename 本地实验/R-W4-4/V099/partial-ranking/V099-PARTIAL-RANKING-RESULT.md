# MODE-DISPATCH-CUTOFF-X V099 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011` (SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`). Candidate SHA256 `e5904e89a91b4dfe37b54f413585d9bae79cc5322425e98ed6fa2f1b07f4ba3f`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 996`; this is the only Candidate-source change. Selected FP32 shapes: `128x992`, `128x996`, `128x1000`.
- `COMPILE=PASS`. Parent and Candidate Correctness both passed all three selected cases (`rc=0,bad=0`). Probe-launch environment failures are retained separately; those attempts did not reach the kernel and are not correctness failures.
- Exact-Parent C15 FP32 `1x32768` remains excluded for the known Parent correctness failure. It was not rerun and is not a Candidate regression.
- Interleaved device-event Local on device 2: six P/C pairs per shape, 45 warmups, 31 samples per block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 event samples are retained (372 per side per shape). No samples were excluded.
- Formula: each pair speedup is `median(Parent raw device_us) / median(Candidate raw device_us)` over each invocation's 62 samples; shape score is the arithmetic mean of six pair speedups; route score is the equal-weight geomean of the three shape scores.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x992 | 1.006744, 1.018510, 1.028574, 0.971482, 0.964229, 0.999048 | 0.998097687770x / -0.190231% | 3/6 / 5/6 | 7.602660 / 7.638750 | 3.789% / 4.918% | 16.701523 / 16.622615 |
| 128x996 | 0.923849, 1.056065, 1.039628, 0.951919, 1.038326, 1.037242 | 1.007838097070x / +0.783810% | 4/6 / 2/6 | 7.926250 / 7.822345 | 48.450% / 5.802% | 16.084277 / 16.297926 |
| 128x1000 | 1.019421, 0.526717, 0.916869, 0.978167, 1.073100, 0.998557 | 0.918805245400x / -8.119475% | 2/6 / 3/6 | 7.617660 / 7.688435 | 5.274% / 27.373% | 16.803060 / 16.648382 |
| Equal-shape geomean | - | **0.974082547284x / -2.591745272%** | **9/18 / 10/18** | - | - | - |

## Quality and Decision

- `LOCAL_SCORE=0.974082547284x`; `LOCAL_DELTA=-2.591745272%` (partial route-local score only; computed from full-precision raw TSV medians).
- `LOAD_QUALITY=PROCESS_PRESENT`; `MEASUREMENT_QUALITY=NOISY`. Across 36 per-pair device snapshots, free HBM was 59,097-59,101 MB, HBM usage rate was 9%, AICore was 0-11%, and AIVector was 0-5%. Existing PID 2055832 (`python`, 3,066 MB) appeared before and after and was untouched.
- The numeric result retains every sample. Noise is material: Parent pooled CV reached 48.450% at width 996 (max 77.9522 us); Candidate pooled CV reached 27.373% at width 1000 (pair 02 median 14.0894 us, max 15.7725 us). Only 9/18 pair medians favored Candidate and 10/18 were within combined MAD.
- `LOCAL_VERDICT=LOCAL_REJECTED`; `CURRENT_LOCAL_BEST=exact R31B-V011`; V099 is not promoted. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- Device 2 release was recorded at `2026-10-08T19:32:44.404127879Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T19:39:35.190201922Z`; `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V099_COMMIT`. No V100 edit has started.
- `NON_EXECUTION_GAP=412.664080858 seconds` from Local capture end (`2026-10-08T19:32:42.526121258Z`) to numeric result derivation.

## Timestamps and Evidence

- Compile: `2026-10-08T19:15:58.321062137Z` to `2026-10-08T19:16:51.902820626Z`; targets `device`, `submission`, `clx_ref_parent_probe`, and `clx_ref_candidate_probe` built successfully. See `v099-compile-gate-receipt.txt`; the incomplete `v099-compile.log` is preserved as captured.
- Successful Parent/Candidate Correctness: `2026-10-08T19:21:36.288280597Z` to `2026-10-08T19:22:16.798020777Z`.
- Local: `2026-10-08T19:27:26.674352595Z` to `2026-10-08T19:32:42.526121258Z`; device release at `2026-10-08T19:32:44.404127879Z`.
- All raw samples are in `local-pair*-parent|candidate-r128-w*-fp32-raw.tsv`; per-invocation statistics, commands, return codes, stdout/stderr, and pre/post device snapshots remain alongside them.
