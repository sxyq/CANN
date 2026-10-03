# HOTLOOP-ADDR-HOIST-CHAMPION-X V002 — ADDR-H1

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V002
DIRECT_PARENT=R31B V011
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL=45.16
HYPOTHESIS=ADDR-H1 pass-1 row-base state
SIBLING_OF=V001 ADDR-H3
MAIN_SELECTED=YES per current Planning control
CONTEXT_CLASS=MAIN-2 WAVE-2 ADDR TRACK-A
ONLINE=NOT_PERMITTED
```

## Exact function and code site

`ProcessWideLowPrecision` in the R31B V011 parent, pass-1 flattened stream:

- current row base: source lines 3129–3130;
- current x/residual GM operands: 3140–3143;
- prefetched row base: 3150–3151;
- next x/residual GM operands: 3172–3173.

## Current behavior

For each current and lookahead unit, V011 forms
`(batchBegin + batchRow) * rowWidth` independently, then adds the tile
column.  The logical `batchRow`, `tile`, `nRow`, `nTile`, `valid`, `nValid`,
reduction indices, and all DMA/event operations remain part of the parent
contract.

## Single-variable change

Keep the flattened loop, decoder, tile geometry, valid lengths, and all
current/next logical indices unchanged.  Introduce only a row-base state for
the pass-1 x/residual addresses: initialize the current row base once, reuse
it for interior tiles, and advance/correct it only at a row boundary for the
lookahead address.  The generated current and next GM offsets must be
byte-for-byte equivalent to V011 for every `batchBegin`, `batchRows`,
`tileCount`, and partial tail.

No H3 decoder change, rolling whole-input cursor, tile-count change, DMA
count/bytes change, UB allocation change, event/wait/barrier change, math or
precision change, store change, dispatch change, or second optimization is
allowed.

## Target and risk

```text
TARGET_DTYPES=FP16,BF16
TARGET_PATH=ProcessWideLowPrecision wide low-precision path
TARGET_PROBES=D=12288,16384,32768; reduced-tile BF16 tail where dispatch permits
EXPECTED_EFFECT=remove repeated pass-1 row-base multiply/address construction
PRECISION_RISK=LOW
ADDRESS_ROLLOVER_RISK=MEDIUM/HIGH
CODEGEN_GATE=required; parent assembly/IR is not retained
```

Correctness must include `batchBegin != 0`, one-row and multi-row batches,
partial final batches, and reduced-tile BF16 geometry before any timing.

## Duplicate audit

ADDR-H1 is distinct from ADDR-H3: H1 preserves the existing decoder and
changes only row-base formation, while H3 changes `(batchRow,tile)` decode
state.  It is also distinct from H2/H4/H5, which are rejected or subordinate
address surfaces, and from R005/R012/R028/R029, which change geometry,
alignment/ownership, reduction/synchronization, or dispatch/cache policy.

```text
SINGLE_CHANGE_AUDIT=PASS_PENDING_IMPLEMENTATION_REVIEW
SOURCE_STATUS=BASELINE_ONLY
BUILD=NOT_RUN
CORRECTNESS=NOT_RUN
LOCAL_SCORE=NOT_RUN
```
