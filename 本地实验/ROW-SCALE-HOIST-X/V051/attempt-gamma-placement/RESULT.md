# ROW-SCALE-HOIST-X V051 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`). Candidate SHA-256: `5aad6db8fb2be25d65cd39c42c6a5ce72b75fd6e485b4495905c0c1d46f25bba`.
- Change: in the narrow FP32 `ProcessSmallFp32ContiguousBatched` subcase, apply each row's existing `invRms` to a gamma scratch held in the dead-after-reduction `xBuf_` row before multiplying the retained value row. No other path changed.
- Compile: PASS for `device` and `submission`; exact current source was compiled at `/tmp/cann-row-scale-hoist-x-v051-canonical-compile.PtQs44`. The earlier V051 order-swap compile remains preserved but was not used for correctness or Local.
- Correctness: Parent and Candidate PASS on FP32 `[128,128]`, 40 blocks, 3-4 rows/core. Both matched ratio `1.0`; max absolute error `7.15255737e-07`. Candidate dispatch audit confirms `ProcessSmallFp32ContiguousBatched` and `candidate_source_delta_executed=true`.
- Local: four interleaved blocks, 128 samples/arm, 20 warmups/process. Pooled medians: Parent `19.6799995 us`, Candidate `19.21 us`; ratio-of-medians score `102.446639770953`, median delta `-2.388208902139%` (negative is faster). Pooled means: Parent `20.34640621875 us`, Candidate `23.784218578125 us`; mean delta `+16.896410709656%` (positive is slower). Candidate CV is `127.18951206%`, max latency `289.679993 us`. Paired block median deltas are `+10.23102860%`, `+17.39369865%`, `-38.95225093%`, `+1.63553164%`.
- Verdict: `MEASUREMENT_BLOCKED` because the median and mean disagree, Candidate has an extreme timing spike/high CV, and three of four paired block medians are slower. No samples were excluded. Local Best remains V026; no promotion or Official Score is claimed.
- Device 0 was explicitly released after numeric capture at `2026-10-08T05:38:02Z`. Pre-Local free HBM was `53,169 MB`; existing processes were left undisturbed.
- Online: NOT_RUN / NOT_AUTHORIZED.
- Full raw evidence, snapshots, build logs, hashes, and result JSON are adjacent to this file.
