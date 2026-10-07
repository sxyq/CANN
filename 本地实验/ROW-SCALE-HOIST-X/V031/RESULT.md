# V031 Result

## Outcome

- Parent: V026. The only kernel change widens the row-scale multiplication in `ProcessFp16FullRowOutputPipelined` to FP32 while preserving gamma-before-row-scale order.
- Compile: PASS on `hwnput3`, local CANN 8.5.T8.0.B060, DAV-2201.
- Correctness: PASS for Parent and Candidate on device 0, FP16 `[128,8192]`; matched ratio 1.0, max absolute error 0.00390625.
- Dispatch: the final runner reports 40 blocks and 3-4 rows per block; the selected branch is `ProcessFp16FullRowOutputPipelined`, and `candidate_source_delta_executed=true` on every Candidate process.
- Local: `NEEDS_ONE_MORE_LOCAL`, interleaved P-C-P-C-P-C, 20 warmups and 32 samples per process; all 96 samples per arm retained.
- Pooled medians: Parent `18.6900005 us`, Candidate `21.34 us`. `LOCAL_SCORE=87.5820079663`; `LOCAL_DELTA=+14.1787021354%` (positive means Candidate slower). Parent/Candidate paired-block medians were `[18.639999,11.94,19.959999]` and `[20.17,22.130001,21.52] us`; Candidate was slower in all three blocks.
- Estimated logical throughput: Parent `676.749474 GB/s`, Candidate `592.710778 GB/s`, using the runner's logical-byte model (`12648448` bytes per invocation). This is not measured HBM bandwidth.
- Quality is HIGH_JITTER_SHARED_DEVICE_AND_HOST_ACTIVITY. Pooled CV was `1.160992` Parent and `0.911196` Candidate; maxima were `180.360001` and `158.419998 us`. Device 0 snapshots were 35% AICore / 24% vector / 6% HBM bandwidth before, then 0% / 0% / 0% after; HBM remained 91% used. Host load averages were `37.83` before and `57.77` after, with a Python process at about 1621% CPU and VLLM workers active. Do not promote from this noisy window.
- The first Local attempt `[128,2048]` is retained separately as `INVALID_WRONG_DISPATCH`; it selected `ProcessNarrowMidOverlap`, not the changed branch, and its score is excluded here.
- `CURRENT_LOCAL_BEST=V026`. No Online action was run.

## Evidence

- Corrected Local raw samples and dispatch audit: `local-paired-final-20261007T061605Z.log`.
- Corrected correctness: `correctness-8192-20261007T060832Z.log` and `correctness-result.json`.
- Build logs: `compile-retry-20261007T054819Z.log`, `correctness-build-8192-20261007T060720Z.log`, and `local-build-final-20261007T061452Z.log`.
- Invalid wrong-dispatch record: `invalid-attempt-20261007T054220Z.json`; original raw log remains unchanged.
- Candidate SHA256: `32f8cc7803ad6e6e18aaf4429866ada26eead9f398d9f6ab1e17c3174fe78979`; Parent SHA256: `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
