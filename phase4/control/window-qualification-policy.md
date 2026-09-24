# Window Qualification Policy (MAIN-2)

AICore 0% is NOT sufficient for timing.

Before any Candidate on a device:
1. Parent baseline only, ≥4-6 samples (PRECHECK-A)
2. Short gap
3. Parent baseline again ≥4-6 samples (PRECHECK-B)
4. QUALIFIED only if BOTH blocks:
   - CV <= 0.15
   - max/min <= 1.30
   - record median/min/max/CV/robust spread/processes/HBM/AICore
5. Else WINDOW_UNQUALIFIED: release device, no Candidate.

Retry budget: max 2 independent WINDOW QUALIFICATION attempts per route.
If both fail: NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED (SERVER_RESOURCE_BLOCKED).
Not LOCAL_REJECTED, not PARK.

SCHED special: STRONG_POSITIVE_LOCAL_SIGNAL BUT LOAD_NOT_QUALIFIED (not ONLINE_CANDIDATE).
