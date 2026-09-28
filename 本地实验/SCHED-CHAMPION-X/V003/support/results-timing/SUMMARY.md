# SCHED-CHAMPION-X V003 Timing (closing OFAT)

SOURCE_SHA: d2db679827e4be1fc4c9bd780ee19e74db080a17171a6cc3271a1126a4e65890
Hypothesis: H1 TAIL-GROUP FOLDING

## 33x100 FP32 (primary)
sb parent: FAIL (drift 6.671), sb cand: FAIL (drift 0.115)
p1: +1.9% CLEAN  p2: +0.0% CLEAN  p3: -1.4% CLEAN  p4: +10.9% CLEAN
median_delta: +0.9% (no improvement over V002 -4.4%)

## 17x257 FP16 (control)
sb parent: FAIL, sb cand: FAIL
p1: -10.1% CLEAN  p2: -1.7% CLEAN  p3: +3.9% CLEAN  p4: -8.8% CLEAN
median_delta: -5.2% (noise level)

## Verdict
Tail-group folding does NOT improve 33x100 (as predicted: folding makes
straggler 3 rows vs 2 rows). Confirms structural ceiling finding.
RECOMMEND PARK.
