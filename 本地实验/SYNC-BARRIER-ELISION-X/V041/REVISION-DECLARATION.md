# SYNC-BARRIER-ELISION-X V041

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V041
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()` first-pass FP16 processing, remove only the `PIPE_V` barrier after `Add(xLocal, xLocal, residualLocal, valid)` and before `ToFloat(valueTile, xLocal, valid)`.
- SINGLE_CHANGE_BOUNDARY: Delete one vector-pipeline barrier from exact R31B-V011. No V001-V040 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V040 route-local actual diff patches contain no deletion at this generic first-pass FP16 in-place-Add-to-`ToFloat` point. V004/V008 delete analogous barriers in the distinct `ProcessNarrowMidOverlap` path; V012 deletes a generic second-pass widening-to-Add barrier; V040 deletes the following generic first-pass conversion-to-square-Mul barrier. Width 128 dispatches to generic `Process()` and executes the selected FP16 Add and barrier.
- COMPILE_TARGET: `sync_barrier_elision_v041`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x128, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `00f07f5e18a283ccf2b91b653c2d1573c989f9bfb4919a8edf36c0bb654e7bd6`
- COMPILE: PASS; target `sync_barrier_elision_v041`; configure RC=0, build RC=0; CANN `8.5.0.alpha002`, host `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T21:35:12Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs on FP16 8x128. Pooled `LOCAL_SCORE=+39.789186957%` geomean paired speedup. Per-run scores were `+67.630486464%` and `+16.571974480%`; paired-median diagnostics were `+14.177492%` and `+5.774273%`. Pooled Parent/Candidate device medians were `16.98/6.28 us`, CV `0.597960/0.622116`; raw ranges were `4.02-56.44/4.02-24.26 us`. The score is positive in both runs but varies by about 51 percentage points, device jitter is high, Parent has a 56.44 us outlier, and pooled wall median slightly favors Parent (`67.180/68.526 us`); all samples retained; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled median-derived throughput `0.060306/0.163057 G elements/s` (Parent/Candidate, diagnostic); pooled wall CV `0.263497/0.115697`, maximum wall latency `169.730/106.831 us`. Device 3 AICore remained 0% before/after both runs; HBM snapshots were `3426/3428 MB` for both. Other devices had active AICore users, including device 4 at 64-66%; host load averages after runs were `52.65/48.85/52.49` and `65.04/51.35/53.10`. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and raw logs under `logs/`.
