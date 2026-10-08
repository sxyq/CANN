# ROW-SCALE-HOIST-X V062 Result

## Revision

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Candidate: `1dd6fb4b2a29d752b0d8a190cc5c8e5310a7725a8aee05d708bf0a0f43955996`
- Shape/dtype: BF16 `[128,128]`, device 0
- Dispatch: `ProcessSmallLowPrecisionContiguousBatched`, narrow branch
- Single change: gamma -> row `invRms` scale -> bias, only for `width <= kFp32RepeatMaxWidth`

## Gates

| Stage | Result |
|---|---|
| Compile | PASS (`device`, `submission`) |
| Correctness Parent | PASS, matched `1.0`, max error `0.00390625` |
| Correctness Candidate | PASS, matched `1.0`, max error `0.00390625` |
| Local runner | PASS after direct environment fix |

## Local measurement

The fixed run used 20 warmups and 32 device-event samples per invocation: one Parent stability invocation followed by four interleaved Parent/Candidate blocks. All 288 timing samples are retained in the retry logs.

| Arm | Median (us) | Mean (us) | Throughput (elements/s) | Samples |
|---|---:|---:|---:|---:|
| Parent | 23.1199995 | 31.4057811 | 708650534.36 | 128 |
| Candidate | 16.8300000 | 21.7917189 | 973499702.91 | 128 |

- Descriptive median speed index: `137.3737344029`
- Latency delta `(candidate - parent) / parent`: `-27.2058807787%`
- Throughput delta `(candidate - parent) / parent`: `+37.3737344029%`
- Parent stability: median `16.599998 us`, CV `0.7161834`, MAD/median `0.5574172`
- Pooled Parent/Candidate CV: `0.8985318` / `1.0056765`
- Paired direction: Candidate faster in `3/4` blocks

| Block | Parent median (us) | Candidate median (us) | Candidate latency delta |
|---:|---:|---:|---:|
| 1 | 19.230001 | 17.760000 | -7.6443106% |
| 2 | 24.850000 | 9.960001 | -59.9195131% |
| 3 | 41.660000 | 13.760000 | -66.9707153% |
| 4 | 20.100000 | 22.190001 | +10.3980149% |

## Verdict

`MEASUREMENT_BLOCKED` — the numeric result is descriptive only. The Parent stability and pooled timing distributions are too noisy for promotion; the concurrent Python processes/AICore load were recorded and left untouched. `CURRENT_LOCAL_BEST` remains V026. `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE_FORBIDDEN=YES`.

The first attempt's loader failure (`libmsprofiler.so`, exit 127) is retained. The retry used `/usr/local/Ascend/ascend-toolkit/set_env.sh` and all nine invocations passed.
