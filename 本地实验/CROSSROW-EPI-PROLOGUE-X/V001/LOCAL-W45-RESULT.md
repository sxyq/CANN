# Event-Qualified Local Result

ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V001
STAGE: LOCAL_QUALIFICATION
PARENT: W3 CROSSROW-FULL-PIPELINE V012 baseline (`Parent.asc`)
CANDIDATE: W5-R01 V001 (`Candidate.asc`)
SHAPE: rows=16 width=6144 blocks=8 dtype=fp16
PATH: generic cached-row `Process()`; effective_blocks=8; two rows per effective block
RUNNER: existing `crossrow_v001_paired_runner`
DEVICE: 0
WARMUPS: 45
PAIRED_SAMPLES: 21
TIMING_METHOD: EVENT_QUALIFIED_SAME_BINARY_PC_CP
EVENT_SUPPORT: PASS; runner brackets each timed call with `aclrtRecordEvent`, synchronizes the stop event, and reads `aclrtEventElapsedTime`
HARNESS_LIMITATION: NONE

## Numeric result

FREE_HBM_MB_PRE: 62259
FREE_HBM_MB_POST: 62259
PARENT_MEDIAN_DEVICE_US: 13.020000
CANDIDATE_MEDIAN_DEVICE_US: 21.520000
MEDIAN_OF_PAIRED_DELTA_DEVICE_US: 3.540000
MEDIAN_OF_PAIRED_DELTA_PERCENT: +15.3779%
PARENT_P05_P95_US: 8.380000 / 27.580000
CANDIDATE_P05_P95_US: 9.180000 / 28.400000
PARENT_SPREAD90_US: 19.200001
CANDIDATE_SPREAD90_US: 19.220000
DIRECTION_COUNT: candidate faster 9/21; candidate slower 12/21

LOCAL_SCORE: 21.520000 us (numeric measurement only)
LOCAL_DELTA: +3.540000 us; +15.3779% paired median
LOCAL_VERDICT: LOCAL_NO_PROMOTION
CURRENT_LOCAL_BEST: NONE

This event-qualified run does not promote `LOCAL_BEST`; the Candidate is slower than the Parent on the selected probe. The prior `LEGACY_TIMING_METHOD` raw data remains unchanged in `local-v001-r16-d6144.tsv`.

## Raw evidence and load context

Raw device and wall samples are retained in `local-v001-r16-d6144-w45.tsv`; runner output is in `local-v001-r16-d6144-w45.log`. Pre/post device context is in `local-w45-load-pre.txt` and `local-w45-load-post.txt`; the pre-run free-HBM probe is in `local-w45-free-hbm-pre.txt`. Both snapshots report `HBM Capacity=65536 MB`, `HBM Usage Rate=5%`, `Aicore Usage Rate=0%`, and `OTHER_PROCESS_PRESENT=YES` as measurement context only.

Parent device samples (us): `[7.600000,8.460000,12.500000,10.180000,27.580000,26.540000,9.540000,10.740000,24.259999,9.980000,24.739999,10.840000,27.319999,23.280000,27.300000,13.020000,23.019999,8.380000,28.160000,10.020000,24.900001]`

Candidate device samples (us): `[25.900001,28.400000,26.660001,21.520000,11.960000,21.100000,27.380001,10.840000,10.620000,24.560001,10.260000,26.219999,14.820000,27.620001,16.600000,23.879999,26.559999,29.200001,9.180000,9.920000,9.060000]`
