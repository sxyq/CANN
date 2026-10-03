# HBH-09 V002 — System measurement blocker closure

```text
UTC=2026-10-03T08:35Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
OFFICIAL_ANCHOR=R31B V011 / 45.16
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
VALID_LOCAL_SCORE=NO
HBH09_V002_LOCAL_SCORE=UNAVAILABLE
SYSTEM_MEASUREMENT_BLOCKER=YES
MAIN2_PERFORMANCE_EXPLORATION=PAUSED_PENDING_PLANNING_OR_USER
ONLINE_READY=NO
NO_NEXT_PERFORMANCE_REVISION=YES
```

## Evidence basis

The exact unchanged V002 Candidate was built and correctness-tested before
measurement. Primary FP32/BF16 combinations were attempted across d0, d1, d3,
d4, d5, and d7; d6 was unavailable because another Main's lease remained
active. Multiple independent temporal windows were run with live HBM and lease
admission checks. AICore, VLLM, and resident processes were recorded and never
touched.

The measurement outcomes were:

```text
Window A: d0 FP32 rows40 and d3 BF16 rows80 qualified; paired directions/noise did not support a score.
Window A: d5 BF16 rows40 qualified; paired result stayed inside mixed noise.
Window B: d4 FP32 rows80 retried after loader-only support recovery; paired Parent jitter invalidated a score.
Window C: d5 BF16 rows40 qualification blocked; no P/C.
Window D: d4 BF16 rows80 qualified; paired deltas -14.346%..+26.991%, mixed and jittered.
Window E: d7 BF16 rows80 NEEDS_VALIDATION; no P/C.
Window F: d5 BF16 rows40 qualification blocked; no P/C.
Window G: d4 BF16 rows80 qualified; 8 paired deltas +0.816,-7.649,+7.514,+1.492,-17.744,-6.952,+23.288,+11.446%.
Window H: d3 BF16 rows80 qualified; 8 paired deltas +4.850,+0.915,+0.096,-5.362,+0.622,+6.042,+8.279,-7.983%.
Window I: support-only repeat_batch=8; MAD/median 19.2811%, drift 8.2812%, worse than N=1 reference 4.0619%/1.1605%; no P/C.
Window J: restored N=1 d7 BF16 rows40 qualification blocked; no P/C.
```

All raw samples and pre/post resource snapshots remain in the corresponding
`support/qualification-recovery-20261003/window-*` directories. No raw sample
was deleted or filtered into a score. The repeat-batch support change was
explicitly reverted by `b95b9bc5`; restored runner source SHA is
`86d6ad6df82ec7b2c6b414c2759a72a48567cf8f11a382a7fdcd003f45a955c0`.

This is a server measurement-environment blocker, not `LOCAL_REJECTED`,
`CORRECTNESS_FAILED`, or evidence that HBH-09 itself regressed. It does not
authorize V003, a new mechanism, Online submission, or a Route lifecycle
decision. Any future retry requires Planning/user direction and a changed
measurement environment or protocol approved as support-only.
