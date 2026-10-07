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

## Resume Gate and Additional Local Window (2026-10-07)

- Correctness resume: after configuring both `ASC_DIR` and `CMAKE_PREFIX_PATH`, the fresh runner build completed and Parent/Candidate both passed FP16 `[128,8192]`; matched ratio was 1.0, with max absolute errors `0.00390625` and `0.001953125`. Evidence: `correctness-resume-20261007T163413Z.log` and `correctness-result-resume-20261007T163413Z.json`. The earlier CMake configuration failure remains preserved.
- Additional Local window: P-C-P-C-P-C, 20 warmups and 32 samples per process; all 96 samples per arm retained, all six output checks passed, and dispatch audit selected `ProcessFp16FullRowOutputPipelined` for each arm.
- Pooled medians: Parent `18.389999 us`, Candidate `20.360001 us`; `LOCAL_SCORE=90.3241556815`; `LOCAL_DELTA=+10.7123551230%` (Candidate slower). Candidate was slower in 2/3 blocks.
- Quality: `HIGH_HOST_LOAD_AND_BLOCK_DRIFT`. Parent/Candidate pooled CV were `0.332482`/`0.268191`; block medians were Parent `[13.960001,21.740000,18.630001] us`, Candidate `[23.200001,19.779999,18.780001] us`. Host load averages changed from `45.74,51.91,51.95` to `38.82,49.65,51.19`; several other NPU devices had large resident workloads. No samples were excluded.
- The runner's reported logical-I/O byte count is twice the FP16 count. Corrected FP16 logical traffic is `6,324,224` bytes; derived throughput is `343.895 GB/s` Parent and `310.620 GB/s` Candidate. The raw runner output is unmodified and retains its reported constant.
- The log contains repeated P1/C1 block markers due to a logging label bug. The six `RUNNER_RESULT` records are in the executed P-C-P-C-P-C order and were grouped by that order; raw data is preserved in `local-resume-20261007T164426Z.log` and summarized in `local-result-resume-20261007T164426Z.json`.
- Do not promote: `CURRENT_LOCAL_BEST=V026`. This single-shape Local metric is not comparable to Official `45.16`; Online was not run.
