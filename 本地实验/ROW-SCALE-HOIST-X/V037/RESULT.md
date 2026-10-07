# V037 Result

## Outcome

- Direct parent: V026. V032-V036 remain unchanged and are not parents.
- Single change: in `ProcessFp32FullRowOutputPipelined`, replace the two per-tile FP32 row-scale `Muls` with one full-row `Muls` after `invRms` and before the output loop. Scale-before-gamma order, dtype, reduction, dispatch, buffers, events, and other paths are unchanged.
- Compile: first attempt stopped before CMake because `nounset` made `set_env.sh` fail on unset `LD_LIBRARY_PATH`; retry passed both `device` and `submission` targets.
- Correctness: Parent and Candidate passed on device 0 for FP32 `[128,8192]`; both matched ratio 1.0 with max absolute error `5.96046448e-06`. Both dispatch audits selected the target branch; Candidate source delta executed.
- Local: descriptive single-shape score `82.4468065172`; pooled Parent/Candidate medians `17.050000/20.680001 us`; Candidate `21.290326%` slower. All three paired blocks and all 96 samples per arm are retained; none excluded.
- Paired-block deltas: `-2.473892%`, `+10.930931%`, and `+42.898230%` (positive is slower). Candidate was slower in two of three blocks.
- Jitter and quality: `MEASUREMENT_BLOCKED`. Parent stability-window medians were `20.500000/16.0999995 us`, with CV `1.409756/0.271196`; pooled CV was `0.340326` Parent and `0.368496` Candidate. Parent window 1 reached `252.420013 us`; Candidate pooled maximum was `59.799999 us`.
- Throughput: runner-declared `12,648,448` FP32 logical bytes per invocation imply descriptive effective logical bandwidths of `741.8445/611.6271 GB/s` (Parent/Candidate), not physical HBM bandwidth.
- Load: host load average was `59.75, 63.56, 62.50` before and `58.55, 60.02, 61.24` after. NPU 0 AICore usage was `0%/1%`, HBM use `7585/7589 MB`; shared Python/VLLM processes and activity on other NPUs were present. No other process was modified.
- Do not promote: `CURRENT_LOCAL_BEST=V026`. This Local score is not comparable to Official. Online was not run.

## Evidence

- Scope and one-factor declaration: `RULE_REFRESH_RECEIPT.md`, `REVISION-DECLARATION.md`.
- Source hashes and one-factor diff: `source-meta.json`, `submission.sha256`, `diff.patch`.
- Compile attempt and successful retry: `compile-local-20261007T134536Z.log`, `compile-retry-local-20261007T134601Z.log`, `compile-result.json`.
- Correctness setup failure and successful build/run: `correctness-local-20261007T134717Z.log`, `correctness-retry-local-20261007T134859Z.log`, `correctness-result.json`.
- Local runner build: `local-build-local-20261007T135105Z.log`.
- Parent stability: `local-parent-stability-1-20261007T135309Z.log`, `local-parent-stability-2-20261007T135338Z.log`.
- Interleaved raw samples and load snapshots: `local-paired-20261007T135428Z.log`, `local-load-before-20261007T135228Z.log`, `local-load-after-20261007T135506Z.log`; summary: `local-result.json`.
