# UB-LIFETIME-SAFE V001 simple local-score audit

```text
ROUTE=UB-LIFETIME-SAFE-CHAMPION-X
REVISION=V001
HYPOTHESIS=UBX-H1-FP16-RETAINED-Y-INPLACE-OUTPUT
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989
TARGET=FP16 rows=1 D=32768 device=7
RUNNER=reused R31B paired runner with UB wrapper; 45 warmups; 21 P/C pairs
TIMING=device-event primary
```

The existing V001 Build is PASS.  The full correctness battery has one
failure (`fp32-wide-16384`, 16337 mismatches), but the legal timing target
`fp16-wide-32768` passed for both Parent and Candidate on every run.  The
support runner only binds the preserved Parent and Candidate artifacts; no
Candidate source or reference arithmetic was changed.

| Run | Parent avg (us) | Candidate avg (us) | Delta (us) | Delta (%) |
|---:|---:|---:|---:|---:|
| 1 | 15.306 | 15.539 | +0.233 | +1.522279 |
| 2 | 17.842 | 18.342 | +0.500 | +2.802825 |
| 3 | 63.190 | 42.343 | -20.847 | -32.990979 |

The run-level values are arithmetic means of the 21 Parent and 21 Candidate
device-event samples in each run.

```text
PARENT_AVG_US=32.112667
CANDIDATE_AVG_US=25.408000
LOCAL_DELTA_US=-6.704667
LOCAL_DELTA_PERCENT=-20.878573
MEASUREMENT_QUALITY=POOR
LOCAL_VERDICT=LOCAL_SCORE_RECORDED_ON_VALID_TARGET_NOT_ACCEPTED
GLOBAL_CORRECTNESS=CORRECTNESS_FAILED_ON_FP32_WIDE_16384
```

The third run has very large external-load tails and the three run directions
are not stable.  The score is retained because the audit explicitly requires
three completed runs on a legal target shape; it does not make V001 eligible
for Online or `LOCAL_ACCEPTED`.

Evidence:

- `support/local-score-configure-20261003.log`
- `support/local-score-build-20261003.log`
- `support/local-score-build-retry-20261003.log`
- `local-score-20261003/run1/`
- `local-score-20261003/run2/`
- `local-score-20261003/run3/`
- `source-meta.json`, `support/correctness/correctness-evidence.json`
