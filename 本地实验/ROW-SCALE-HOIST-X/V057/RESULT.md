# ROW-SCALE-HOIST-X V057 Result

- Direct parent: V026, source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Candidate change: scale an FP32 scratch copy of `gammaLocal` by each row's existing `invRms` in `ProcessSmallFp32FullTileBatched`, then multiply the retained value row by that scratch.
- Candidate source and Compile input SHA-256: `8404337fd0c9ec8056876d06790e310b04872bde14d7d1b544c857e06c146146`.
- Compile: PASS for `device` and `submission`; see `compile-result.json` and `compile/` logs.
- Correctness: Parent PASS (`matched_ratio=1.0`, `max_abs_error=4.05311584e-06`); Candidate FAIL (`matched_ratio=0.975345612`, `max_abs_error=2.85903168`). Both ran FP32 `[128,4096]` on device 3 through the intended `ProcessSmallFp32FullTileBatched` branch; Candidate source delta execution was confirmed.
- Verdict: `CORRECTNESS_FAILED`. This is a Candidate-specific failure against a passing Parent; it is not a Parent-gate failure.
- Local: NOT_RUN because the Candidate correctness gate failed. No Local numeric score exists.
- Device: device 2 was excluded for MODE-DISPATCH V093. Device 3 had 5% HBM use and no process before Correctness; it was released after the post-capture snapshot at `2026-10-08T16:15:09Z`. See the snapshot and release receipts.
- Current Local Best: V026. No Online or shared-record action was performed.

Raw correctness logs, runner identities, and pre/post device snapshots are retained under this revision directory.
