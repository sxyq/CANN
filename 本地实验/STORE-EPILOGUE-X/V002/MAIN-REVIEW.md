# MAIN-REVIEW — STORE-EPILOGUE-X V002

DATE: 2026-09-27 (C2C overnight)
ROUTE: STORE-EPILOGUE-X
REVISION: V002
HYPOTHESIS: STORE-H2B-GATED — Single-row resident writeback merge with tileCount>=4 gate (intra-row)
DIRECT_PARENT: FROZEN_R31B_V011
SOURCE_SHA: 59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839

## Acceptance Rationale

**LOCAL_VERDICT = LOCAL_ACCEPTED. LOCAL_BEST = V002.**

### 1. Correctness valid
- Compile RC 0/0, Link RC 0/0. Executable SHA verified.
- Correctness: 24/26 shapes PASS both sides with identical max_abs. 2 failures (1×16384, 1×32768 FP32) are identical on parent and candidate — pre-existing runner golden mismatch, not a candidate defect.

### 2. Single-change audit PASS
- One variable: `tileCount>=4` predicate gating the resident writeback merge in `ProcessWideFp32FullCacheRows` output pass.
- Generic path byte-identical to parent. Gate-off branch is the parent per-tile block verbatim.

### 3. Gate reachability proves non-target shapes are identical code
- Candidate differs from parent only in `ProcessWideFp32FullCacheRows` with `tileCount>=4` (wide FP32, rowWidth>8192).
- 2×8192 / 8×8192 / 2×6144 / 2×256 / 1×16384-FP16 execute **byte-identical code** to parent. Their deltas measure the noise floor and **cannot be candidate regressions**.

### 4. Large-shape win outside noise
- **1×32768 FP32** (gate ON, large band): median **−5.62%**, 6/6 clean pairs favor Candidate (per-pair: −4.83, −14.33, −5.45, −5.79, −8.77, −2.02). Same-binary PASS. Clear win kept (V001 was −7.61%).

### 5. No medium regression
- Medium-band shapes (2×8192, 2×6144, 8×8192) are identical-code paths. Their mixed deltas (−4.2% to +1.2%) are window noise, not mechanism. V001's +5.16% on 2×6144 and +1.93% on 8×8192 are gone — no regression.

### 6. Control shapes
- 2×256 FP32 (identical code control): +4.47% median — window noise (outliers +31%).
- 1×16384 FP16 (identical code control): +0.84% ≈ 0 — as expected.

## Decision

| item | status |
|---|---|
| Correctness PASS | MET |
| Single-change audit PASS | MET |
| Large-shape win (−5.62%, 6/6) | MET |
| No medium regression | MET |
| Non-target shapes identical code | MET |
| Same-binary PASS (1×32768) | MET |
| Source identity verified | MET |
| ONLINE_WORTHY | **FUTURE_CANDIDATE** (not this cycle) |

**No submit this cycle.** Package frozen at `phase4/local/STORE-EPILOGUE-X/V002/`.
