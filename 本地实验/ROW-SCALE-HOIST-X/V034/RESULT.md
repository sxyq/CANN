# V034 Result

## Outcome

- Direct parent: V026. V033 remains unchanged and is not the parent.
- Single change: in `ProcessFp16FullTileBatchedOutputPipelined`, swap the existing native-half row-scale and gamma Mul order after `FromFloat`; bias and all other behavior remain unchanged.
- Compile: PASS on local host `hwnput3`, Ascend910B3 / `dav-2201`, toolkit directory `8.5.0.alpha002`; `device` and `submission` targets built.
- Correctness: Parent and Candidate PASS on device 0 for FP16 `[128,4096]`; matched ratio 1.0 and max absolute error 0.00390625 for each.
- Local: numeric descriptive score `105.1524710831`; pooled Parent/Candidate medians `10.00/9.51 us`; Candidate `4.90%` faster. Paired blocks were mixed: Candidate faster in 2/3.
- Local verdict: `MEASUREMENT_BLOCKED`. Parent same-binary medians were `9.79` and `10.56 us`; pooled CVs were `2.0854` Parent and `1.9074` Candidate, with maxima `325.62/283.74 us`. All 96 raw samples per arm are retained and none excluded.
- Load: device 0 was 91% HBM occupied with shared VLLM residency; host load was high. No other process was modified.
- Do not promote: `CURRENT_LOCAL_BEST=V026`. This single-shape Local metric is not comparable to Official 45.16. Online was not run.

## Evidence

- Declaration: `REVISION-DECLARATION.md`.
- Compile: `compile-result.json`, `compile-local-20261007T113356Z.log`.
- Correctness: `correctness-result.json`, `correctness-local-20261007T1134Z.log`.
- Local runner build: `local-build-20261007T1137Z.log`.
- Parent same-binary stability: `local-parent-stability-20261007T1138Z.log`.
- Raw interleaved samples, dispatch audits, verification and load snapshots: `local-paired-20261007T1140Z.log`; summary: `local-result.json`.
- Source hashes and OFAT diff: `source-meta.json`, `submission.sha256`, `diff.patch`.
