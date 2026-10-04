# UB-LIFETIME-SAFE-CHAMPION-X — bounded next-hypothesis review

DATE: `2026-10-04 UTC`
ROUTE: `UB-LIFETIME-SAFE-CHAMPION-X`
WORKTREE: `/home/data4t2/lelinfeng/cann-w2-m2-ub`
TRACK: `TRACK-B ONLY`
DIRECT_PARENT: `R31B V011`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
PARENT_OFFICIAL_SCORE: `45.16`
V001_SOURCE_SHA256: `9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989`
MAIN_SELECTED: `NONE`
REVISION_CREATED: `NO`

## Decision

`NO_3_TO_5_NEW_SAFE_HYPOTHESES_FOUND_WITHIN_CURRENT_ROUTE_BOUNDARY`.

The exact-parent and V001 evidence separates the known correctness instability
from V001's selected FP16-wide lifetime change: exact R31B V011 is also
non-repeatable on `FP32-wide-D16384`, while the tested FP16/BF16 cases pass for
both sources. That makes the parent instability a bounded FP32-wide issue; it
does **not** validate any new low-precision candidate or remove the requirement
for its own exact-source correctness gate.

The existing route handoff already enumerates the distinct UB lifetime options.
After applying this task's requirement that an alias/reuse be fully proven
before it is proposed, H1/H2/H4/H5 do not qualify as new candidates: each still
has an unresolved typed-view, reduction-workspace, exact-path, or event/slot
proof obligation. The only remaining researchable item is the existing H3
dead-allocation omission below. It is independent of the V001 FP32-wide
parent-instability, but it is **not a new mechanism** and must not be counted as
a fresh hypothesis. No further alias/reuse is proposed by this pack.

## Evidence boundary

- The SHA-bound parent is `线上结果/R31B/V011/submission.asc`; its current
  bytes and the preserved V001 `parent-submission.asc` both hash to
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- V001 changes only the wide-FP16 retained-y/output storage role. Its original
  eight-case runner reported 7/8, with the failure on `fp32-wide-16384`.
- The corrected UB-DIAG-2 evidence ran the exact parent and V001 in two
  independent processes, twice each. Both sources pass the same seven listed
  FP16/BF16 cases; both fail and vary across runs on `fp32-wide-16384`.
  `研究/UB-LIFETIME-SAFE-CHAMPION-X/V001-FORENSIC-DIAGNOSTIC.md` therefore
  classifies the result as `PARENT_INSTABILITY`, not Candidate-only failure.
- The separate V001 local-score audit is
  `LOCAL_SCORE_RECORDED_ON_VALID_TARGET_NOT_ACCEPTED`, `QUALITY=POOR`; it is
  not correctness clearance, a valid parent promotion, or permission for V002.
- The exact tested domains do not include the narrow-mid shape proposed below.
  If Planning selects it later, that shape needs its own source-bound build and
  correctness evidence before any measurement.

Evidence files reviewed:

- `线上结果/R31B/V011/submission.asc` and its source metadata/result
- `研究/UB-LIFETIME-SAFE-CHAMPION-X/TRACK-B-HANDOFF.md`
- `研究/UB-LIFETIME-SAFE-CHAMPION-X/BUFFER-LIFETIME-PROOF-V001.md`
- `研究/UB-LIFETIME-SAFE-CHAMPION-X/V001-FORENSIC-DIAGNOSTIC.md`
- `研究/UB-LIFETIME-SAFE-CHAMPION-X/V001-GATE-AUDIT.md`
- `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/source-meta.json`
- `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/support/ub-diag-2/correctness-evidence-20261002.md`
- `本地实验/UB-LIFETIME-SAFE-CHAMPION-X/V001/LOCAL-SCORE-AUDIT-20261003.md`

## One existing survivor (not a new hypothesis)

HYPOTHESIS_ID: `UBX-H3-BF16-NARROW-MID-DEAD-PARAM-CACHE-OMISSION`

STATUS: `EXISTING_UNSELECTED_HYPOTHESIS`; `NOT_COUNTED_AS_NEW`;
`MAIN_SELECTED=NONE`.

DIRECT_PARENT: `R31B V011` (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`)

TARGET: BF16, exact single-row shape `[1,2048]` only for this narrowed audit.
This avoids FP32-wide and V001's FP16-wide source path.

MECHANISM: On the BF16 narrow-mid dispatch, omit only the two FP32 parameter
cache reservations that the selected function never accesses. Do not alias
them to another buffer or change parameter loads, conversion, compute, or
synchronization.

BUFFER: `gammaFp32Buf_`, `biasFp32Buf_` (two independent 8192-float
reservations); all native parameter and arithmetic buffers remain unchanged.

ALLOC: Parent `Init`, `:153-155`:
`pipe_.InitBuffer(gammaFp32Buf_, kCacheElems * sizeof(float))` and the matching
`biasFp32Buf_` allocation, with `kCacheElems=8192`.

FIRST_WRITE: `NONE` on the exact BF16 `[1,2048]` path. Cache writes occur in
the `cacheParams` generic/contiguous paths (`:267-280`, `:1679-1688`), neither
of which is entered for this one-row narrow-mid dispatch.

LAST_READ: `NONE` on the exact path. `ProcessNarrowMidOverlap` (`:499-620`)
gets and loads native `gammaBuf_`/`biasBuf_` (`:511-516`, `:535-541`) and uses
those native buffers; it never gets either FP32 parameter cache. The other
cache consumers are after earlier dispatch returns or in distinct special
functions.

REUSE_POINT: `NONE`. This is dead-allocation omission only; no alias, storage
reuse, or replacement buffer is proposed.

STATIC_PATH_PROOF: For BF16, `rowWidth=2048` is not wide (`:60-114`), is not
the `D==4096/8192` batched special path (`:212-221`), and with `rowCount=1`
has `localRows=1`, so it misses the aligned multi-row contiguous branch
(`:223-231`). Since `2048 > kMidRowMinWidth=128` and `2048 <= kTileElems=4096`,
dispatch enters `ProcessNarrowMidOverlap` and returns (`:234-240`). `Init`
receives both `rowWidth` and `rowCount` (`:46-48`), so this exact path predicate
is expressible without relying on runtime timing or local-row guesses.

WHY_IT_MAY_HELP: Avoids reserving `2 * 8192 * sizeof(float) = 65536` bytes of
UB for buffers with no first write or read on this path. No latency gain is
assumed; allocator/occupancy effect remains unknown.

WHY_IT_MAY_FAIL: A template/allocator requirement may retain or require these
members despite skipped `InitBuffer`; compile-time instantiation or an
unexpected path predicate could expose a cache consumer. The narrowed path
proof must remain exact. Existing V001 correctness does not cover BF16
`[1,2048]`.

DUPLICATE_CHECK: This is the same unselected H3 already documented in
`TRACK-B-HANDOFF.md`, not a fresh route mechanism. It is distinct from V001's
FP16-wide retained-y alias and from parameter-residency changes: no data
movement, cache contents, or parameter reuse changes.

MINIMAL_FUTURE_GATE (only if Planning selects): verify the same predicate in
the eventual selected source, then exact-source Build and BF16 `[1,2048]`
Correctness. No build, correctness, timing, NPU, or Revision action is
authorized by this research note.

## Prior candidates not admitted as new hypotheses

These are screened-out entries from the existing Track-B pack, not proposals
or selection recommendations:

| Existing item | Why not admitted under this request |
|---|---|
| H1: `xFp32Buf_` backing re-viewed as FP16 output | Cross-dtype view/capacity legality is not established; no alias is proposed without that proof. |
| H2: FP16 output backing re-viewed as FP32 `ReduceSum` storage, then output | `ReduceSum` workspace/alignment and typed-view contract remain unresolved; no alias is proposed. |
| H4: FP16 full-tile input slots reused as output ring | The broad exact dispatch/allocation predicate and odd-row event-slot coverage are still open; the lifetime sketch alone is insufficient. |
| H5: generic FP16 `xBuf_` reused as output | Exact generic-only Init predicate and the source/compiler use boundary are unresolved; no alias is proposed. |
| Prior `valueFp32Buf_` / `residualFp32Buf_` reduction-workspace alias | Already explicitly duplicate-rejected in the route audit; the workspace contract is unresolved. |

Active-buffer capacity right-sizing was not counted as a lifetime/dead-buffer
hypothesis: it expands beyond the current route handoff's stated boundary and
would need Planning to authorize that axis before further study. No such
change is recommended here.

## Planning handoff

- `NEW_SAFE_HYPOTHESES_COUNT: 0`
- `EXISTING_INDEPENDENT_SURVIVOR_COUNT: 1` (H3; not new)
- `3_TO_5_NEW_HYPOTHESES_AVAILABLE: NO`
- `V001_CORRECTNESS_CLASSIFICATION: PARENT_INSTABILITY` on exact FP32-wide
  D=16384; candidate-specific attribution unsupported by current evidence.
- `V001_PERFORMANCE: ENGINEERING_REFERENCE_ONLY / QUALITY=POOR`; no V002 parent.
- `MAIN_SELECTED: NONE`
- `REVISION_CREATED: NO`
- `KERNEL_OR_BUILD_OR_NPU_ACTION: NONE`
- `DASHBOARD_OR_SHARED_LEDGER_CHANGED: NO`
- `PUSH: NOT_REQUESTED / NOT_PERFORMED`

Planning may select the existing H3, request a bounded expansion of the UB
route boundary for capacity right-sizing, or conclude that the current
evidence does not support a 3–5-hypothesis wave. This handoff makes none of
those lifecycle or route-selection decisions.
