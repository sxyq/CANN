# ROW-SCALE-HOIST-X V062 finite retest

- Source identity: `4042019061242d3ec3a6c47cd6057b5e1ef2e9c1:本地实验/ROW-SCALE-HOIST-X/V062/submission.asc`
- Candidate SHA-256: `1dd6fb4b2a29d752b0d8a190cc5c8e5310a7725a8aee05d708bf0a0f43955996`
- Direct Parent: V026; Parent SHA-256: `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`
- Compile: `PASS`; existing `device`/`submission` targets and existing Local targets `v062_local_parent`/`v062_local_candidate` built in fresh directories.
- Correctness: `PASS` for both Parent and Candidate on BF16 `[128,128]`; matched ratio `1.0`; max absolute error `0.00390625`; actual dispatch `ProcessSmallLowPrecisionContiguousBatched`; Candidate source delta executed.
- Local: fresh device-0 run, 20 warmups and 32 device-event samples per invocation; one Parent stability invocation plus four interleaved Parent/Candidate blocks; all 9 invocations exited 0; 128 pooled samples per arm.
- Formula: `score = 100 * pooled Parent median device_us / pooled Candidate median device_us`; positive Candidate delta means slower.
- Numeric result: Parent pooled median `19.7300005 us`, mean `25.1692187969 us`; Candidate pooled median `20.8500005 us`, mean `29.5020313203 us`; descriptive score `94.6282974909`; Candidate delta `+5.6766344228%` slower.
- Paired block medians: `21.550001/19.389999`, `18.48/18.82`, `21.42/21.23`, `16.780001/27.6 us`; Candidate faster in `2/4` blocks.
- Quality: `MEASUREMENT_BLOCKED` because pooled CV is Parent `0.8762475931`, Candidate `1.3302106960`, Candidate raw maximum is `339.080017 us`, and paired direction is mixed. Existing device processes were left untouched.
- Current Local Best remains `V026`; this is descriptive and not Official-comparable. `EXPORTABLE=YES` as an experimental route-local package; `ONLINE=NOT_RUN`.
- Fresh raw/load/release evidence is under `local/`; the older V062 Local logs under `本地实验/ROW-SCALE-HOIST-X/V062/local/` are preserved and were not used for this score.
