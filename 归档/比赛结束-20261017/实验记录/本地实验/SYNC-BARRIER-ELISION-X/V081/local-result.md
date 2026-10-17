# V081 Local Result

## Identity and gates

- Route: `SYNC-BARRIER-ELISION-X`; host `hwnput3`; device 3; SoC `910B3` / `dav-2201`.
- Direct Parent / Local Best: exact `R31B-V011`; V080 is not the Parent.
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `f62c2692d8303288387a33501fa1a54ca8a57b7fbcf29ac8483e027063de570e`.
- Compile: PASS, both targets, `/tmp/sync-v081-build.20261008`.
- Correctness: PASS, 9/9 cases, zero Parent/Candidate bit mismatches.

## Measurement

- Shape / dtype: `128x256 / bf16`.
- Method: 60 warmups; three complete runs of 31 interleaved Parent/Candidate pairs; device-event timing with wall-clock diagnostics; no outlier filtering.
- All raw device-event samples, wall timings, throughput, and pre/post NPU snapshots are retained in `local.log`.

| Run | Parent median us | Candidate median us | Paired median delta us | Parent throughput Gelem/s | Candidate throughput Gelem/s | Throughput delta | Local score |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 18.1400 | 18.6800 | +0.3000 | 1.806395 | 1.754176 | -0.052219 | -1.653804% |
| 2 | 14.4800 | 14.4800 | -0.3200 | 2.262983 | 2.262983 | +0.000000 | +2.209942% |
| 3 | 16.1200 | 17.0600 | +0.9600 | 2.032754 | 1.920750 | -0.112004 | -5.955334% |

- Recorded result uses the latest complete run: `LOCAL_SCORE = -5.955334%`; `LOCAL_DELTA = -5.955334%`.
- Independent run scores disagree in direction; verdict is `LOCAL_REJECTED_NOISY`, not a Local Best promotion.
- Device-event CV (Parent/Candidate): run 1 `0.43192/0.36585`; run 2 `0.54183/0.40950`; run 3 `0.45560/0.39237`.
- Paired-delta ranges: run 1 `-12.5400..+13.8000 us`; run 2 `-24.3400..+14.3200 us`; run 3 `-26.5000..+17.9000 us`.
- Device 3 HBM: `3428/65536 MB` pre-run and `3430/65536 MB` post-run, approximately 62 GB free; AICore `0%`; no process on NPU 3.
- Other-device Python/VLLM processes were present and untouched; host load was high (`72.79` to `77.07`). `LOAD_QUALITY = LOW`.
- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`.
- `CURRENT_LOCAL_BEST` remains exact `R31B-V011`; Online is forbidden.

## Evidence note

The copied runner wrote `V080` in internal `LOCAL_STATS` and `LOCAL_RESULT` labels even though the executed binary was `/tmp/sync-v081-build.20261008/sync_barrier_elision_correctness` and the V081 command header records the V081 candidate hash. The raw `local.log` is preserved unchanged; this labeling discrepancy is recorded here rather than silently rewriting raw evidence.

## Continuity timestamps

- `RULE_REFRESH_TIMESTAMP = 2026-10-08T20:27:35Z`.
- `EDIT_TIMESTAMP = 2026-10-08T20:27:57Z` (source mtime/evidence handoff).
- `COMPILE_PASS_TIMESTAMP = 2026-10-08T20:39:11Z` (formal evidence recheck).
- `CORRECTNESS_START_TIMESTAMP = 2026-10-08T20:39:11Z`.
- `CORRECTNESS_PASS_TIMESTAMP = 2026-10-08T20:39:15Z`.
- `LOCAL_START_TIMESTAMP = 2026-10-08T20:33:50Z`.
- `LOCAL_RESULT_TIMESTAMP = 2026-10-08T20:34:06Z`.
- The V081 local run preceded the formal evidence recheck; all timestamps and raw outputs are retained without rewriting.

