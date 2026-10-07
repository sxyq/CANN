# V038 Result

## Outcome

- Direct parent: V026. V032-V037 remain preserved, independent probes and are not parents.
- Single change: in `ProcessFp32FullRowOutputPipelined`, move each per-tile FP32 row-scale `Muls` from before gamma multiplication to after gamma multiplication and before bias. Scale extent, dtype, reduction, loop, dispatch, buffers, events, and other paths are unchanged.
- Compile: PASS on `hwnput3`, Ascend910B3 / dav-2201, CANN 8.5.0.alpha002; `device` and `submission` targets passed.
- Correctness: Parent and Candidate passed for FP32 `[128,8192]`, matched ratio 1.0; maximum absolute errors were `5.96046448e-06` and `5.7220459e-06`. Both dispatched through the intended branch and the Candidate source delta executed.
- Local: `MEASUREMENT_BLOCKED`. The descriptive score was `101.3937282230`, with pooled medians Parent `20.370000 us` and Candidate `20.090000 us`; the corresponding delta is `-1.3745704467%` (Candidate faster).
- Repeatability does not support promotion: Parent stability windows had medians `20.5100005` and `25.6100005 us`. Paired block deltas were `+55.587391%`, `-17.094775%`, and `+3.183426%` (positive means slower); Candidate was slower in two of three blocks. Pooled CV was `0.484932` Parent and `0.480684` Candidate. All 96 raw samples per arm were retained; none were excluded.
- Load: NPU 0 was healthy with 0% AICore and `3431/65536 MB` HBM before and after. Shared host/device activity on other devices was observed. Host load average changed from `63.59,49.82,49.75` to `50.28,49.59,49.73`. No other process was modified.
- An initial Parent stability runner could not load `libmsprofiler.so` in a fresh shell. The retry after sourcing the existing Ascend environment passed; both the failure and successful retry remain recorded. This setup failure is not treated as a route failure.
- Do not promote: `CURRENT_LOCAL_BEST=V026`. This local single-shape score is not comparable to Official. Online was not run.

## Evidence

- Compile and correctness: `compile-local-20261007T2222Z.log`, `compile-result.json`, `correctness-local-20261007T2222Z.log`, `correctness-result.json`.
- Local runner build and raw evidence: `local-build-local-20261007T2222Z.log`, `local-parent-stability-20261007T2222Z.log`, `local-parent-stability-retry-env-20261007T2230Z.log`, `local-paired-interleaved-20261007T2230Z.log`.
- Device and host load snapshots: `local-load-before-paired-20261007T2230Z.log`, `local-load-after-20261007T2230Z.log`.
- Source identity and OFAT diff: `source-meta.json`, `submission.sha256`, `diff.patch`.
