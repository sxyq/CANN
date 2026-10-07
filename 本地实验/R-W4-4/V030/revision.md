# R-W4-4 V030

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V030`
- DIRECT_PARENT: V029, exact source SHA256 `9e9a4a17e0079b77e8f9b41f26e57653e70128081da27c5ffff0b064375bb978`
- SOURCE_SHA256: `ff5543d4b4748953ad9f43880f9f3d21d562771440f6e2cb514ab11ce099f168`
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `128 -> 192`
- COMPILE: PASS; `device` and `submission` targets, observed 2026-10-07T12:23:30Z-12:23:38Z.
- PROBE_BUILD: PASS; exact V029 Parent and V030 Candidate binaries, observed 2026-10-07T12:24:28Z-12:25:12Z.
- CORRECTNESS: PASS on jointly-tested FP32 shapes `128x136`, `128x192`, `128x200`; both sides `rc=0`, `bad=0`.
- LOCAL: six interleaved Parent/Candidate pairs per shape, warmup 45, 31 samples, batch 64; all 36 paired invocations `rc=0`, `bad=0`.
- PARTIAL_CORRECTNESS: YES; known exact V027 Parent C15 failure remains and was not rerun.
- CURRENT_LOCAL_BEST: NONE; V029 is inconclusive and is not promoted.
- LOCAL_SCORE: `0.993147013644x` partial route-local speedup geomean; `LOCAL_DELTA=-0.685298636%`; calculated from all six raw device-event pairs on each of `128x136`, `128x192`, and `128x200` (pair speedup = Parent median / Candidate median; mean six pair speedups per shape; equal-weight geomean across shapes).
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: NO.
- LOAD_QUALITY: NOISY; MEASUREMENT_QUALITY: NOISY; NEEDS_ONE_MORE_LOCAL: YES.
- RESULT: `NEEDS_ONE_MORE_LOCAL`; direction is mixed and every shape's paired deltas remain within its measured Parent/Candidate MAD envelopes.
- NEXT_PARENT_SELECTION: No verified Candidate Local Best exists. Use the exact `R31B-V011` sibling baseline, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; do not promote V029 or V030.
- EVIDENCE: `本地实验/R-W4-4/V030/partial-ranking/`.
