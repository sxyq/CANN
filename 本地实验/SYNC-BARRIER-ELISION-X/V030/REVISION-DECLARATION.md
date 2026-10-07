# SYNC-BARRIER-ELISION-X V030

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V030
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In `ProcessNarrowMidOverlap`, remove only the second `SyncSToV()` event handoff, immediately after extracting scalar `invRms` and before the output `Muls` consumes it.
- SINGLE_CHANGE_BOUNDARY: Delete one S-to-V event-handoff call from exact R31B-V011. No V001-V029 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V029 actual route diff patches contain one `SyncSToV()` deletion, V028's first handoff after `meanSquare` and before `Duplicate`; this revision deletes only the later handoff after `invRms` extraction. V029 deletes a distinct V-to-S handoff.
- COMPILE_TARGET: `sync_barrier_elision_v030`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x2048, device 3, 60 warmups and two 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering if Correctness passes.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `3679a312dc90985545c8b482de942858e157f40a292c995a454eb01abe7f5b5c`
- COMPILE: PASS; target `sync_barrier_elision_v030`; Configure RC=0, build RC=0; CANN `8.5.0.alpha002`, `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T18:26:33Z`.
- LOCAL: 62 raw interleaved Parent/Candidate device-event pairs across two 31-pair runs. Pooled `LOCAL_SCORE=-9.831006337%` geomean paired device-event speedup. Per-run scores were `-5.681988840%` and `-13.797509954%`; runner median-delta diagnostics were `-1.234568%` and `-15.384615%`. Pooled Parent/Candidate device medians were `14.39/14.44 us`, CV `0.568332/0.539287`; maxima were `28.56/30.78 us`. Both runs were negative; event jitter remains high. All 62 samples retained; `LOCAL_REJECTED_NOISY`, no promotion.
- LOCAL_QUALITY: Pooled median-derived throughput `1.138568/1.134626 G elements/s` (Parent/Candidate, diagnostic). Pooled wall medians were `73.415/73.291 us`, wall CV `0.253203/0.112233`; maximum wall latency `171.971/89.819 us`. Device 3 AICore remained 0% before and after both runs; HBM snapshots were 3426/3428 MB and 3425/3428 MB. This single-shape Local score is not comparable to Official 45.16.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and all raw logs under `logs/`.
