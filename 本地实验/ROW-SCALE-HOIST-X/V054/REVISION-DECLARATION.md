# ROW-SCALE-HOIST-X V054

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Axis: FP32 row-scale placement/order in the row-scale-hoist consumer.
- Single change: in `ProcessSmallFp32ContiguousBatched`, retain the current scale placement for `rowWidth <= kFp32RepeatMaxWidth`; in the wide branch, scale the gamma operand by that row's `invRms` using the dead post-reduction `xLocal` row as scratch, then multiply the value row by the scaled gamma before adding bias. This moves the existing scale operation without adding/removing arithmetic or changing dispatch, reduction, output, or synchronization structure.
- Target case: FP32 `[128,256]`, wide branch (`rowWidth > 192`).
- Distinction: V051 exercised the narrow branch of this consumer; V054 covers the wide branch. V054 is a sibling of V026, not a continuation of V053.
- Edit timestamp: `2026-10-08T12:20:11Z`; V053 Local capture ended `2026-10-08T11:45:02Z` (elapsed `2109 s`, `1929 s` beyond the 180-second target).
- Compile: PASS, targets `device` and `submission`, exit 0; build directory `/tmp/cann-row-scale-hoist-x-v054-compile.D3QpEO`. Candidate and compile input SHA-256 `58844eee194a62ae7b0f051484ba9e7b0c069ce1cac1fdc6c63b67d9452530c0`.
- Correctness: Parent and Candidate PASS on device 0 for FP32 `[128,256]`; the Candidate source delta executed in the `ProcessSmallFp32ContiguousBatched` wide branch.
- Local: descriptive pooled-median index `83.3092092786`; Candidate median delta `+20.0347486982%` slower; Candidate slower in all four paired blocks. Verdict `MEASUREMENT_BLOCKED` due 20 warmups vs the current 45-warmup protocol and high Candidate variance (CV `0.637362`). Current Local Best remains V026. Device 0 released after capture.
- Required order: Candidate edit, immediate Compile; then Parent/Candidate Correctness and Local only after Compile PASS and a fresh device-0 snapshot. Preserve all raw evidence and device/load receipts.
