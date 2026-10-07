# V036 Result

## Outcome

- Direct parent: V026. V032, V033, V034, and V035 remain unchanged and are not parents.
- Single change: in the FP16 full-row output path, materialize the existing `half(invRms)` scalar once per row before the two-tile output loop and reuse it for both existing post-gamma scale operations. The vector arithmetic, precision, order, and conversion boundary are unchanged.
- Compile: initial attempt failed with `rc=2` because the declaration landed in the BF16 sibling; after moving that declaration into the intended FP16 function, both `device` and `submission` targets passed. Both logs are retained.
- Correctness: Parent and Candidate passed on device 0 for FP16 `[128,8192]`; both matched ratio 1.0 and max absolute error `0.00390625`.
- Local: descriptive single-shape score `95.2763391592`; pooled Parent/Candidate medians `20.170001/21.170000 us`; Candidate `4.957853%` slower. Corrected FP16 logical traffic is `6,324,224` bytes per invocation (X + residual + output + gamma + bias), implying descriptive effective logical bandwidths of `313.5460/298.7352 GB/s` (Parent/Candidate), not physical HBM bandwidth. The runner emitted the FP32 byte count `12,648,448`; raw logs are unchanged. All three paired blocks were slower.
- Local verdict: `MEASUREMENT_BLOCKED`. Parent stability windows had CVs `1.669162/1.017382` and maxima `283.100006/198.180008 us`; pooled CVs were `1.252571` Parent and `1.325533` Candidate, with maxima `225.879990/305.839996 us`. All 96 raw samples per arm are retained; none excluded.
- Load: HBM stayed at 91%; AICore/AIVector usage was `33/28%` before and `32/27%` after; HBM bandwidth usage rose `20%` to `29%`; host load average rose from `55.31` to `61.52` (1-minute). Shared activity was observed; no other process was modified.
- Do not promote: `CURRENT_LOCAL_BEST=V026`. This local score is not comparable to Official. Online was not run.

## Evidence

- Scope receipt and declaration: `RULE_REFRESH_RECEIPT.md`, `REVISION-DECLARATION.md`.
- Source hashes and one-factor diff: `source-meta.json`, `submission.sha256`, `diff.patch`.
- Compile failure and successful retry: `compile-local-20261007T1252Z.log`, `compile-retry-local-20261007T1255Z.log`, `compile-result.json`.
- Correctness build and execution: `correctness-build-local-20261007T1258Z.log`, `correctness-run-local-20261007T1300Z.log`, `correctness-result.json`.
- Local runner build: `local-build-local-20261007T1302Z.log`.
- Parent same-binary stability: `local-parent-stability-1-20261007T1303Z.log`, `local-parent-stability-2-20261007T1304Z.log`.
- Raw interleaved samples and load snapshots: `local-paired-20261007T1305Z.log`, `local-load-before-20261007T1303Z.log`, `local-load-after-20261007T1306Z.log`; summary: `local-result.json`.
- Post-commit audit correction: the FP16 runner's logical-byte metadata was twice the FP16 tensor traffic; only derived throughput metadata was corrected. Latencies, score, verdict, and raw logs are unchanged.
