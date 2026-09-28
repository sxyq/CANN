# Window Qualification Policy (MAIN-2)

AICore 0% is not a prerequisite for timing. For timing, use a device when live HBM utilization is below 100% and the shared lease has no conflict; resolve the append-only lease file by the last row for each `lease_id`, and record AICore and resident processes as load facts only. Device 7 follows the same rule as every other device.

For compile, link, source/executable identity, host build, and safe correctness work, the forced rule is: use up to 8 cards in parallel for 8 different versions, one version per card; continue each card while `FREE_HBM >= 100 MB`; pause that card only when `FREE_HBM < 100 MB`. AICore activity, VLLM residency, other processes, and high HBM use with at least 100 MB free do not stop the work. Do not wait for zero AICore or a VLLM-free window. This replaces the older over-conservative resource wording.

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
