# V032 Result

## Outcome

- Direct parent: V026. V031 is not the parent.
- Single change: in `ProcessFp16FullRowOutputPipelined`, move row-scale from two native-half 4096-element output-tile Muls to one FP32 8192-element Muls on `valueLocal` before output conversion.
- Compile: PASS on `hwnput3`, CANN 8.5.T8.0.B060, Ascend910B3 / dav-2201.
- Correctness: Parent and Candidate PASS on device 0, FP16 `[128,8192]`; matched ratio 1.0; max absolute errors 0.00390625 and 0.001953125.
- Local: `NEEDS_ONE_MORE_LOCAL`, P-C-P-C-P-C, 20 warmups and 32 samples per process; all 96 samples per arm retained.
- Pooled medians: Parent `12.01 us`, Candidate `12.56 us`. `LOCAL_SCORE=95.6210267214`; `LOCAL_DELTA=+4.5795087427%` (Candidate slower). Candidate was slower in 2/3 paired blocks.
- Quality: `HIGH_JITTER_SHARED_DEVICE_AND_HOST_ACTIVITY`. Pooled CV was 2.1350 Parent and 1.2658 Candidate; Parent maximum was 366.38 us and Candidate maximum was 134.98 us. Device 0 used about 60.2 GB of 65.5 GB HBM with a resident VLLM worker; host load increased from `25.98,31.50,32.75` to `35.79,33.03,33.20`. No samples were excluded.
- Do not promote: `CURRENT_LOCAL_BEST=V026`. This single-shape Local score is not comparable to Official 45.16. Online was not run.

## Evidence

- Compile: `compile-correctness-fix-20261007T1020Z.log`, `compile-result.json`.
- Correctness: `correctness-final-20261007T1024Z.log`, `correctness-result.json`.
- Local raw samples, dispatch audit, verification and load snapshots: `local-paired-final-20261007T1035Z.log`; summary: `local-result.json`.
- Candidate SHA256 and Parent SHA256 are recorded in `submission.sha256`, `source-meta.json` and the machine-readable results.
- OFAT source delta: `diff.patch`.
- Earlier failed and misconfigured attempts remain in their original logs; they are not used as final gate results.
