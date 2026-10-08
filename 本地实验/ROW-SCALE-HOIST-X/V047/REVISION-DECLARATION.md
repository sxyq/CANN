# ROW-SCALE-HOIST-X V047

- Direct parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Focus axis: row-scale placement/order, FP16 resident multi-row `ProcessNarrowMidOverlap`
- Single change: for resident FP16 parameters only, multiply cached `gammaLocal` into a native-half scratch by `half(invRms)`, then multiply the half output by that scratch. Keep the one-row operation order, bias add, event schedule, other dtypes, and all other paths unchanged.
- Rationale: test whether moving the existing native-half inverse-RMS scale onto the reused resident gamma operand improves the multi-row output path without changing parameter loads or synchronization.
- Target: FP16 `[128,2050]`, 40 vector cores/blocks, 3-4 rows/core. Width 2050 is not aligned to the low-precision contiguous path and satisfies the narrow-mid width range.
- Expected dispatch: `ProcessNarrowMidOverlap`, `residentParams == true`.
- Compile: PASS for `device` and `submission`; see `compile-result.json`.
- Correctness: Parent and Candidate PASS with the intended resident multi-row dispatch; see `correctness-result.json`.
- Local: descriptive score `113.2207396622`, Candidate `11.6769592759%` faster by pooled median; `MEASUREMENT_BLOCKED` because both arms had high variance and Parent block medians were unstable. Local Best remains V026; see `local-result.json` and `RESULT.md`.
- Current Local Best before this revision: V026
- Online: NOT_RUN / NOT_AUTHORIZED
