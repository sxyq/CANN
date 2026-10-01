# Buffer Lifetime Proof — UB-LIFETIME-SAFE-CHAMPION-X V001

## Identity and scope

- ROUTE: `UB-LIFETIME-SAFE-CHAMPION-X` (MAIN-2 M2-1)
- PROPOSED REVISION: `V001`
- SELECTED CONCEPT: `UBX-H1-FP16-RETAINED-Y-INPLACE-OUTPUT`
- MAIN_SELECTED: `YES` for this concept only, per the Planning selection in the 2026-10-01 task directive. This supersedes the earlier `MAIN_SELECTED=NONE` research-only state in the committed Track-B handoff; no other hypothesis is selected.
- DIRECT_PARENT / OFFICIAL_ANCHOR: `R31B V011`, Official `45.16`
- PARENT SOURCE: `线上结果/R31B/V011/submission.asc`
- PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` (verified against the file bytes)
- AUDIT_VERDICT: `PASS_STATIC_LIFETIME_PROOF`
- IMPLEMENTATION_STATUS: not implemented when this proof was written.

This proof is limited to the exact FP16 `ProcessWideLowPrecision` path in the
V011 parent. The single proposed storage transition is: after a retained
half-precision `yTile` has had its final read in pass 2, use that same
half-precision tile range for the output tile, in place of the separately
allocated `outputBuf_`. No DMA descriptor/count/order, event, synchronization,
arithmetic, tile, row mapping, dtype dispatch, or other alias is part of the
proposal.

## Exact path and allocation facts

All line numbers below refer to the SHA-bound parent source named above.

1. `Init` enters the wide path only when `rowWidth > kCacheElems` (`:60-61`;
   `kCacheElems=8192` at `:1288`). The FP16 specialization chooses a 4096-element
   tile and `wideFullYRows_`, allocates `outputBuf_` as 4096 half elements,
   and allocates `gammaBuf_` for `rowWidth * wideFullYRows_` half elements
   (`:67-68`, `:81-97`, `:1282-1284`). `ChooseWideFullYRows` sets the tile to
   `kWideFullYTileElems=4096` (`:1293-1298`).
2. `Process` dispatches a wide non-FP32 type to `ProcessWideLowPrecision`
   (`:163-169`, function begins at `:3076`). In its `T=half` pass-1 arm,
   `gammaBuf_` is viewed as `yStore`, and the tile at
   `batchRow * rowWidth + col` is written by `Muls` (`:3176-3185`). The
   pass-1 ping-pong inputs are `xBuf_` and `residualBuf_`; they are not
   `gammaBuf_` (`:3117-3121`, `:3135-3174`).
3. Per-row reductions complete before the pass-2 boundary
   (`:3224-3243`). Pass 2 loads gamma/bias into the independent ping-pong
   `xBuf_`/`residualBuf_` ranges (`:3256-3265`, `:3272-3297`); it does not
   overwrite the retained-y backing.
4. In the FP16 pass-2 arm, the same indexed `yTile` is converted to
   `valueFp32` by the only retained-y read (`:3313-3319`). Its FP32 value is
   scaled, then the existing `FromFloat(..., CAST_ROUND)` writes the separate
   output tile at `:3320-3323` (`FromFloat` definition: `:3397-3406`). The
   proposal makes that output view the exact same indexed half range as
   `yTile`; it does not change the conversion or subsequent in-place `Mul`
   / `Add` sequence (`:3324-3326`).
5. Output vector work is ordered before the existing store by
   `PipeBarrier<PIPE_V>` and `SyncVToMTE3` (`:3338-3340`). The existing
   `Store` issues `DataCopyPad` from its LocalTensor source (`:3341-3343`,
   helper at `:3377-3382`). The per-output `SyncMTE3ToV` at `:3344` waits for
   that MTE3 read/store to complete (helper: `:3427-3430`). These calls and
   their order remain unchanged.

## Buffer lifetime table

The alias edge involves the retained-y backing and the parent's separate
output backing. `valueFp32Buf_`, the parameter ping-pong buffers, and reduction
storage remain separate and unchanged.

| BUFFER | ALLOC | FIRST_WRITE | FIRST_READ | LAST_READ (old role) | REUSE_FIRST_WRITE (output role) | STORE_FIRST_READ | STORE_COMPLETE | NEXT_OVERWRITE (same address) |
|---|---|---|---|---|---|---|---|---|
| `gammaBuf_` half range `yStore[batchRow*rowWidth+col]` — selected alias backing | `Init`: `:91-92`, `rowWidth * wideFullYRows_ * sizeof(half)` | Pass 1 `Muls(yTile,...)`: `:3182` | Pass 2 `ToFloat(valueFp32,yTile,valid)`: `:3318` | `yTile` is read once, at `:3318`; the following vector barrier is `:3319` | Proposed `FromFloat(outputLocal,...)` at `:3322`, with `outputLocal` bound to the same indexed half range | Existing `Store` call `:3341-3343`; MTE3 reads the LocalTensor in `DataCopyPad` at `:3382`, after `SyncVToMTE3` at `:3340` | `SyncMTE3ToV`: `:3344` (wait implementation `:3427-3430`) | Next pass-1 write to this same indexed range is `Muls(yTile,...)` at `:3182` of a subsequent `batchBegin`. Other rows/tiles in this batch use disjoint `[index,index+valid)` ranges. The next batch cannot begin until the current pass-2 loop has completed; every store in that loop has already passed `:3344`. |
| `outputBuf_` half tile — parent's separate output backing, removed only from the selected wide-FP16 allocation/use | `Init`: `:88`, `tileElems * sizeof(half)` = 8192 bytes | Parent `FromFloat(outputLocal,...)`: `:3322` | Parent vector `Mul` / `Add`: `:3324`, `:3326` | MTE3's last source read is the `DataCopyPad` issued at `:3382` | Parent's next tile/row `FromFloat`: `:3322` | `Store`/`DataCopyPad`: `:3341-3343` / `:3382` | `SyncMTE3ToV`: `:3344` | Parent reuses this tile at the next `:3322`, after the previous iteration's `:3344`; candidate no longer allocates or reads this buffer in the wide-FP16 branch. |

For the selected alias range, the required strict ordering is therefore:

```text
LAST_READ(yTile)       = ProcessWideLowPrecision :3318
REUSE_FIRST_WRITE(out) = ProcessWideLowPrecision :3322
3318 < 3322
```

`PipeBarrier<PIPE_V>` at `:3319` and again at `:3321` preserve the existing
vector dependency before the in-place output write. Output-role vector reads
(`Mul` / `Add`) and the MTE3 store source read occur only after that first
output write. The `Store` completion wait at `:3344` precedes the same address's
next pass-1 write at `:3182` in a later `batchBegin`.

## Bounds and non-overlap

- The backing has `rowWidth * wideFullYRows_` half elements (`:91-92`). Each
  processed `batchRow` is less than `batchRows`, and `batchRows <= batchLimit
  == wideFullYRows_` (`:3089-3101`).
- For a tile, `col=tile*tileWidth` and `valid=min(rowWidth-col,tileWidth)`
  (`:3126-3133`, repeated for pass 2 at `:3267-3271`). Therefore
  `[batchRow*rowWidth+col,
  batchRow*rowWidth+col+valid)` stays within that row's retained-y range.
  Successive tiles and rows are disjoint; a short final tile writes only
  `valid` elements. The selected output view is the same `LocalTensor<half>`
  range and has capacity for at most `tileWidth=4096` elements, matching the
  removed output tile.
- `gammaBuf_` and `outputBuf_` are both `VECCALC` TBuf allocations. The
  proposed view remains `half`; there is no cross-dtype reinterpretation or
  increase in accessed length. `DataCopyPad` remains the same helper and
  receives the same `valid` count and output GM offset.
- `valueFp32Buf_` (`:93`) remains separate: pass 2 reads/writes it at
  `:3318-3322`, and it is not the source passed to `Store`; `Store` receives
  only `outputLocal` (`:3341-3343`). Gamma/bias continue to use the existing
  `xBuf_`/`residualBuf_` ping-pong views (`:3256-3297`). No other buffer is
  aliased or removed.
- The next overwrite of the same `gammaBuf_` range occurs only after the
  previous batch's pass-2 output loop (`:3245-3363`) and its final per-store
  `SyncMTE3ToV` (`:3344`); the outer `batchBegin` loop advances at `:3096-3101`.
  Thus an MTE3 store cannot still be reading the previous output when pass 1
  overwrites that address with the next retained `y`.

## Duplicate boundary and related evidence

- `UB-LIVENESS-X V003` uses phase-role emit aliases through x-slots, not the
  retained-y `gammaBuf_` range (`本地实验/UB-LIVENESS-X/V003/source-meta.json`,
  `ARCHITECTURE`). Its local battery passed 8/8 plus an alias-zero control, but
  the recorded official result was only 8/15; that evidence is not a correctness
  proof for this V011 path. The new candidate must pass its own exact-source
  correctness checks.
- The archived R31A-V016 staging source is byte-identified as
  `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0`.
  Its FP16 initialization reserves a distinct `outputBuf_` (`:143-152`); its
  `gammaBuf_`-as-output comment/code is in the BF16 path (`:153-158`), where it
  is a parameter staging tile, not retained FP16 `y`. R31A-V021's separate
  change deferred an MTE3 completion wait for a value tile it did not reuse
  (`本地实验/R31A/V021/source-meta.json`, `SINGLE_HYPOTHESIS`). V001 does not
  copy that sync change: the V011 MTE3 wait stays in place and is required by
  this proof.
- R31B-V011 itself retains complete FP16 `y` in `gammaBuf_` but writes output
  to a distinct `outputBuf_` (`:88-97`, `:3177-3182`, `:3315-3344`). The selected
  concept is not another full-y retention, pipeline, or arithmetic change; it
  ends each tile's retained-y lifetime at its existing last read and reuses
  only that dead half range for the same tile's output. It does not extend
  `y` residency or remove an input reread.
- The previously committed Track-B H1 (FP32 scratch to output) is not this
  selected concept. This proof and the subsequent V001 declaration apply only
  to `UBX-H1-FP16-RETAINED-Y-INPLACE-OUTPUT`.
- The Track-B handoff marks the Wave-1 phase-role/live-y alias family
  `DUPLICATE_REJECTED` (`:213`). That guard is not treated as blanket evidence
  that this different source range is safe or new: it rules out reviving the
  Wave-1 x-slot alias architecture. The current task explicitly selects this
  narrower V011 transition, whose distinction is the retained-y tile's
  statically proven final read at `:3318`, immediate same-tile output write at
  `:3322`, and preserved store-completion wait before the next y generation.
  No Wave-1 alias flag/pool architecture is carried over. The formal V001
  declaration will record this selection and duplicate boundary explicitly.

## Static audit decision

`PASS_STATIC_LIFETIME_PROOF`: exact source order proves
`LAST_READ(yTile) < REUSE_FIRST_WRITE(output)` and proves that the same address
is not overwritten for the next retained-y generation until after its prior
MTE3 store completion. This authorizes only the one declared FP16 alias. Typed
view compilation, exact-source link, and NPU correctness remain mandatory
independent gates; no performance conclusion is implied by this static proof.

Repository freshness note: the one bounded `git fetch origin main` attempt
timed out after 30 seconds. The local `origin/main` ref at
`ed860e392d7604694ac6664da60aff1fc1f4c04f` is therefore provisional and remote
freshness is `UNKNOWN`; the proof is bound to the verified local V011 source
blob and this worktree's committed route evidence.
