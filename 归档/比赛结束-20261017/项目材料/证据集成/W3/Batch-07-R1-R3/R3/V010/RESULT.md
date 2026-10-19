# V010 Result

ROUTE=TINY-MINIMAL-KERNEL-CHAMPION-X
REVISION=V010
DIRECT_PARENT=R31B-V011
FOCUS_AXIS=tiny-fixed-overhead-loop-control
SINGLE_CHANGE=Precompute the final tile start once per Process call and use `col == lastTileStart` for the cached-row FP32 output drain condition.

## Source

PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=cf6d958312b401ca40da0b89cb5fb6cd638e92c77aea522a6d22b48d34f2ea03

## Compile

COMPILE=PASS
COMPILE_LOG=本地实验/TINY-MINIMAL-KERNEL-CHAMPION-X/V010/support/compile-20261005T173705+0800.log
REMOTE_ROOT=/home/data4t2/lelinfeng/cann/server_runs/TINY-MINIMAL-KERNEL-CHAMPION-X/V010/20261005T093705Z-20515
RUNNER=tiny_runner_v010
RUNNER_SHA256=15b600c874ac9d25dccc53c51220c2795474643740448a71661da22ab4ab5043

The first compile invocation stopped before toolchain execution because the copied runner filename was still `tiny_runner_v009.asc`. That log is retained; the input filename was corrected and the next compile passed without changing Candidate source.

## Correctness

CORRECTNESS=PASS
CASES=9/9
COMPARE=bitwise
MISMATCHES=0
CORRECTNESS_LOG=本地实验/TINY-MINIMAL-KERNEL-CHAMPION-X/V010/support/run-correctness-20261005T173839+0800.log

## Parent Qualification

PARENT_QUALIFICATION=NOT_QUALIFIED
PARENT_QUALIFICATION_LOG=本地实验/TINY-MINIMAL-KERNEL-CHAMPION-X/V010/support/run-parent-qualify-20261005T173944+0800.log
QUALIFICATION_NOTE=All three FP32 cases exceeded the timing-qualification MAD or block-drift limits under the observed load. The complete raw samples are retained, and the result did not prevent paired Local.

## Local

DEVICE_ID=7
SAMPLE_COUNT=21 pairs per case
METHOD=paired parent/candidate device-event timing
LOCAL_LOG=本地实验/TINY-MINIMAL-KERNEL-CHAMPION-X/V010/support/run-paired-20261005T174039+0800.log
FREE_HBM_CONTEXT=The run admitted with FREE_HBM >= 100 MB; a post-run query reported 30147 MB available on device 7.

| Case | Parent median us | Candidate median us | Delta us | Delta percent |
| --- | ---: | ---: | ---: | ---: |
| target-r1-d64-fp32 | 25.440 | 29.440 | +1.200 | +15.7233% |
| control-r2-d64-fp32 | 21.700 | 14.560 | -3.720 | -32.9032% |
| control-r1-d129-fp32 | 15.880 | 25.520 | +6.420 | +60.7053% |

LOCAL_SCORE=+15.7233% target delta; a positive value is slower than the parent in this report.
CURRENT_LOCAL_BEST=R31B-V011 Official 45.16/15/15
INTERPRETATION=The target regressed, one control improved, and D129 regressed. This is not a stable Local improvement and does not replace the current Local Best.
ONLINE=NOT_RUN
PUSH=NO

All raw Compile, Correctness, parent-qualification, load-snapshot, and paired timing logs remain in this V010 directory.
