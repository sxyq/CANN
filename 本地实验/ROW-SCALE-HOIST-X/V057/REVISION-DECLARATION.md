# ROW-SCALE-HOIST-X V057

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Current Local Best: V026. V056 is committed `MEASUREMENT_BLOCKED` and is not the parent.
- Focus axis: row-scale operand placement in the FP32 full-tile small-batched output consumer.
- Single change: in `ProcessSmallFp32FullTileBatched`, reuse the dead-after-reduction `xBuf_` row as scratch for `gammaLocal * invRmsValues[batchRow]`; multiply the retained FP32 `valueRow` by this scaled gamma instead of applying `invRms` to `valueRow`.
- Target: FP32 `[128,4096]`, `rowWidth == kTileElems`, expected dispatch `ProcessSmallFp32Batched` -> `ProcessSmallFp32FullTileBatched`.
- Distinctness: V040 tests scale/gamma order on the value operand in this same consumer; V050 tests gamma-scratch placement in the separate non-full-tile `ProcessSmallFp32Batched` consumer at width 3072; V039 tests a separate full-row consumer. No prior declaration applies gamma-scratch scaling in this full-tile consumer.
- Scratch lifetime: all input rows have been reduced and all `invRmsValues` materialized before the output loop. `xBuf_` and `residualBuf_` are not consumed during that epilogue; use one `xBuf_` row at a time, without allocation or synchronization changes.
- Unchanged: exact V026 parent base; FP32 precision and one scale plus one value/gamma multiply; inverse-RMS formula/value; cached gamma and bias contents; bias placement; reduction, row/batch dispatch, buffers, events, synchronization, stores, and all other consumers.
- Correctness target: FP32 mixed tolerance `atol=2^-16`, `rtol=2^-10`, matched ratio `>=0.99`, max absolute error `<=1e-2`.
- Local protocol: 45 warmups, 32 timed samples per invocation, 32-sample Parent stability, and four interleaved P/C blocks; preserve all raw samples/load snapshots and report numeric results independently of qualification.
- Candidate edit: complete; Candidate and compile input SHA-256 `8404337fd0c9ec8056876d06790e310b04872bde14d7d1b544c857e06c146146`.
- Compile: PASS for `device` and `submission`; see `compile-result.json` and `compile/` logs.
- Runtime device: device 2 was excluded by MODE-DISPATCH V093. Device 3 was selected under the current campaign rule from snapshot `device-snapshot-selection-20261008T160846Z.log`; a fresh pre-Correctness snapshot at `2026-10-08T16:13:22Z` confirmed 5% HBM used (about 62.3 GB free), 0% AICore/AIVector, and no process listed.
- Online/shared-record changes: not authorized and not run.

## Result

- Compile: PASS; `device` and `submission` targets.
- Correctness: Parent PASS; Candidate FAIL on FP32 `[128,4096]`; see `correctness-result.json` and retained raw logs.
- Local: NOT_RUN because Candidate correctness failed.
- Device 3 was released after the post-correctness snapshot; see `device-release.log`.
- Current Local Best remains V026.
