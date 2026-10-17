# MAIN-2 Window Qualification — SCHED-ROWGROUP-X V001

Lease: Q-SCHED-D6
Received: 2026-09-24T07:02:03Z
SHA: 0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c (unchanged)

## Preflight d6
2026-09-24T07:02:38Z · AICORE=0 · FREE_HBM=5661 · other timing NONE

## Precheck gate (CV<=0.15 AND max/min<=1.30, BOTH A and B)

- d6 A: {'n': 6, 'vals': [161.24, 217.58, 207.05, 114.52, 165.39, 112.279], 'median': 163.315, 'cv': 0.2486, 'min': 112.279, 'max': 217.58, 'max_over_min': 1.9379, 'passed': False, 'fail_reason': 'CV>0.15;max/min>1.30'}
- d6 B: {'n': 6, 'vals': [101.159, 87.86, 100.23, 220.72, 202.809, 88.11], 'median': 100.695, 'cv': 0.4183, 'min': 87.86, 'max': 220.72, 'max_over_min': 2.5122, 'passed': False, 'fail_reason': 'CV>0.15;max/min>1.30'}
- qualified_d6: False
- d4 A: {'n': 6, 'vals': [134.64, 99.61, 135.62, 126.28, 234.19, 134.98], 'median': 134.81, 'cv': 0.2922, 'min': 99.61, 'max': 234.19, 'max_over_min': 2.3511, 'passed': False, 'fail_reason': 'CV>0.15;max/min>1.30'}
- d4 B: {'n': 6, 'vals': [143.299, 125.44, 189.55, 133.91, 337.23, 267.99], 'median': 166.425, 'cv': 0.3919, 'min': 125.44, 'max': 337.23, 'max_over_min': 2.6884, 'passed': False, 'fail_reason': 'CV>0.15;max/min>1.30'}
- qualified_d4: False
- qualified_device: None
- attempts_used: 2 / max 2

No candidate pairs (no qualified window after max 2 attempts).

## Decision: **NEEDS_ONE_MORE_LOCAL**

WINDOW_UNQUALIFIED after max 2 attempts: d6 A/B fail (CV>0.15;max/min>1.30/CV>0.15;max/min>1.30); d4 A/B fail (CV>0.15;max/min>1.30/CV>0.15;max/min>1.30). No qualified window for pairs. SHA unchanged. Max 2 WINDOW QUALIFICATION attempts used today.

Devices released. support/results-window-qual/ written. No source change. No V002. No CANNJudge.
