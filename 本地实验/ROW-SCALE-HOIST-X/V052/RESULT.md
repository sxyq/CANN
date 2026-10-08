# ROW-SCALE-HOIST-X V052 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`). Candidate source SHA-256: `69eeccade066c7865f28d997932f573131d395df3354abec34b6f1f5f411b237`.
- Change: in the BF16 narrow subcase of `ProcessSmallLowPrecisionContiguousBatched`, scale the FP32 gamma scratch per row using the existing `invRms`, then multiply the retained FP32 value row by that scratch. The wide subcase, FP16 arm, and all other paths remain V026-identical.
- Compile: PASS for `device` and `submission`; see `compile-result.json` and `compile.log`.
- Correctness: Parent and Candidate PASS on BF16 `[128,128]`, 40 blocks, 3-4 rows/core; matched ratio `1.0`, maximum absolute error `0.00390625`. Both dispatch audits select `ProcessSmallLowPrecisionContiguousBatched`; Candidate confirms `candidate_source_delta_executed=true`.
- Local method: ACL device-event timing; 20 warmups and 32 samples per runner process; 32-sample Parent stability window plus four interleaved P/C blocks (128 comparison samples per arm). All primary samples were retained; no exclusions.
- Pooled medians: Parent `17.659999 us`, Candidate `21.0200005 us`; ratio-of-medians descriptive score `84.015216840742`; median delta `+19.0260571362%` (Candidate slower).
- Pooled means: Parent `18.7804687422 us`, Candidate `22.3259374609 us`; mean delta `+18.8784889633%` (Candidate slower).
- Paired block median deltas (Candidate slower positive): `+25.837313%`, `+3.531148%`, `+27.620214%`, `+9.402600%`; Candidate was slower in all four blocks.
- Quality: `HIGH_VARIANCE_WITH_CANDIDATE_SPIKE`; pooled CV Parent `49.10%`, Candidate `64.02%`; Candidate maximum `165.080002 us`. Median, mean, and all paired block medians agree on a slowdown, so verdict is `LOCAL_REJECTED` rather than promotion. Raw logs are retained unchanged.
- Current Local Best: V026. No Official score is claimed; Online was not run.
- Device 0 was explicitly released after numeric result capture at `2026-10-08T07:46:01Z`; post-capture HBM was `4814/65536 MB`, AICore `0%`, and no V052 runner remained. The pre-existing Python PID `251901` was left undisturbed; see `device-release.log`.

Evidence includes the source and one-factor diff, source hashes, compile/correctness/Local build logs, Parent/Candidate correctness outputs, every Local raw block, snapshots, and invocation records.
