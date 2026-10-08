# V080 Local Result

## Identity and gates

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V080`
- Host: `hwnput3`; device 3; SoC `910B3` / `dav-2201`.
- Direct Parent / Local Best: exact `R31B-V011`; V069 and V079 are not Parents.
- Parent SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate SHA256: `acff22e50cc4aa2ddea833e6ef8da6625800740ac71cc77841e63cb11c6d6581`
- Compile: PASS, `sync_barrier_elision_v080` and correctness runner, `/tmp/sync-v080-build.20261008`.
- Correctness: PASS, 9/9 cases, zero Parent/Candidate bit mismatches.

## Measurement

- Shape / dtype: `128x256 / bf16`.
- Method: 60 warmups; three independent runs of 31 interleaved Parent/Candidate pairs; device-event timing with wall-clock diagnostics; no outlier filtering.
- All raw device-event samples, wall timings, throughput and pre/post NPU snapshots are retained in `local.log`.

| Run | Parent median us | Candidate median us | Paired median delta us | Parent throughput Gelem/s | Candidate throughput Gelem/s | Throughput delta | Local score |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 7.7600 | 16.0400 | +0.4000 | 4.222680 | 2.042893 | -2.179787 | -5.154642% |
| 2 | 13.7800 | 8.0400 | -0.6200 | 2.377939 | 4.075622 | +1.697683 | +4.499272% |
| 3 | 8.8800 | 14.4400 | -0.2200 | 3.690090 | 2.269252 | -1.420838 | +2.477475% |

Primary recorded score is the latest complete run: `LOCAL_SCORE = +2.477475%`; `LOCAL_DELTA = +2.477475%`. The three run scores disagree in direction, so this is not a reliable improvement.

- Device-event CV (Parent/Candidate): run 1 `0.45027/0.39390`; run 2 `0.44497/0.53600`; run 3 `0.43145/0.48557`.
- Paired delta ranges: run 1 `-12.3600..+14.6400 us`; run 2 `-16.4800..+15.1200 us`; run 3 `-12.6600..+17.2600 us`.
- Device 3 HBM: `3428/65536 MB` pre-run and `3431/65536 MB` post-run, approximately 62 GB free; AICore `0%` in both snapshots; no running process on NPU 3.
- Other-device Python/VLLM processes were present and untouched; host load was high (`62.92` to `69.83`).
- `LOCAL_SCORE_TYPE = SINGLE_SHAPE_DEVICE_EVENT_LOCAL`
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL = NO`
- `LOAD_QUALITY = LOW`; `REPEATABILITY = FAIL / noisy direction across independent runs`.
- Verdict: `LOCAL_REJECTED_NOISY`; `CURRENT_LOCAL_BEST` remains exact `R31B-V011`.

## Continuity timestamps

- `RULE_REFRESH_TIMESTAMP = 2026-10-08T20:16:00Z` (recorded before edit)
- `EDIT_TIMESTAMP = 2026-10-08T20:16:37Z`
- `COMPILE_PASS_TIMESTAMP = 2026-10-08T20:17:19Z` (compile log capture)
- `CORRECTNESS_START_TIMESTAMP = 2026-10-08T20:19:25Z` (runtime-path retry; initial loader failure retained)
- `CORRECTNESS_PASS_TIMESTAMP = 2026-10-08T20:19:29Z`
- `LOCAL_START_TIMESTAMP = 2026-10-08T20:19:53Z`
- `LOCAL_RESULT_TIMESTAMP = 2026-10-08T20:20:08Z`
- `COMPILE_PASS_TO_CORRECTNESS_START_SECONDS = 126`
- `CORRECTNESS_PASS_TO_LOCAL_START_SECONDS = 24`
- `LOCAL_RESULT_TO_NEXT_EDIT_SECONDS = recorded when V081 edit begins`
- `ONLINE = FORBIDDEN`
