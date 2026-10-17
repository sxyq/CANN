# Main Review — SCHED-ROWGROUP-X V001
SINGLE_CHANGE_AUDIT=PASS
CORRECTNESS=PASS
LOAD_QUALITY=LOAD_CONTAMINATED
Aligned control -26.9% under contamination => magnitude not attributable
LOCAL_CONFIDENCE=LOW
decision=NEEDS_ONE_MORE_LOCAL
Action: same V001; repeat the same 4 pairs after live HBM/lease admission and exact-shape same-binary qualification, including the aligned control; no source change; no online.

## Round-2 Main Review
set-1 vs set-2 disagree (median -34.47% vs +1.80%); aligned control opposite signs both sets.
LOAD set-2: historical VLLM/HBM observation; it is retained as evidence, not a current stop condition.
decision remains NEEDS_ONE_MORE_LOCAL.
Next attempt requires live HBM utilization below 100%, an unconflicted lease, and a qualified exact shape; AICore 0% or stopped VLLM is not required.
No V002. No online. Source SHA locked.

## Round-3
Resident VLLM was present (root/zhangkaijie); set1/set2 disagree; PROBE PARKED; correctness PASS; source frozen. The load is recorded, not used as an AICore-zero prerequisite.

## L004 Main Review
PASS + LOAD_CONTAMINATED (parent sofm max 0.305) + DIR 0.5 → do not judge candidate.
decision=NEEDS_ONE_MORE_LOCAL. d4 released. Not ONLINE/REJECT/technical-fail.
