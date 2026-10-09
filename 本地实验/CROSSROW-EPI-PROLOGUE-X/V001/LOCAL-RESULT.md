# Local Result

ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V001
STAGE: LOCAL
PARENT: W3 CROSSROW-FULL-PIPELINE V012 baseline (`Parent.asc`)
CANDIDATE: W5-R01 V001 (`Candidate.asc`)
SHAPE: rows=16 width=6144 blocks=8 dtype=fp16
PATH: generic cached-row `Process()`; effective_blocks=8; min_rows_per_block=2; max_rows_per_block=2
RUNNER: existing `crossrow_v001_paired_runner`
DEVICE: 0
FREE_HBM_MB_PRE: 62259
FREE_HBM_MB_POST: 62259
WARMUPS: 5
PAIRED_SAMPLES: 21
TIMING_METHOD: LEGACY_TIMING_METHOD

## Numeric result

PARENT_MEDIAN_DEVICE_US: 17.780000
CANDIDATE_MEDIAN_DEVICE_US: 16.080000
MEDIAN_OF_PAIRED_DELTA_DEVICE_US: -0.580001
MEDIAN_OF_PAIRED_DELTA_PERCENT: -4.9743%
MEDIAN_DIFFERENCE_US: -1.700000 (candidate minus parent medians)
PARENT_P05_P95_US: 9.080000 / 28.160000
CANDIDATE_P05_P95_US: 8.280000 / 33.700000
PARENT_SPREAD90_US: 19.080000
CANDIDATE_SPREAD90_US: 25.420001
DIRECTION_COUNT: candidate faster 13/21; candidate slower 8/21

## Qualification boundary

LOCAL_SCORE: 16.080000 us (numeric measurement only)
LOCAL_DELTA: -0.580001 us; -4.9743% paired median (numeric measurement only)
LOCAL_VERDICT: NEEDS_ONE_MORE_LOCAL
CURRENT_LOCAL_BEST: NONE
LOCAL_BEST_AFTER_REVISION: NONE

The current runner uses `warmups=5,repeats=21`, which is `LEGACY_TIMING_METHOD` under the current protocol. This result is not `LOCAL_ACCEPTED` and does not advance `LOCAL_BEST`; a future 45-warmup/event-qualified run is required before promotion.

## Raw evidence and load context

Parent/Candidate raw device and wall samples are retained in `local-v001-r16-d6144.tsv`; runner summary/output is in `local-v001-r16-d6144.log`. Pre/post device context is in `local-load-pre.txt` and `local-load-post.txt`. Both snapshots report `HBM Capacity=65536 MB`, `HBM Usage Rate=5%`, `Aicore Usage Rate=0%`; `OTHER_PROCESS_PRESENT=YES` is recorded as context only and did not block measurement.

Parent device samples (us): `[36.120001,8.500000,21.140000,26.540000,22.500001,13.660000,23.480000,10.820000,11.660000,28.160000,11.480000,9.840000,20.600000,11.480000,11.040000,13.280000,17.780000,18.719999,19.020000,9.080000,19.619999]`

Candidate device samples (us): `[33.700000,22.000000,8.280000,19.320000,8.380000,33.960000,17.940000,11.080000,11.080000,25.540000,9.880000,17.659999,25.880000,16.080000,15.939999,15.520000,17.460000,11.300000,18.619999,7.220000,15.939999]`
