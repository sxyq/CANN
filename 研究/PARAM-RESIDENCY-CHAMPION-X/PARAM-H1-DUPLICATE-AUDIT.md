# PARAM-RESIDENCY H1 Duplicate Audit

ROUTE=PARAM-RESIDENCY-CHAMPION-X
HYPOTHESIS=H1-GAMMA-TWO-ROW-LIFETIME
DIRECT_PARENT=R31B-V011
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
AUDIT_STATUS=COMPLETE

## Scope

The proposed mechanism is: keep one FP32 gamma tile live across exactly two
existing adjacent row consumers, with bias still loaded per row, no tile
change, no row remapping, no new DMA form, and no new event family.

The audit distinguishes source-level similarity from the actual mechanism
that would be measured. A renamed implementation of an existing lifetime or
parameter-residency mechanism is not a new Revision.

## Historical Comparison

| Existing evidence | Mechanism actually covered | H1 relationship | Result |
| --- | --- | --- | --- |
| R014 / FULL-R014 | Load complete gamma/bias once and reuse across all rows when capacity permits | H1 is a narrower two-row form of the same parameter residency idea; exact V011 already implements the relevant reuse | DUPLICATE |
| FULL-R030 wide parameter reuse | Wide-path parameter reuse/residency, with a separate wide-row design and an Online runtime error | Same parameter-residency family; H1 cannot claim novelty merely by limiting the row span | DUPLICATE FAMILY |
| BATCH-RESIDENT-X V001 | R014-style parameter residency combined with contiguous multi-row batch DMA | H1 removes the batch-DMA part but retains the same row-invariant parameter reuse mechanism; the implementation carrier does not make the mechanism new | DUPLICATE FAMILY |
| COEFF-LOCALITY-X V001 | Two-deep FP32 parameter MTE2 prefetch in `ProcessWideFp32FullCacheRows` | Any H1 change that adds or reorders parameter staging is already covered by this axis | DUPLICATE IF PREFETCH/STAGING CHANGES |
| COEFF-LOCALITY-X V003 | Split-phase gamma/bias parameter emission using the existing x/residual slots | Any H1 change that relies on slot rotation or parameter lifetime across emission is covered by this adjacent experiment | DUPLICATE IF SLOT-ROTATION CHANGES |
| COEFF-LOCALITY-X V004 | Full-row parameter preload for single-row multi-tile cached rows | Same objective of removing repeated gamma/bias loads; H1 is not a new mechanism on paths already preloaded | DUPLICATE FAMILY |
| UB-LIVENESS-X H2 | Explicit parameter residency via cold staging; recorded as duplicate/defer-to-BATCH | Confirms that parameter lifetime/residency is already owned by a prior evidence family; not an unclaimed UB route | DUPLICATE / DEFERRED |
| R31B V011 itself | FP32 D=16384 uses one parameter tile for both rows in the full-y batch; D=8192 cached path preloads once for multi-row cores | This is the exact proposed H1 behavior in the direct parent | EXACT DUPLICATE |
| ASYNC-OVERLAP / ASYNC-TRIPLE | Parameter-load timing and MTE2/V/MTE3 overlap around existing row batches | Adjacent timing evidence, not a new two-row residency mechanism | ADJACENT, NOT NOVEL |
| ADDR / REDUCE / STORE | Address, reduction, or store paths; no independent gamma two-row lifetime | No direct duplicate, but none supplies a missing H1 runtime mechanism | NO DIRECT DUPLICATE |

## Why H1 Is Not a New Revision

The only potentially distinct claim would be a new lifetime on D=32768. The
exact parent cannot retain two FP32 `y` rows at that width under its fixed
176 KiB budget. The current chooser selects one row, so there is no existing
two-row consumer interval to extend. Realizing that claim would require one
or more of:

- changing the retained-y batch size;
- changing tile width;
- changing reduction or work-buffer allocation;
- changing row scheduling or batch structure.

Each option violates the declared H1 boundary and would no longer be a
gamma-only lifetime experiment. It would also overlap the already tested UB,
parameter staging, batch, or scheduling mechanisms.

## Gate Results

| Gate | Result | Evidence |
| --- | --- | --- |
| Exact parent identity | PASS | `线上结果/R31B/V011/submission.asc`, SHA-256 verified |
| Hot-path existence | FAIL | H1 behavior already exists at D=16384 and D=8192; D=32768 has no two-row interval |
| Duplicate audit | FAIL_DUPLICATE | R014/FULL-R014, BATCH-RESIDENT, COEFF-LOCALITY, and exact V011 coverage |
| UB capacity | PASS for parent; no candidate headroom proven | Current two-row D=16384 footprint fits; D=32768 two-row footprint does not |
| Dependency safety | PASS for existing parent behavior | Existing MTE2/V and V/MTE3 ordering protects the current staging reuse |
| Single-variable implementation | FAIL | Any new D=32768 two-row realization changes batch/UB/tile structure |

## Final Decision

PARAM_H1_GATE=FAIL_DUPLICATE
PARAM_H1_EXISTENCE_GATE=FAIL_NO_RUNTIME_MECHANISM
PARAM_H1_IMPLEMENTATION=FORBIDDEN
PARAM_V001=NOT_CREATED
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
PERFORMANCE=NOT_RUN
ONLINE=FORBIDDEN

This route must return to Planning / Review for a different hypothesis. Do
not rename H1, widen its scope, or create a V001 from a modified batch/tile
implementation without a new Planning decision.
