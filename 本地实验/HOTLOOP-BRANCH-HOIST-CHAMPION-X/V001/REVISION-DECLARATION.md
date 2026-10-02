# HOTLOOP-BRANCH-HOIST-CHAMPION-X V001 Revision Declaration

## Identity

- `ROUTE`: `HOTLOOP-BRANCH-HOIST-CHAMPION-X`
- `REVISION`: `V001`
- `TRACK`: `A` / Main-2 Wave-2 independent BRANCH route
- `SINGLE_HYPOTHESIS`: `HBH-10`
- `BRANCH_KIND`: `WIDE_PASS1_SLOT_SELECT`
- `DIRECT_PARENT`: `R31B V011`
- `PARENT_SOURCE`: `线上结果/R31B/V011/submission.asc`
- `PARENT_SOURCE_SHA256`: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- `PARENT_OFFICIAL`: `45.16`
- `CONTEXT_CLASS`: `MAIN-2_WAVE-2_BRANCH_OFAT`
- `SINGLE_CHANGE_AUDIT`: `PASS_INTENDED_SINGLE_HBH-10_MECHANISM`

## EXACT_FUNCTION

`AddRmsNormBiasKernel<T>::ProcessWideLowPrecision()` in the exact parent
source, pass-1 block only (parent source lines 3076–3365; pass-1 loop lines
3125–3211).

## EXACT_BRANCH

The pass-1 flattened-unit slot-selection state:

- current selection `const bool useA = (u & 1) == 0` and the following
  `xLocal`, `residualLocal`, `rd`, and `rel` selections (parent lines
  3134–3139);
- lookahead selection `const bool nextA = !useA` and the following `xNext`,
  `rNext`, and `nrd` selections (parent lines 3155–3160).

The existing release checks, the terminal `u + 1 < units` guard, the final
pending waits, and the final `if (useA)` release-ownership update remain in
the same order and are not independently optimized.

## LOOP_LEVEL

One flattened pass-1 `u` iteration inside each existing `batchBegin` batch,
where `units = batchRows * tileCount`. The current slot is selected `units`
times per batch and the lookahead slot is selected `units - 1` times. Per
batch row, current selection executes `tileCount` times; lookahead selection
executes `tileCount` times for each non-final batch row and `tileCount - 1`
times for the final batch row. The initial `u == 0` load is preserved and is
not itself the hypothesis.

## INVARIANT_REASON

HBH-10 is only a representation change for the existing two local input
slots and their existing event IDs. The following must remain identical in
value, byte range, order, and ownership:

- `u`, `batchRow`, `tile`, `col`, `rowOffset`, `nRow`, `nTile`, `nCol`, and
  `nRowOffset`;
- current and lookahead `valid`/`nValid` min expressions, including the
  partial final tile;
- `xBase`/`rBase` offsets, all GM addresses, `rd0`/`rd1`, `rel0`/`rel1`,
  waits, loads, flags, `relA`/`relB`, `ReduceSum` destinations, arithmetic,
  final waits, and event releases.

The terminal unit must still issue no lookahead load or lookahead flag. No
indirect tensor/event lookup, new event schedule, spill, or buffer-lifetime
change is admissible.

## CURRENT_CONTROL_FLOW

Each runtime `u` iteration decodes the same row/tile and computes the same
current valid length. It derives `useA` from runtime `u` parity, selects the
current A/B local tensors and event IDs with ternaries, waits on the current
read event, conditionally selects the opposite A/B state for the next-unit
load, then performs the unchanged dtype-specific math and reduction. It sets
the current release event and updates the existing A/B release ownership.

The next-unit load remains guarded by `u + 1 < units`; the final iteration
does not load or flag a next unit.

## PROPOSED_ONE_FACTOR_DIFF

Replace only the repeated pass-1 A/B selection expressions with a rolling
current/next state representation initialized to the same A/B handles and
event IDs. The loop will use the current state directly, use the next state
directly for the existing lookahead load, and exchange the current/next
handles and event IDs once per completed unit. A boolean state may remain
only for the existing `relA`/`relB` ownership and lookahead release checks;
those checks and their order are retained.

This revision does **not** change pass 2, `tile & 1`, gamma/bias/`prd`/`prel`
selection, generic `Process()`, host dispatch, wide-path threshold, adaptive
tile geometry, addresses, DMA counts, event allocation/lifetime, barriers,
reductions, arithmetic, precision, buffers, or synchronization. HBH-09 and
HBH-11 are explicitly out of scope. Hand-written alternating bodies,
loop-peeling, fixed trips/offsets, rolling GM address cursors, array-indexed
or indirect tensor/event lookup, and any schedule change are rejected forms.

## TARGET_SHAPES

The implementation is for the existing non-float wide path only (`rowWidth >
8192`). The initial correctness/performance probes will include:

- FP16 `D=16384` as a multi-unit control;
- FP16 `D=32768`;
- BF16 `D=32768`, recording the runtime adaptive `tileWidth`, `tileCount`,
  `batchRows`, and `units` rather than assuming a 4096-element tile;
- one existing FP16/BF16 wide shape with a nonzero runtime tail, selected only
  after observing the unchanged adaptive geometry.

No fixed D bucket, tile-count bucket, or new dispatch gate is introduced.

## TARGET_DTYPES

Only `half` and `bfloat16_t` instantiations reaching the existing
`ProcessWideLowPrecision()` path. FP32 wide, generic/specialized paths, and
host dtype dispatch are controls/out of scope, not Candidate targets.

## WHY_COMPILER_DOES_NOT_ALREADY_ELIMINATE

The parent `u` and `tile` values are runtime loop state and `u` parity changes
on every iteration, so the source-level current/lookahead parity selections
are not loop-invariant. The exact parent compiles with the installed compiler,
but the parent-only evidence has no usable device IR or source-correlated
AICore disassembly; therefore compiler status is `UNVERIFIED`, not a claim
that the compiler retained the branches. If the emitted Candidate still
performs the same parity selection or an equivalent indirect lookup, the
result is `CODEGEN_NOOP` and no performance conclusion may be drawn.

## RISKS

- AscendC `LocalTensor` handle/state exchange or event-ID state exchange may
  be unsupported or lower to extra moves, spills, or indirect selection;
- an incorrect exchange point could change which slot is waited, loaded,
  flagged, released, or reduced, especially across batch-row and final-tile
  boundaries;
- tail and terminal-unit behavior could regress even if full tiles pass;
- FP16/BF16 instantiations may differ in register/code-size pressure;
- a build-pass Candidate may be an emitted-code no-op or may alter the event
  trace, in which case it must not proceed as a performance claim;
- correctness must pass before any same-binary qualification or timing;
- `cann-server3` is DNS-unresolved from the current runtime; the local host
  `hwnput3`/local `npu-smi` access and the supplied d0–d7 FREE_HBM snapshot
  will be recorded as the exact access/resource fact, and no remote result
  will be invented.

## Gate and lineage rule

This declaration is created before any V001 source edit. V001 must be built
from the exact R31B V011 parent above. A future HBH-09 sibling, if authorized
after V001 receives a clear verdict, must be created directly from the same
R31B V011 parent and must not use V001 as its parent. HBH-11 is not authorized
in this revision.
