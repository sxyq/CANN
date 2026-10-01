# UB-LIFETIME-SAFE-CHAMPION-X V001 — Revision Declaration

STATUS: `DECLARED_BEFORE_SOURCE_EDIT`
ROUTE: `UB-LIFETIME-SAFE-CHAMPION-X`
REVISION: `V001`
MAIN: `MAIN-2 / M2-1`
MAIN_SELECTED: `YES` — only the concept named below, per the Planning selection
in the 2026-10-01 task directive.
CONTEXT_CLASS: `MAIN_SELECTED_CHAMPION_EXPLOIT`

## Parent and Official anchor

- DIRECT_PARENT: `R31B V011`
- PARENT_SOURCE_PATH: `线上结果/R31B/V011/submission.asc`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- PARENT_SCORE / OFFICIAL_ANCHOR: `45.16` (`R31B V011`, promoted Overall Champion)
- The parent file's SHA-256 was recomputed and matches the declared value.
- Static lifetime gate: `PASS_STATIC_LIFETIME_PROOF`, recorded in
  `研究/UB-LIFETIME-SAFE-CHAMPION-X/BUFFER-LIFETIME-PROOF-V001.md`.

## Single hypothesis

HYPOTHESIS_ID: `UBX-H1-FP16-RETAINED-Y-INPLACE-OUTPUT`

In the existing FP16 wide `ProcessWideLowPrecision` path only, write each output
tile into the same `gammaBuf_` half-tile range that held its retained `yTile`,
after that tile's final retained-y read in pass 2. Omit only the separate
`outputBuf_` allocation from the existing wide-FP16 `Init` specialization.
The output view is the exact `batchRow * rowWidth + col` range read as `yTile`;
the parent source already proves `LAST_READ(yTile) :3318 <
REUSE_FIRST_WRITE(output) :3322`, with MTE3 store completion at `:3344` before
that address is reused for the next batch's retained y at `:3182`.

IMPLEMENTATION_GUARD: V011 line `:3299` unconditionally calls
`outputBuf_.Get<T>()` before the dtype branch. Because this revision removes
only the FP16-wide `outputBuf_` allocation, the candidate must construct the
half `outputLocal` view inside the existing `if constexpr (T=half)` branch
from `gammaBuf_`/`yStore[batchRow * rowWidth + col]` before `FromFloat` at
`:3322`; the `outputBuf_.Get<T>()` expression remains only for non-half
(BF16), whose allocation is unchanged. This is required source-use hygiene
for the same alias, not an additional optimization.

## Why this is not a duplicate

The Wave-1 `UB-LIVENESS-X V001-V003` alias architecture routes emit through
x-slots; it does not reuse V011's retained-y `gammaBuf_` range after its final
read. Its local 8-shape battery was not an exact-source proof for V011, and its
recorded Official result was 8/15, so it is used only as a risk reference.
The archived R31A-V016 staging source uses `gammaBuf_` as an output tile in a
BF16 parameter-staging path, while its FP16 path allocates a separate
`outputBuf_`; it has no full retained FP16 y array. R31A-V021 changes MTE3 wait
timing, which this revision explicitly does not copy.

The committed Track-B handoff marks the broader Wave-1 phase-role/live-y alias
family as `DUPLICATE_REJECTED`. Planning has now explicitly selected this
narrow, V011-specific lifetime transition. V001 does not revive the old
x-slot/pool alias implementation, does not extend y residency, and does not
remove a GM reread; it ends each existing retained-y tile lifetime at its
proven last read. Only this concept is authorized.

## Target shapes and dtypes

- Candidate dtype/path: FP16, wide path (`rowWidth > 8192`).
- Correctness primary: `[1,16384]`, `[3,16384]`, `[1,32768]`, and
  `[2,32768]` FP16. The multi-row cases use a one-block probe where supported
  to exercise reuse across `batchBegin`; a multi-tile tail probe `[1,16385]`
  is also included if the exact-source runner accepts arbitrary row widths.
- Unchanged controls: `[1,8192]` FP16 (wide-dispatch boundary), `[1,16384]`
  BF16 (wide low-precision non-alias path), and `[1,16384]` FP32 (wide FP32
  path). The exact correctness runner and source/executable identities will be
  recorded with the result.
- Local timing candidate focus: FP16 `[1,32768]` versus this Direct Parent;
  additional shapes are not inferred from that result and require their own
  qualification if measured.

## Expected effect and risks

- Expected allocation effect: remove `4096 * sizeof(half) = 8192` bytes of
  separate UB output-tile reservation per core in the wide-FP16 specialization.
- No latency or Official-score gain is assumed. Runtime arithmetic, output
  bytes/counts, tile loop, row mapping, dtype dispatch, DMA issue, event order,
  and synchronization remain unchanged. Only the output LocalTensor view and
  the now-unneeded FP16-wide `outputBuf_` allocation change.
- Main risks: a hidden y consumer missed by static audit; an in-place view or
  TBuf allocator/compiler restriction; and a delayed MTE3 read racing with
  reuse. The proof found no later retained-y consumer, uses a same-dtype half
  view within `gammaBuf_` capacity, and preserves the per-store
  `SyncMTE3ToV`; exact-source compile/link and targeted NPU correctness remain
  mandatory gates.

## OFAT boundary

Allowed source edits are limited to (1) omitting the wide-FP16 `outputBuf_`
`InitBuffer`, and (2) binding the existing per-row `outputLocal` view to the
corresponding retained-y `yTile` in the already-existing FP16 specialization.
No second alias, buffer resizing, dispatch change, arithmetic change, DMA
shape/count/timing change, store issue change, event/sync change, tile change,
or row-map change is allowed. No Online submission is authorized.

At declaration time the candidate has not yet been edited, built, or run.
