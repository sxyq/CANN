# ROW-SCALE-HOIST-X V054 Rule Refresh

- Refreshed: 2026-10-08.
- Worktree/branch: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4`, `exp/row-scale-hoist-x-w4`.
- Live checkpoint before V054: `0c8e6b7bc6c92f2b9e18ccb97dea18672574ce8f` (V053 result commit).
- Rule refresh was completed in this owner context before V054 preparation. This turn reread the project `cann-mainline` skill; the unchanged required-rule inventory and SHA-256 values are retained in V053's `RULE_REFRESH_RECEIPT.md`.
- Current Local Best and direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- V053 is closed as `MEASUREMENT_BLOCKED`; its raw samples and commit are preserved. No Local Best promotion.
- Authorized V054 point: one row-scale operand-placement change in `ProcessSmallFp32ContiguousBatched`, wide branch `rowWidth > kFp32RepeatMaxWidth`; target FP32 `[128,256]`. The narrow branch and all other consumers remain unchanged.
- V053 Local result capture ended `2026-10-08T11:45:02Z`; V054 Candidate edit timestamp was `2026-10-08T12:20:11Z`: elapsed `2109 s` (`35m09s`), which is `1929 s` (`32m09s`) beyond the 180-second target.
- The V054 source and compile input were staged from exact V026 before the Candidate edit. Device 0 remains reserved but unused until Compile PASS; fresh HBM/process snapshot is required before correctness.
- Scope: existing worktree/branch only; no shared-record edits, Online action, push, or changes to prior evidence/scratch.
