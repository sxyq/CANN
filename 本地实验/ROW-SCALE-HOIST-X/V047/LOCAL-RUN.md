# V047 Local Run

- Runner build: `/tmp/cann-row-scale-hoist-x-v047-local.1fH882`; targets `v047_local_parent` and `v047_local_candidate`.
- Runtime environment: `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest`, `ASCEND_OPP_PATH=/usr/local/Ascend/ascend-toolkit/latest/opp`, `LD_LIBRARY_PATH=/usr/local/Ascend/ascend-toolkit/latest/lib64:/usr/local/Ascend/driver/lib64/driver`.
- Device: 0, exclusively assigned to this Route from correctness through numeric capture.
- Every process performs 20 warmups and captures 32 ACL device-event samples, then reports correctness and all raw latencies.
- Sequential block order: Parent 1, Candidate 1, Candidate 2, Parent 2, Parent 3, Candidate 3, Parent 4, Candidate 4. No runners overlapped.
- Raw outputs: `local-parent-block1.log`, `local-candidate-block1.log`, `local-candidate-block2.log`, `local-parent-block2.log`, `local-parent-block3.log`, `local-candidate-block3.log`, `local-parent-block4.log`, `local-candidate-block4.log`.
- All eight runner exits were 0; all eight logs report correctness PASS and the expected FP16 `[128,2050]` `ProcessNarrowMidOverlap_FP16` resident-parameter dispatch.
- Pre-Local device/process snapshot: `local-load-before.log`. Post-capture device release snapshot: `device-release.log`.
