# SYNC-BARRIER-ELISION-X V043

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V043
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()` FP16 output with cached parameters, remove only the `PIPE_V` barrier between `Mul(outputLocal, outputLocal, gammaLocal[col], valid)` and the bias `Add`.
- SINGLE_CHANGE_BOUNDARY: Delete one vector-pipeline barrier from the exact R31B-V011 parent. No prior candidate changes are inherited.
- DUPLICATION_AUDIT: V001-V042 route-local diff patches contain no deletion at this cached-parameter FP16 branch. V015 removes the barrier in the distinct uncached `gammaLocal` branch. For 16x127 on 8 blocks, `localRows=2`; width 127 is not aligned to 16, so it bypasses the contiguous-batched path, and it is not greater than the narrow/mid minimum 128. It reaches generic `Process()` with `cacheRow=true` and `cacheParams=true`.
- COMPILE_TARGET: `sync_barrier_elision_v043`
- CORRECTNESS_CASES: FP16 8x128, 8x256, 8x1024, 8x2048, 8x4096, and 16x127; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 16x127, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; all samples retained without outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `83f8aa5ee2fa00138fa8ed14c5b36645276c35a59571b2fd0296ed44f12ca807`
- COMPILE: PASS; configure and target build RC=0 on `hwnput3`, CANN `8.5.0.alpha002`, Ascend910B3 / `dav-2201`.
- CORRECTNESS: PASS; all six cases had zero bit mismatches; runner RC=0 at `2026-10-07T21:58:24Z`.
- LOCAL: 62 raw Parent/Candidate device-event pairs. Pooled `LOCAL_SCORE=-2.397100419%`; run geomean scores were `-6.710105982%` and `+2.115305275%`. Scores reverse direction; pooled device-event CV is `0.378346/0.391658` (Parent/Candidate), while pooled device medians are nearly equal at `16.80/16.37 us`. Retain all samples and mark `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled wall medians `71.3015/71.3270 us`; median-derived throughput `0.120952381/0.124129505 G elements/s` (Parent/Candidate, diagnostic). Device 3 AICore remained 0% before/after both runs; HBM was `3425/3428 MB` and `3427/3428 MB`. Other devices reached 64% AICore; host load averages fell from `70.46/58.97/55.47` to `63.45/57.85/55.14` in run 1, then to `57.47/56.75/54.82` in run 2. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
