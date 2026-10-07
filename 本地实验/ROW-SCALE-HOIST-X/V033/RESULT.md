# V033 Result

## Outcome

- Direct parent: V026. V032 remains unchanged and is not the parent.
- Single change: in `ProcessFp16FullRowOutputPipelined`, swap the existing native-half row-scale and gamma Mul order after `FromFloat`; bias and all other behavior remain unchanged.
- Compile: PASS on local host `hwnput3`, Ascend910B3 / `dav-2201`, toolkit directory `8.5.0.alpha002`; `device` and `submission` targets built.
- Correctness: Parent and Candidate PASS on device 0 for FP16 `[128,8192]`; matched ratio 1.0 and max absolute error 0.00390625 for each.
- Local: numeric descriptive score `94.9924127466`; pooled Parent/Candidate medians `12.52/13.18 us`; Candidate `5.2715654952%` slower. All three paired block medians favored Parent.
- Local verdict: `MEASUREMENT_BLOCKED`. Parent same-binary medians drifted from `13.01` to `23.04 us`; pooled CVs were `1.2237` Parent and `1.7972` Candidate, with maxima `188.76/294.28 us`. All 96 raw samples per arm are retained and none excluded.
- Load: device 0 was 91% HBM occupied with shared VLLM residency; Parent qualification also observed 40% AICore use and 35% HBM bandwidth. Host load was high. No other process was modified.
- Do not promote: `CURRENT_LOCAL_BEST=V026`. The single-shape Local metric is not comparable to Official 45.16. Online was not run.

## Evidence

- Declaration and rule refresh: `REVISION-DECLARATION.md`, `RULE_REFRESH_RECEIPT.md`.
- Compile: `compile-result.json`, `compile-local-20261007T1056Z.log`.
- Correctness: `correctness-result.json`, `correctness-local-20261007T1101Z.log`.
- Parent same-binary stability: `local-parent-stability-20261007T1105Z.log`.
- Raw interleaved samples, dispatch audits, verification and load snapshots: `local-paired-20261007T1107Z.log`; summary: `local-result.json`.
- Source hashes and OFAT diff: `source-meta.json`, `submission.sha256`, `diff.patch`.
