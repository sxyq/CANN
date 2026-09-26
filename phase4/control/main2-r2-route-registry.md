# MAIN-2 R2 Route Registry

Owner: MAIN-2
Canonical workspace: `/Users/sunyiyang/Desktop/Project/cann`
Repository: `sxyq/CANN`
Canonical branch: `exp/independent-breadth`
Worktree root: `/Users/sunyiyang/Desktop/Project/cann-main2-r2/`
Created: 2026-09-27

## Frozen strong baseline

| Field | Value |
|---|---|
| FROZEN_BASELINE_NAME | R31B-V011 |
| FROZEN_BASELINE_SOURCE | `phase4/workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc` |
| FROZEN_BASELINE_SNAPSHOT | `phase4/online/R31B/V011/submission.asc` |
| FROZEN_BASELINE_SHA | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| OFFICIAL_ANCHOR | 45.16 (15/15) |
| OFFICIAL_SUBMISSION_ID | `6ab2c10c0304f72a56a0c5cb` |
| SOURCE_COMMIT | `43a1049a1e08e518c88e354a754fdebb85a96f99` |

Identity check 2026-09-27: workspace source bytes == online snapshot bytes == sidecar SHA. Both paths hash to the same value above.

Forbidden seeds: R31B current worktree, R31B V016, MAIN-1 uncommitted implementations, cherry-picks of MAIN-1 current experimental commits.

## SCHED exact Official (donor evidence)

| Field | Value |
|---|---|
| Route | SCHED-ROWGROUP-X V001 |
| Official | 15/15, 22.27 |
| Direct parent | R016-V001 COMPILEFIX-A, score 17.14 |
| Submission ID | `6ab6c41b694b590c3ce2ef67` |
| LOCAL_SHA == REMOTE_SHA | `0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c` |
| Mechanism | 32B-safe row-group ownership + shape-aware rows/task scheduling |
| Evidence | `phase4/online/SCHED-ROWGROUP-X/V001/`, `phase4/workspaces/SCHED-ROWGROUP-X/` |

## UB V003 Judge state

| Field | Value |
|---|---|
| Route | UB-LIVENESS-X V003 |
| Status | JUDGE_READY=YES, NOT_FORMALLY_SUBMITTED |
| Frozen SHA | `2eb9b5d087267a54fb84f8734847ecb68cf94b967102693c0d150fd57d6da7cd` |
| Judge Owner | MAIN-1 (designated 2026-09-25) |
| `phase4/online/UB-LIVENESS-X/` | absent |
| Disposition | GATED_WAIT_UB_V003_OFFICIAL — UB-CHAMPION-X not started until result returns |

## MAIN-1 current six routes (read-only for NON_OVERLAP)

Source: `phase4/control/main1-revision-ledger.tsv`, `phase4/control/scheduler.tsv`. MAIN-1 worktrees are READ/WRITE FORBIDDEN for MAIN-2.

| Route | Current revision | Latest single hypothesis | Reserved axis |
|---|---|---|---|
| R31B | V016 | FP16/BF16 wide-row full-y tile 4096→8192 | aggressive multimode / lowp wide tile |
| R31A | V021 | single fence-placement change | conservative exploit / fence placement |
| MIX-A | V007 | remove first SyncVToMTE2 before Load | hybrid architecture / synchronization-removal |
| WIDE-X-FRESH4 | V001 | wide-path tile 2048→4096 | wide-D specialist / wide-path redesign |
| MODE-X-R015C | r4 | map one row per block | multi-row DMA / rows-per-block / mode selection |
| DTYPE-SPECIAL-X | V001 | FP32 aligned local-copy helpers replace Adds(x,0) | dtype-specific specialization |

## NON_OVERLAP_MATRIX (MAIN-2 R2 vs MAIN-1)

| MAIN-2 R2 route | Core mechanism | vs R31B | vs R31A | vs MIX-A | vs WIDE | vs MODE-X | vs DTYPE | Verdict |
|---|---|---|---|---|---|---|---|---|
| SCHED-CHAMPION-X | row ownership / core assignment / 32B row-group scheduling | orthogonal (R31B=lowp wide tile) | orthogonal (R31A=fence) | orthogonal (MIX-A=sync removal) | orthogonal (wide path forbidden here) | orthogonal (MODE=multi-row DMA/rows-per-block) | orthogonal (dtype forbidden here) | PASS |
| REDUCE-HIER-X | RMS square-sum reduction topology only | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal (dtype forbidden) | PASS |
| EPILOGUE-FUSE-X | invRms→norm→gamma→bias→store dataflow only | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal | near-border, scoped to post-RMS arithmetic/dataflow only; must not enter dtype-specific paths | PASS_WITH_SCOPE |
| COEFF-LOCALITY-X | gamma/bias load locality within a tile | orthogonal | orthogonal | orthogonal | orthogonal | near-border if multi-row mode needed — STOP if required | orthogonal | DEFERRED |
| UB-CHAMPION-X | UB lifetime / alias only | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal | GATED on UB V003 Official |
| VECTOR-MATH-X | invD/mean/sqrt/rsqrt math sequence, unified FP32 intermediate | orthogonal | orthogonal | orthogonal | orthogonal | orthogonal | must not specialize per dtype | READY_LATER |

Forbidden MAIN-1 axes that MAIN-2 must not create: broad R31A/R31B exploit evolution, generic multimode evolution, MIX/hybrid architecture, synchronization-removal/fence-placement, wide-D specialist, wide-path redesign, multi-row DMA, rows-per-block/mode-selection, dtype-specific specialization, independent FP16/BF16/FP32 paths.

## SCHED-CHAMPION-X non-duplication audit

| Check | Result |
|---|---|
| Frozen R31B-V011 already has 32B row-group ownership? | **NO.** `Process()` uses only `blockIdx` / `baseRows` / `extraRows` partitioning (lines 171–176). No `rowGroup = 32/gcd(rowBytes,32)` rounding, no whole-group ownership rule. |
| SCHED-ROWGROUP-X V001 has the mechanism? | YES — donor only (`WHY_NOT_DUPLICATE.md`, submission header comments). |
| Equivalent mechanism under another name in frozen champion? | NO evidence found in frozen source search (`rowGroup`, `ownership`, `rowsPerTask` rounding absent). |
| Duplicate of old MAIN-2 SCHED-ROWGROUP-X? | NO — that route is the validated donor on a weak parent (17.14). This route re-applies the same single mechanism onto frozen 45.16 baseline in an independent workspace. |
| NON_DUPLICATION_AUDIT | **PASS** |
| MAIN_APPROVAL | **YES** — first-priority route |

## Route registry (R2 batch 1)

| Route | Branch | Worktree | Parent seed | Status | Priority |
|---|---|---|---|---|---|
| SCHED-CHAMPION-X | `exp/main2-r2-sched-champion` | `cann-main2-r2/SCHED-CHAMPION-X` | FROZEN R31B-V011 | CREATED / TRACK_B | 1 |
| REDUCE-HIER-X | `exp/main2-r2-reduce-hier` | `cann-main2-r2/REDUCE-HIER-X` | FROZEN R31B-V011 | CREATED / TRACK_B | 2 |
| EPILOGUE-FUSE-X | `exp/main2-r2-epilogue-fuse` | `cann-main2-r2/EPILOGUE-FUSE-X` | FROZEN R31B-V011 | CREATED / TRACK_B | 3 |
| COEFF-LOCALITY-X | — | — | — | DEFERRED (batch 2) | 4 |
| VECTOR-MATH-X | — | — | — | DEFERRED (batch 2) | 6 |
| UB-CHAMPION-X | — | — | — | GATED on UB V003 Official | 5 |

## Old MAIN-2 route disposition (evidence donors only)

| Old route | Disposition | Use |
|---|---|---|
| SCHED-ROWGROUP-X | VALIDATED_MECHANISM donor | Official 22.27; mechanism source for SCHED-CHAMPION-X |
| UB-LIVENESS-X | JUDGE_PENDING donor | wait Official before UB-CHAMPION-X |
| ALIGN-TAIL-X | EVIDENCE_ONLY | do not recreate; wide/tail collision with MAIN-1 |
| BATCH-RESIDENT-X | EVIDENCE_ONLY / PARK | do not restore multi-row DMA |
| ASYNC-TRIPLE-X | EVIDENCE_ONLY / PARK | do not restore sync/pipeline route |
| REDUCE-INVSCALE-X | EVIDENCE DONOR ONLY | REDUCE-HIER-X is a new declaration from frozen baseline; old V002 is not parent |

## Rules in force

- One Route = one Agent = one branch = one worktree = one context.
- Track-B (3–5 hypotheses) before any kernel edit. Main approves exactly one hypothesis.
- OFAT. One revision, one conceptual change.
- Do not wait for performance window to compile/link/correctness.
- Local timing follows `phase4/control/local-timing-protocol.md` only.
- Online is scarce: only ONLINE_WORTHY, one at a time, via Judge Owner.
- MAIN-1 worktrees READ/WRITE FORBIDDEN.
- Shared control is Main-owned; Route Agents write only inside their own worktree route dirs.

## Track-B outcome and Main approval (2026-09-27)

| Route | Track-B file | Candidates | Approved V001 hypothesis | Approval doc |
|---|---|---|---|---|
| SCHED-CHAMPION-X | `cann-main2-r2/SCHED-CHAMPION-X/phase4/research/SCHED-CHAMPION-X/TRACK-B-HYPOTHESES.md` | 4 | H1 GROUP-ALIGNED BALANCED OWNERSHIP | `.../MAIN-APPROVAL-V001.md` |
| REDUCE-HIER-X | `cann-main2-r2/REDUCE-HIER-X/phase4/research/REDUCE-HIER-X/TRACK-B-HYPOTHESES.md` | 4 | H1 Eager running-fold of tile partials | `.../MAIN-APPROVAL-V001.md` |
| EPILOGUE-FUSE-X | `cann-main2-r2/EPILOGUE-FUSE-X/phase4/research/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES.md` | 4 | H1 SCALE-FOLD | `.../MAIN-APPROVAL-V001.md` |

All three H1s are single-variable, orthogonal to MAIN-1 reserved axes, and distinct from old MAIN-2 donors. Implementation of V001 is in progress on the route branches.
