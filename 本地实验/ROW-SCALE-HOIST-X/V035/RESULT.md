# V035 Result

## Outcome

- Direct parent: V026. V032, V033, and V034 remain unchanged and are not parents.
- Single change: in the FP16 branch of `ProcessNarrowMidOverlap`, move the native-half row-scale `Muls` before the native-half gamma `Mul`; conversion and bias placement are unchanged.
- Compile: PASS on local host `hwnput3`, Ascend910B3 / `dav-2201`; `device` and `submission` targets built.
- Correctness: Parent and Candidate PASS on device 0 for FP16 `[128,3072]`; both matched ratio 1.0 and max absolute error `0.00390625`. Dispatch audit selected `ProcessNarrowMidOverlap_FP16` with 40 blocks and 3–4 rows/core.
- Local: descriptive single-shape score `102.4054982818`; pooled Parent/Candidate medians `17.88/17.46 us`; Candidate `2.348993%` faster. Paired blocks were mixed: Candidate faster in 2/3.
- Local verdict: `MEASUREMENT_BLOCKED`. Parent same-binary windows had CVs `1.496591/1.946616` and maxima `118.12/211.00 us`; pooled CVs were `1.451395` Parent and `1.416522` Candidate, with maxima `239.92/241.42 us`. All 96 raw samples per arm are retained; none excluded.
- Load: HBM was 91% occupied before and after; AICore/vector usage rose `21/11%` to `33/21%`; host load average rose from `51.93` to `66.94` (1-minute). Shared device/host activity was observed; no other process was modified.
- Do not promote: `CURRENT_LOCAL_BEST=V026`. This local score is not comparable to Official. Online was not run.

## Evidence

- Scope receipt and declaration: `RULE_REFRESH_RECEIPT.md`, `REVISION-DECLARATION.md`.
- Compile: `compile-result.json`, `compile-local-20261007T121751Z.log`.
- Correctness: `correctness-result.json`, `correctness-local-20261007T1219Z.log`.
- Local runner build: `local-build-20261007T1223Z.log`.
- Parent same-binary stability: `local-parent-stability-20261007T1225Z.log`.
- Raw interleaved samples and load snapshots: `local-paired-20261007T1227Z.log`; summary: `local-result.json`.
- Source hashes and OFAT diff: `source-meta.json`, `submission.sha256`, `diff.patch`.
