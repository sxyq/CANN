# V039 Result

## Outcome

- Direct parent: V026. V032-V038 remain independent probes and are not parents.
- Single change: in `ProcessFp32FullRowOutputPipelined`, apply the existing `invRms` scalar to an FP32 scratch copy of each gamma tile, then multiply the value tile by that scaled gamma. The cached gamma is not mutated; scale extent, dtype, reduction, output loop, dispatch, synchronization, and bias position are unchanged.
- Compile: PASS on `hwnput3`, Ascend910B3 / dav-2201, CANN 8.5.0.alpha002; `device` and `submission` targets passed.
- Correctness: Parent and Candidate passed FP32 `[128,8192]`, matched ratio 1.0; maximum absolute errors were `5.96046448e-06` and `5.7220459e-06`. Candidate dispatched through the intended branch and executed the source delta.
- Local: `MEASUREMENT_BLOCKED`. Descriptive score `90.2999979850`; pooled medians Parent `18.0600005 us`, Candidate `20.000001 us`; delta `+10.7419736782%` (Candidate slower).
- Per-block deltas were `+47.574229%`, `-18.771932%`, and `+54.824871%`; Candidate was slower in two of three blocks. Parent stability medians were `23.110001/19.56 us`; pooled CV was `0.319558` Parent and `0.238391` Candidate. All 96 samples per arm and both stability windows were retained, none excluded.
- Load: NPU 0 was healthy with 0% AICore and `3432/65536 MB` HBM before and after. Shared activity on other devices was visible. Host load averages were `65.17,60.58,56.52` before stability, `63.16,59.87,56.50` before paired runs, and `38.41,52.98,54.36` after. No other process was modified.
- Do not promote: `CURRENT_LOCAL_BEST=V026`. This single-shape Local score is not comparable to Official. Online was not run.

## Evidence

- Compile and correctness: `compile-local-20261007T2240Z.log`, `compile-result.json`, `correctness-local-20261007T2240Z.log`, `correctness-result.json`.
- Local harness, stability windows, interleaved raw samples, and load snapshots: `local-build-local-20261007T2240Z.log`, `local-parent-stability-20261007T2250Z.log`, `local-paired-interleaved-20261007T2250Z.log`, `local-load-before-20261007T2240Z.log`, `local-load-before-paired-20261007T2250Z.log`, `local-load-after-20261007T2250Z.log`.
- Source identity and one-factor diff: `source-meta.json`, `submission.sha256`, `diff.patch`.
