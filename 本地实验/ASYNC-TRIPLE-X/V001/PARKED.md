# ASYNC-TRIPLE-X V001 — IDLE (after TA-8x8192 Track-A floors)

```text
MAIN2_TRACK_A_20260925: same-binary floors UNQUALIFIED on both sanctioned shapes —
  8x8192  MAD/med 0.4565  drift 0.4427  (FAIL)
  8x4096  MAD/med 0.3449  drift 0.0893  (FAIL) [sanctioned fallback]
  both > 0.25 -> MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE; P/C pairs NOT RUN
DECISION: NEEDS_ONE_MORE_LOCAL + MEASUREMENT_BLOCKED
CORRECTNESS: smoke 8x8192 both binaries bad=0 max_abs 2.62e-06; floors bad=0
FROZEN_SOURCE_SHA256: 2defc6c270e898c00aa151dbef00f68c72ccf77f33e54db18c75b8bf4f4b4f9c (UNCHANGED)
STATUS: IDLE — next timing only in a window with no active sibling route lease
        (run_ref_floor.sh / run_ref_pairs.sh ready; condition is floor PASS)
SETS: results, results-set2, results-controlled-dev4, results-round2, results-window-qual,
      results-ref-8x8192, results-ref-8x4096
HANDOFF: HANDOFF-TRACK-A-20260925.md
V002: none
CANNJUDGE: none
```
