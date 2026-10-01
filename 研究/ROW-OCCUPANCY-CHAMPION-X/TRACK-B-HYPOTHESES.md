# ROW-OCCUPANCY-CHAMPION-X Track-B Hypotheses

ROUTE=ROW-OCCUPANCY-CHAMPION-X
TRACK=TRACK-B ONLY
WORKTREE=/home/data4t2/lelinfeng/cann-w2-m2-occupancy
BRANCH=w2/m2/row-occupancy
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE_COMMIT=43a1049a1e08e518c88e354a754fdebb85a96f99
PARENT_SCORE=45.16
OFFICIAL_ANCHOR=45.16
MAIN_SELECTED=NONE
REVISION_CREATED=NO

## Scope lock

This package contains exactly three research hypotheses. They vary only rows,
active-core count, row-to-core ownership, row grouping, or static work
distribution. They do not propose a tile, reduction, DMA mechanism, store,
arithmetic, or parameter-residency change.

The frozen parent currently launches
`blockCount = min(availableCoreNum, rowCount)` and uses a contiguous
`baseRows/extraRows` split in the narrow path. The wide helpers repeat their
own parent base/extra split. The mode predicates, including `localRows > 1`,
are observations and are not a permitted edit surface.

No hypothesis below is selected. Planning / Review Layer must select at most
one before any Revision declaration. No build, server3 run, correctness run,
timing run, profiling run, Online submission, or Judge action was performed
for this package.

---

## H1 - Bounded logical-block oversubscription

HYPOTHESIS_ID: ROW-OCC-H1-BOUNDED-LOGICAL-BLOCKS

MECHANISM: Keep the parent device-side row-unit `baseRows/extraRows`
ownership byte-for-byte unchanged, but allow a bounded number of logical
blocks above the physical `availableCoreNum` in a row-count regime where
`rowCount > availableCoreNum`. A concrete OFAT candidate is a fixed 2x
logical-block ceiling:
`blockCount = min(rowCount, 2 * availableCoreNum)`.
The hardware schedules the logical blocks over the same physical cores; no
new queue, atomic, row group, or device-side work loop is introduced.

BOTTLENECK: The parent has one logical block per physical core when rows
exceed core count. That gives a coarse static packet size and leaves the
hardware with no extra block boundary at which to smooth per-block startup,
tail, or execution variation. The hypothesis tests whether a small logical
work-packet surplus improves occupancy despite unchanged physical core count.

DIRECT_PARENT: R31B V011

PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

TARGET_SHAPES: Row counts 64, 96, 128, and 192, with existing narrow, mid,
and wide shape classes where the parent currently launches exactly
`availableCoreNum` blocks. Use representative widths already present in the
15-case input family; do not invent a new shape solely to claim a result.

TARGET_DTYPES: FP32, FP16, and BF16, with the same row count and width held
constant between parent and candidate.

WHY_IT_MAY_HELP: More logical blocks can expose additional scheduling
boundaries while each physical core remains available. If block startup or
tail duration is uneven across blocks, a bounded surplus may reduce the
critical-path effect of one slow packet. The change preserves the parent
per-row arithmetic and ownership coverage, so any signal would be attributable
to launch width / occupancy rather than a new kernel algorithm.

WHY_IT_MAY_FAIL: Rows have uniform work, so the parent base/extra split is
already close to balanced. A 2x logical-block count doubles block entry,
TPipe initialization, and fixed event overhead without adding physical
compute capacity. Hardware may execute the extra blocks in waves and make
latency strictly worse. A benefit on one short shape can also be an artifact
of launch variance rather than a stable occupancy effect.

UB_IMPACT: No UB layout, buffer size, lifetime, alias, or tile allocation
change. More logical blocks mean more independent instances of the existing
per-block UB initialization; this is a cost risk, not a capacity change.

DMA_IMPACT: Total GM traffic and per-row copy primitives are unchanged. The
only possible change is the temporal interleaving of existing row copies from
more logical blocks.

SYNC_IMPACT: No cross-block synchronization, atomic counter, or barrier is
added. Existing intra-block events remain unchanged.

PRECISION_RISK: None expected. Row ownership and arithmetic order are
unchanged. The required correctness check would still cover all dtypes because
logical block count changes execution order, not values.

DUPLICATE_CHECK: NOT_DUPLICATE after audit. This is not the SCHED-ROWGROUP
32B group split, not SCHED-CHAMPION V002 active-core preservation, and not the
old group-quantized launch-width proposal. Those mechanisms reduce or gate
blocks by row groups; H1 deliberately keeps row-unit ownership and tests a
bounded block count greater than physical core count. It is also not a
dynamic task-pull queue because ownership remains static and no atomic is
introduced. The overlap with R016 is only the broad scheduling axis; R016's
recorded mechanism uses D-band task granularity and does not test logical
oversubscription.

RELATED_OLD_ROUTES: R016 / FULL-R016 scheduling are the closest broad
ancestors. SCHED-ROWGROUP-X and SCHED-CHAMPION-X are direct duplicate-audit
neighbors for group ownership and core count. R31A/R31B wide and pipeline
revisions, MIX-A dispatch revisions, and Wave-1 M2 routes do not use a
bounded logical-block surplus.

MINIMAL_EXPERIMENT: After Planning approval, use one host-only OFAT with the
fixed 2x ceiling on the frozen parent. Keep the device row partition and all
mode/path code unchanged. Compare only against the same parent binary and
record the actual block count for every probe. Reject the hypothesis if the
extra blocks do not beat the parent noise floor on at least one qualified
multi-row regime and do not remain neutral on a row-count control.

ROUTE_BOUNDARY: Host launch-width policy and occupancy only. Device ownership
must remain the parent contiguous row-unit split.

ALLOWED_CHANGES: One host-side formula for `blockCount`; associated research
metadata and an exact launch-count trace if a later approved Revision is run.

FORBIDDEN_CHANGES: Any `.asc`, device `.cpp`, `.h`, `.hpp`, CMake, runner,
host wrapper outside the single launch-width expression, row-group alignment,
row grouping, mode predicates, tile sizes, reduction, DMA descriptors,
store path, arithmetic, parameter residency, UB layout, atomics, or dynamic
task queues.

EVIDENCE_PATHS: `线上结果/R31B/V011/source-meta.json`,
`线上结果/R31B/V011/result.json`, `技术路线/冠军/champions.tsv`,
`归档/历史整理记录/review/staging/R31B-V011/src/submission.asc`,
`研究/SCHED-CHAMPION-X/TRACK-B-HYPOTHESES.md`,
`研究/SCHED-CHAMPION-X/TRACK-B-HYPOTHESES-V003.md`,
`研究/SCHED-ROWGROUP-X/next-hypotheses.md`.

PROPOSED_ONE_FACTOR_DIFF: Replace only the parent host rule
`min(availableCoreNum, rowCount)` with a fixed bounded logical-block rule.
Do not edit the device `baseRows/extraRows` calculation.

EXPECTED_LOCAL_PROBES: Future only. First qualify the parent and candidate on
the same device, runner, shape, and dtype. Then use interleaved parent/candidate
device-event samples on row counts 64/96/128/192, with row-count controls at
or below the physical core count. Keep raw samples and reject any result that
is inside the same-binary floor or that changes a control unexpectedly.

CHILD_RECOMMENDED_HYPOTHESIS: REVIEW_PRIORITY=2; keep as a bounded occupancy
probe only if Planning wants to test hardware logical-block scheduling. This
is a recommendation for review, not a selection; MAIN_SELECTED remains NONE.

OPEN_QUESTIONS: Does the target scheduler gain anything from more logical
blocks when all physical cores are already occupied? What is the fixed per-
block initialization cost relative to one row for each target dtype? Does the
effect persist across narrow and wide paths without touching their code?

---

## H2 - Near-divisor active-core count

HYPOTHESIS_ID: ROW-OCC-H2-NEAR-DIVISOR-CORE-COUNT

MECHANISM: Keep the parent contiguous row-unit ownership and choose a
conservative active logical block count that is an exact divisor of `rowCount`
when the nearest divisor is close to `availableCoreNum`. A concrete OFAT rule
is: for `rowCount > availableCoreNum`, choose the largest divisor of
`rowCount` that is at most `availableCoreNum` and at least 75 percent of
`availableCoreNum`; if no such divisor exists, retain the parent block count.
No rowGroup, byte alignment, or D-dependent band is used.

BOTTLENECK: The parent `baseRows/extraRows` split gives some blocks one more
row whenever `rowCount % blockCount != 0`. This hypothesis asks whether
eliminating the remainder and giving every participating block the same row
count can reduce tail imbalance enough to offset using fewer active cores.

DIRECT_PARENT: R31B V011

PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

TARGET_SHAPES: Row counts 48, 64, 96, 128, 144, and 160 where the parent
physical-core count leaves a remainder and a near divisor exists. Include
divisible controls such as 40, 80, or 160 when those exact rows exist in the
test family; the rule must be a no-op for an already balanced launch.

TARGET_DTYPES: FP32, FP16, and BF16. Width is held constant and is only a
shape label; no dtype-specific path is proposed.

WHY_IT_MAY_HELP: Uniform row counts remove the parent one-row remainder and
make the row-to-core critical path predictable. This may matter when each row
is long enough that one extra row dominates a core's completion time. The
candidate also has a strict fallback, so prime or poorly factorable row counts
retain the parent policy.

WHY_IT_MAY_FAIL: The parent imbalance is at most one row, while reducing the
block count can leave more total rows per core. The lost parallelism is likely
larger than the saved remainder, especially for long rows. Divisor arithmetic
can be neutral on most row counts, and a small measured benefit can be launch
noise. This hypothesis is intentionally a high-risk, likely-negative control
of the occupancy model.

UB_IMPACT: UB allocation and lifetime are unchanged. Fewer active blocks mean
fewer concurrent UB instances, but each remaining block may retain more rows.

DMA_IMPACT: Total GM bytes and per-row DMA operations are unchanged. Contiguous
row ranges remain contiguous; only the number of ranges changes.

SYNC_IMPACT: No new cross-block or intra-block synchronization is introduced.
Existing per-row and per-tile synchronization remains untouched.

PRECISION_RISK: None expected. Each row keeps the same arithmetic and the
same reduction order. Correctness risk is limited to coverage arithmetic if a
divisor fallback is implemented incorrectly.

DUPLICATE_CHECK: NOT_DUPLICATE after audit. This is not 32B/group-aligned
ownership, group-quantized launch width, active-core preservation, even-split
task extents, cyclic ownership, or tail-group folding. It uses only the
rowCount factorization of the unchanged parent row-unit split. R016 and
SCHED-ROWGROUP-X provide the nearest scheduling context, but neither records
an exact-divisor active-core policy on the frozen parent.

RELATED_OLD_ROUTES: R016 / FULL-R016 and SCHED-CHAMPION-X are the direct
scheduling references. SCHED-ROWGROUP-X even-split and group-floor proposals
are adjacent but distinct and are listed as duplicate-rejected in the handoff.
R31A/R31B and MIX-A have no divisor-based core-count change.

MINIMAL_EXPERIMENT: After approval, change only the host block-count chooser
and log `rowCount`, `availableCoreNum`, chosen block count, and divisor status.
Use one shape with a valid near divisor, one prime or no-divisor fallback, and
one already-divisible control. Do not add a device-side balance loop.

ROUTE_BOUNDARY: Static row-unit work distribution and active-core count only.
The candidate must preserve contiguous ownership and all parent paths.

ALLOWED_CHANGES: One host-side active-core selection rule and its integer
factorization helper if needed; evidence and metadata for that rule.

FORBIDDEN_CHANGES: 32B row grouping, group rounding, cyclic mapping, task
queues, D-band task sizes, mode dispatch, tiles, reduction, DMA mechanism,
store, arithmetic, parameter residency, UB layout, source wrappers, runner,
or any change to the parent device ownership loop.

EVIDENCE_PATHS: `归档/历史整理记录/review/staging/R31B-V011/src/submission.asc`
lines 171-176 and 3532-3540, `线上结果/R31B/V011/source-meta.json`,
`技术路线/冠军/champions.tsv`, `研究/SCHED-CHAMPION-X/MAIN-APPROVAL-V002.md`,
`研究/SCHED-CHAMPION-X/TRACK-B-HYPOTHESES-V003.md`,
`研究/SCHED-ROWGROUP-X/next-hypotheses.md`.

PROPOSED_ONE_FACTOR_DIFF: In the parent host launch calculation, replace only
the selected `requestedBlocks` value with the near-divisor result and keep the
kernel ABI and device `baseRows/extraRows` equations unchanged.

EXPECTED_LOCAL_PROBES: Future only. Qualify parent and candidate separately,
then interleave at least four parent/candidate pairs on a near-divisor shape,
a no-divisor fallback, and an already-divisible control. The success bar is a
stable improvement beyond noise on the near-divisor shape with no regression
on the fallback or control; otherwise reject the policy.

CHILD_RECOMMENDED_HYPOTHESIS: REVIEW_PRIORITY=3; use as a falsification
probe for remainder imbalance only after the broader launch-width question is
resolved. This is not a selection; MAIN_SELECTED remains NONE.

OPEN_QUESTIONS: Is a one-row remainder measurable after parent/candidate
launch overhead is removed? Which row counts have a divisor close enough to
physical core count to make this policy plausible? Does wide-row execution
prefer more cores even when the split is not mathematically uniform?

---

## H3 - Few-row active-core cap

HYPOTHESIS_ID: ROW-OCC-H3-FEW-ROW-ACTIVE-CORE-CAP

MECHANISM: In the small-row occupancy regime, cap the number of logical blocks
below the parent one-block-per-row policy so each block owns multiple rows.
A concrete OFAT candidate is `rowCount <= 16` and
`blockCount = max(1, ceil(rowCount / 2))`; all other row counts retain the
parent block count. The device keeps the existing contiguous base/extra split.
The cap is a row-count regime rule, not a tile, dtype, or path-selection rule.

BOTTLENECK: For very few rows, per-block entry, TPipe initialization, and
fixed event setup can be a material fraction of total time. The parent uses one
logical block for every row when `rowCount <= availableCoreNum`; grouping two
rows per block may amortize that fixed cost and can expose the already existing
multi-row path without editing its predicate.

DIRECT_PARENT: R31B V011

PARENT_SOURCE_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

TARGET_SHAPES: Row counts 2, 4, 8, and 16, especially existing few-row
long-row cases, plus a short-row control at the same row counts. The width,
dtype, and path are held fixed within each parent/candidate pair.

TARGET_DTYPES: FP32, FP16, and BF16.

WHY_IT_MAY_HELP: Halving block entry and initialization instances can reduce
fixed overhead when there are too few rows to occupy a full launch. Each
remaining block has a deterministic two-row contiguous range, so the test
isolates occupancy amortization from row-group alignment and from dynamic
work distribution.

WHY_IT_MAY_FAIL: The parent intentionally exposes one row per physical core;
the cap removes concurrency and doubles serial row work per participating
core. Long rows can regress sharply, and a multi-row mode may have different
buffer or event costs even though its predicate is unchanged. Any gain on a
5-6 us kernel is especially vulnerable to the known measurement apparatus
floor and must not be accepted without a qualified control.

UB_IMPACT: No allocation or layout edit. A block retains existing buffers for
more rows in sequence, which may increase buffer lifetime but does not change
the parent allocation plan.

DMA_IMPACT: Total bytes and copy descriptors per row are unchanged. Copies are
serialized for more rows within each surviving block and are issued by fewer
blocks.

SYNC_IMPACT: No new synchronization primitive is allowed. Existing
intra-block events execute for the additional local rows; no cross-block
dependency is introduced.

PRECISION_RISK: None expected because row arithmetic is unchanged. Correctness
must nevertheless cover every dtype and the small-row boundary because
`localRows > 1` may select an existing path that was not selected by the
parent on the same shape.

DUPLICATE_CHECK: NOT_DUPLICATE after audit. This is the inverse occupancy
question to SCHED-CHAMPION V002's active-core preservation gate: it deliberately
tests a bounded under-subscription in the parent one-row-per-core regime and
does not use row groups. It is not R31A V008/V009 setup collapse, R31A V011
D-slice, R31B wide pipeline, MIX-A dispatch, or any Wave-1 DMA/pipeline route.
No reviewed route records this row-count-only cap as a candidate mechanism.

RELATED_OLD_ROUTES: R016 / FULL-R016 are broad scheduling references;
R31A's few-row and D-slice revisions are path/layout changes, not this cap.
SCHED-CHAMPION-X V001/V002/V003 and SCHED-ROWGROUP-X are the active-core and
row-group neighbors. R31B V006/V009/V011 and MIX-A are pipeline/dispatch
neighbors only.

MINIMAL_EXPERIMENT: After approval, change only the host `blockCount` rule
for `rowCount <= 16`. Use one narrow and one existing long-row shape at
rowCount 2, 4, 8, or 16, with the same-dtype parent as control. Do not edit
the device mode predicate, even if the cap causes `localRows > 1`.

ROUTE_BOUNDARY: Row-count regime selection of active logical blocks. All
device paths, mode conditions, and work inside each owned row remain frozen.

ALLOWED_CHANGES: One host-side small-row active-core cap and its explicit
row-count guard; research metadata and later timing evidence if approved.

FORBIDDEN_CHANGES: Any tile threshold, wide-path implementation, mode
selection condition, reduction, DMA segmentation, store, arithmetic,
parameter residency, UB allocation, synchronization pattern, row-group
formula, cyclic/task-pull ownership, runner, or wrapper change.

EVIDENCE_PATHS: `线上结果/R31B/V011/result.json`,
`归档/历史整理记录/review/staging/R31B-V011/src/submission.asc` lines
171-245, `技术路线/路线成绩表.tsv` R31A/R31B rows,
`研究/主代理/MAIN-2/CAMPAIGN-STATUS.md`,
`研究/SCHED-CHAMPION-X/TRACK-B-HYPOTHESES-V003.md`,
`研究/SCHED-ROWGROUP-X/next-hypotheses.md`.

PROPOSED_ONE_FACTOR_DIFF: Add only a host row-count guard that changes
`requestedBlocks` from `rowCount` to `ceil(rowCount / 2)` for the selected
small-row regime. Keep the kernel source and all device ownership code
unchanged.

EXPECTED_LOCAL_PROBES: Future only. Same-binary qualify the parent first on
each exact shape. Then interleave parent/candidate on row counts 2, 4, 8, and
16, with both short and long existing widths where available. Require a
stable gain beyond the null/control floor and no long-row regression; a
neutral or mixed result is `NEEDS_ONE_MORE_LOCAL`, not evidence of a win.

CHILD_RECOMMENDED_HYPOTHESIS: REVIEW_PRIORITY=1 for a narrow, row-count-only
occupancy test because it is the clearest non-group alternative; Planning must
still approve it explicitly and MAIN_SELECTED remains NONE.

OPEN_QUESTIONS: Is block initialization measurable in a qualified small-row
shape? Does two-row ownership amortize setup without creating a longer
critical path? Which existing judge cases, if any, are in the rowCount <= 16
regime?

---

## Track-B stop statement

These are research proposals, not Revision declarations. The route has not
selected an implementation, and the child has no authority to create a
Revision or change route lifecycle. The Planning / Review Layer owns the next
choice; until it acts, `MAIN_SELECTED=NONE` and all code and experiment
operations remain forbidden by this handoff.
