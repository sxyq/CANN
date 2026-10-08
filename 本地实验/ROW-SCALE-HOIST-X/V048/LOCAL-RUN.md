# V048 Local Run

- Runner build: `/tmp/cann-row-scale-hoist-x-v048-local.i6jbTU`; targets `v048_local_parent` and `v048_local_candidate`.
- Runtime environment: `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest`, `ASCEND_OPP_PATH=/usr/local/Ascend/ascend-toolkit/latest/opp`, `LD_LIBRARY_PATH=/usr/local/Ascend/ascend-toolkit/latest/lib64:/usr/local/Ascend/driver/lib64/driver`.
- Device: 0, exclusively assigned to this Route from correctness through numeric capture.
- Every process performs 20 warmups and captures 32 ACL device-event samples, then reports correctness and all raw latencies.
- Sequential block order: Parent 1, Candidate 1, Candidate 2, Parent 2, Parent 3, Candidate 3, Parent 4, Candidate 4. No runners overlapped.
- All eight runner exits were 0; each reported correctness PASS and the expected FP16 `[40,3072]` `ProcessNarrowMidOverlap_FP16` one-row dispatch. Candidate logs report `candidate_source_delta_executed=true`.
- Raw outputs: `local-parent-block1.log`, `local-candidate-block1.log`, `local-candidate-block2.log`, `local-parent-block2.log`, `local-parent-block3.log`, `local-candidate-block3.log`, `local-parent-block4.log`, `local-candidate-block4.log`.
- Pre-Local device/process snapshot: `local-load-before.log`. Numeric result is recorded in `local-result.json`; post-capture release snapshot is `device-release.log` (`2026-10-08T02:46:17Z`).
- Candidate pooled CV `1.3817`, mean `20.7097 us`, maximum sample `296.380005 us`; classification is `MEASUREMENT_BLOCKED`. No sample was omitted.
