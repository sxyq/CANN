# Main Review — SCHED-ROWGROUP-X V001
SINGLE_CHANGE_AUDIT=PASS
CORRECTNESS=PASS
LOAD_QUALITY=LOAD_CONTAMINATED
Aligned control -26.9% under contamination => magnitude not attributable
LOCAL_CONFIDENCE=LOW
decision=NEEDS_ONE_MORE_LOCAL
Action: same V001; clean-window repeat of same 4 pairs including aligned control; no source change; no online.

## Round-2 Main Review
set-1 vs set-2 disagree (median -34.47% vs +1.80%); aligned control opposite signs both sets.
LOAD set-2: DEVICE_AICORE_IDLE_VLLM_HBM_RESIDENT — not sufficient clean.
decision remains NEEDS_ONE_MORE_LOCAL.
Next attempt requires VLLM stopped or HBM below threshold, not merely AICore=0.
No V002. No online. Source SHA locked.

## Round-3
No VLLM-free window (root/zhangkaijie); set1/set2 disagree; PROBE PARKED; correctness PASS; source frozen.

## L004 Main Review
PASS + LOAD_CONTAMINATED (parent sofm max 0.305) + DIR 0.5 → do not judge candidate.
decision=NEEDS_ONE_MORE_LOCAL. d4 released. Not ONLINE/REJECT/technical-fail.
