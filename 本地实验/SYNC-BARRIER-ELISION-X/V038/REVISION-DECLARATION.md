# SYNC-BARRIER-ELISION-X V038

- ROUTE: SYNC-BARRIER-ELISION-X
- REVISION: V038
- DIRECT_PARENT: R31B-V011
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_HYPOTHESIS: In generic `Process()`, remove only the `SyncVToMTE3()` immediately before the FP16 output `Store(outputGm_, rowOffset + col, outputLocal, valid)`.
- SINGLE_CHANGE_BOUNDARY: Delete one V-to-MTE3 event handoff from exact R31B-V011. No V001-V037 candidate edits are inherited.
- DUPLICATION_AUDIT: V001-V037 route-local actual diff patches contain no deletion of this generic `Process()` FP16 output-store handoff. V027 deletes the distinct `ProcessNarrowMidOverlap` V-to-MTE3 call. Width 128 dispatches to generic `Process()` because the narrow/mid branch requires `rowWidth > 128`; the selected half-precision output branch executes this call unconditionally.
- COMPILE_TARGET: `sync_barrier_elision_v038`
- CORRECTNESS_CASES: FP16 rows=8, widths=128, 256, 1024, 2048, 4096; exact Parent/Candidate bitwise comparison.
- LOCAL_CASE: FP16 8x128, device 3, 60 warmups and two scored 31-pair interleaved runs using device events and wall-clock diagnostics; retain every sample without outlier filtering. One additional 31-pair run with a stale V037 runner label is preserved but excluded from the score.
- CURRENT_LOCAL_BEST: R31B-V011
- ONLINE: NO

- CANDIDATE_SOURCE_SHA256: `8053e410a98cb38ae23b144b3e6030b6074dd3cd61f56889eec01a243a10ac2c`
- COMPILE: PASS; target `sync_barrier_elision_v038`; configure RC=0, build RC=0; CANN `8.5.0.alpha002`, host `hwnput3`, Ascend910B3, device 3.
- CORRECTNESS: PASS; FP16 rows=8 widths 128, 256, 1024, 2048, and 4096 all had zero bit mismatches; runner RC=0 at `2026-10-07T21:10:26Z`. The first harness build attempt failed before kernel execution because `ASCEND_HOME_PATH` was not set; the environment was corrected and the same Candidate passed all cases.
- LOCAL: 62 scored raw interleaved Parent/Candidate device-event pairs on FP16 8x128. Pooled `LOCAL_SCORE=+6.870728825%` geomean paired speedup. Per-run scores were `+2.839869847%` and `+11.059579292%`; paired-median delta diagnostics were `+2.666664%` and `-10.236219%`. Pooled Parent/Candidate device medians were `7.32/7.07 us`, CV `0.854966/0.661595`; raw ranges were `4.06-69.30/3.94-27.98 us`. Jitter is high and the paired-median diagnostics reverse; all samples retained; `LOCAL_REJECTED_NOISY`.
- LOCAL_QUALITY: Pooled median-derived throughput `0.139891/0.144837 G elements/s` (Parent/Candidate, diagnostic); pooled wall medians `67.941/68.891 us`, wall CV `0.280082/0.092678`, maximum wall latency `179.972/93.531 us`. Device 3 AICore remained 0% before/after both scored runs; HBM snapshots were `3426/3428 MB` for both. Other devices had active AICore users, including device 4 at 62-66%; host load averages after the runs were `57.55/61.77/54.50` and `73.02/64.16/55.52`. This single-shape Local score is not comparable to Official 45.16.
- LOCAL_DIAGNOSTICS: An earlier 31-pair run is preserved at `logs/local-device3-20261007T211122Z-run1.log`; the copied runner printed `revision=V037`, so its samples are excluded from the V038 score. The runner label was corrected and rebuilt before the two scored runs. The first Correctness harness-build failure is also retained; it did not execute a kernel.
- CURRENT_LOCAL_BEST: `R31B-V011` (unchanged).
- ONLINE: NOT_SUBMITTED.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `source-meta.json`, `submission.sha256`, `diff.patch`, and all raw logs under `logs/`.
