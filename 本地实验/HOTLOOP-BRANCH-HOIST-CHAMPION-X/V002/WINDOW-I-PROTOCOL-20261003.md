# HBH-09 V002 — Window I repeat-batch calibration protocol

```text
PREDECLARED_UTC=2026-10-03T08:14Z
ROUTE=HOTLOOP-BRANCH-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=HBH-09
DEVICE=4
PRIMARY_SHAPE=BF16_D6144_ROWS80
REPEAT_BATCH_N=8
RUNNER_EXECUTABLE_SHA256=76362979c08b1748f66fb63893f3727ae566e981b228a925808f413d65c22cf5
RUNNER_SOURCE_SHA256=c3cd60fbae5956ceed65d561ec2f77a1fef8641626a105479ffc24877b21358e
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=1aa6d8ecaa1af058b039341f63c67f4f67403bb4a8e5c6bc57dd96914f42cb65
```

This is a support-only measurement calibration. The runner executes eight
identical kernel calls between one device-event pair and reports the per-call
average; Parent and Candidate use the same setting. It does not change the
Kernel, source, shape, dtype, or correctness executable. The prior N=1 d4
qualification is the comparison reference, not a score.

Run exact-shape Parent qualification first with 45 warmups and 31 samples x 2
blocks. Only if this N=8 same-binary result passes, immediately run 8 paired
blocks with 31 samples per side under the same d4 lock. Keep raw data and all
resource snapshots. If N=8 does not improve both MAD/median and block drift,
do not treat the calibration as a valid Local score and do not create another
performance mechanism.

```text
CURRENT_STATE=WAITING_FOR_VALID_LOCAL_SCORE
LOCAL_SCORE=NONE
NO_NEXT_PERFORMANCE_REVISION=YES
ONLINE_READY=NO
```
