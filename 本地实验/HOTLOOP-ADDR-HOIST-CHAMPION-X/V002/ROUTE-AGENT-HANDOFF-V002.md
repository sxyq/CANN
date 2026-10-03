# ADDR V002 Route-Agent handoff

```text
ROUTE=HOTLOOP-ADDR-HOIST-CHAMPION-X
REVISION=V002
HYPOTHESIS=ADDR-H1 pass-1 row-base state
DIRECT_PARENT=R31B V011
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
BASELINE_COMMIT=f6cd9ae8
IMPLEMENTATION_STATUS=WAITING_ROUTE_AGENT
```

## Authorized implementation

Modify only `ProcessWideLowPrecision` in `V002/submission.asc`:

- current row base at the parent sites corresponding to `:3129–3130`;
- prefetched row base at the parent sites corresponding to `:3150–3151`;
- the associated current/next x and residual GM offset expressions.

The row-base state must be mathematically identical to V011 for every
`batchBegin`, `batchRows`, `tileCount`, `tileWidth`, and partial tail.  Retain
the existing decoder, `valid`/`nValid`, tile geometry, DMA call/count/bytes,
events, waits, barriers, reductions, math, precision, stores, and dispatch.

Forbidden: H3 stateful `(batchRow,tile)` decoder, whole-input cursor, tile or
dtype changes, UB allocation changes, synchronization changes, arithmetic
changes, store changes, or any second performance factor.

## Required handback

1. Candidate source diff and new source SHA.
2. `SINGLE_CHANGE_AUDIT=PASS` with exact changed lines.
3. Build evidence and executable identity.
4. Correctness evidence including nonzero `batchBegin`, one/multi-row,
   partial final batch, and BF16 reduced-tile geometry.
5. After correctness PASS, exactly 2–3 valid same-parent/same-candidate
   device-event local runs and the arithmetic average score.

No Online action is authorized.  If implementation cannot be completed by a
Route Agent, report `IMPLEMENTATION_BLOCKED`; do not modify the V002 parent
baseline or create V003.
