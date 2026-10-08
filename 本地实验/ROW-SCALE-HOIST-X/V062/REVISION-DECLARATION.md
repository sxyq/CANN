# ROW-SCALE-HOIST-X V062

- Direct parent and `CURRENT_LOCAL_BEST`: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- V061 is preserved as correctness-failed evidence and is not the parent.
- Single OFAT change: in the BF16 narrow `ProcessSmallLowPrecisionContiguousBatched` branch (`width <= kFp32RepeatMaxWidth`), move the existing row `invRms` multiply from before gamma to `value * gamma`, then `* invRms`, then `+ bias`.
- Candidate source SHA-256: `1dd6fb4b2a29d752b0d8a190cc5c8e5310a7725a8aee05d708bf0a0f43955996`.
- Compile: PASS for `device` and `submission`.
- Correctness: Parent and Candidate PASS on BF16 `[128,128]`, matched ratio `1.0`, max absolute error `0.00390625`; target dispatch and Candidate delta execution were confirmed.
- Local: all nine retry invocations completed with device-event timing, 32 samples each, and raw logs retained. Numeric descriptive median speed index `137.3737344029`; latency delta `-27.2058807787%`; throughput delta `+37.3737344029%`.
- Local quality: `MEASUREMENT_BLOCKED`. Parent stability MAD/median is `0.5574172`, Parent stability CV is `0.7161834`, pooled Parent CV is `0.8985318`, pooled Candidate CV is `1.0056765`, and paired direction is `3/4`. Device 0 had other Python processes and AICore load; none were disturbed. The result is descriptive only.
- The first Local attempt failed before runner startup because `/usr/local/Ascend/ascend-toolkit/latest/set_env.sh` does not exist and `libmsprofiler.so` was unresolved. The direct environment fix used `/usr/local/Ascend/ascend-toolkit/set_env.sh`; the failed logs and fixed-environment evidence are retained.
- `CURRENT_LOCAL_BEST` remains V026. No Official-comparable score, Online candidate, shared-record edit, or Online submission was made.
- The runner used 20 warmups per invocation and this result is not promoted; all raw samples remain available for audit.
