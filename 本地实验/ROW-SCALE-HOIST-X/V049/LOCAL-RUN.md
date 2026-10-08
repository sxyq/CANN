# V049 Local Run

- Runner build: `/tmp/cann-row-scale-hoist-x-v049-local.KmE4yC`; targets `v049_local_parent` and `v049_local_candidate`.
- Runtime environment: `LD_LIBRARY_PATH=/usr/local/Ascend/ascend-toolkit/latest/lib64:/usr/local/Ascend/driver/lib64/driver`.
- Device: 0, exclusively assigned to this Route through numeric result capture.
- Each process performs 20 warmups and captures 32 ACL device-event samples; all eight processes ran serially.
- Block order: Parent 1, Candidate 1, Candidate 2, Parent 2, Parent 3, Candidate 3, Parent 4, Candidate 4.
- All eight exits were 0 and every log reports correctness PASS for BF16 `[40,3072]`, 40 vector cores, one row/core, `ProcessNarrowMidOverlap`, `resident_params=false`. Candidate logs report `candidate_source_delta_executed=true`.
- Raw samples are retained in `local-parent-block1.log`, `local-candidate-block1.log`, `local-candidate-block2.log`, `local-parent-block2.log`, `local-parent-block3.log`, `local-candidate-block3.log`, `local-parent-block4.log`, and `local-candidate-block4.log`.
- Pre-Local snapshot: `local-load-before.log`; numeric aggregate: `local-result.json`.
- Device 0 was explicitly released after numeric capture at `2026-10-08T03:49:06Z`; final resource/process snapshot: `device-release.log`.
