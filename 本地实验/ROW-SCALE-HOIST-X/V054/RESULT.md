# V054 Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`). Candidate SHA-256: `58844eee194a62ae7b0f051484ba9e7b0c069ce1cac1fdc6c63b67d9452530c0`.
- Change: Move the existing row scale in `ProcessSmallFp32ContiguousBatched` only for its wide branch (`rowWidth > 192`) from the value operand to a per-row gamma scratch in dead post-reduction `xLocal`; retain the narrow branch and other consumers unchanged.
- Compile: PASS for `device` and `submission`, exit 0, fresh build `/tmp/cann-row-scale-hoist-x-v054-compile.D3QpEO`.
- Correctness: Parent and Candidate PASS on device 0, FP32 `[128,256]`. Dispatch audit confirms 40 blocks, 3-4 rows/core, and Candidate delta execution. Both matched ratio `1.0`; max absolute error `7.15255737e-7`.
- Local: 128 raw samples per arm across four interleaved blocks; all nine invocations exited 0. Pooled medians: Parent `17.2699995 us`, Candidate `20.7300005 us`; descriptive speedup index `83.3092092786`, Candidate median delta `+20.0347486982%` slower. Pooled means: Parent `16.2718750234 us`, Candidate `22.5692187031 us`; mean delta `+38.7007869137%` slower.
- Paired block median deltas (positive is slower): `+27.669619%`, `+0.160165%`, `+9.862869%`, `+48.270992%`; Candidate was slower in all four blocks. Paired block geometric-mean speedup index: `83.2391369119`.
- Local verdict: `MEASUREMENT_BLOCKED`, descriptive only. The run used 20 warmups per process, below the current 45-warmup protocol; Candidate pooled CV was `0.637362` with a `137.979996 us` maximum. Do not promote; current Local Best remains V026. This score is not Official-comparable.
- Device 0 was released at `2026-10-08T12:43:32Z` after numeric capture. HBM was `9134/65536 MB`, with the pre-existing Python process left undisturbed. See the before/after snapshots and release receipt.
- Build/runtime setup failures were preserved in `runners-build.log`, `device-snapshot-attempt1-failure.txt`, and `device-snapshot-attempt2-failure.txt`; the exact ASC-path retry succeeded. No timing rerun was performed.

Raw samples and per-invocation details are retained in `local-*.log` and `local-invocation.txt`.
