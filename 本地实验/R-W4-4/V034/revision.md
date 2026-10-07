# R-W4-4 V034

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V034`
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 64`.
- SOURCE_SHA256: `dcb2eca042fee260f6b7418623f32cd0c0f3c3a69cfbdcf3b778cb50ea99827b`
- COMPILE: PASS for `device` and `submission`.
- PROBE_BUILD: PASS for exact Parent and Candidate probes.
- CORRECTNESS: Parent and Candidate PASS (`rc=0`, `bad=0`) on `128x64`, `128x72`, and `128x80` FP32. C15 was not run.
- LOCAL: 6 interleaved pairs per shape; device-event timing, warmup 45, 31 samples x 2 blocks, batch 64. All 36 invocations returned `rc=0`, `bad=0`.
- PARTIAL_CORRECTNESS: YES; known route Parent C15 failure remains untested and is not classified as a Candidate regression.
- LOCAL_SCORE: `0.967353314406x` partial route-local speedup geomean; `LOCAL_DELTA=-3.26466856%`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: NO.
- LOAD_QUALITY / MEASUREMENT_QUALITY: NOISY; `NEEDS_ONE_MORE_LOCAL=YES`.
- CURRENT_LOCAL_BEST: NONE. V029/V030/V031/V032/V033/V034 are not promoted.
- NEXT_PARENT: exact R31B-V011 sibling baseline, not a noisy route Candidate.
- EVIDENCE: `本地实验/R-W4-4/V034/partial-ranking/`.
