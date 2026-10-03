# MAIN-2 Approval — REDUCE-FINALIZE V001

```text
DATE_UTC=2026-10-03
ROUTE=REDUCE-FINALIZE-HANDOFF-CHAMPION-X
REVISION=V001
HYPOTHESIS_ID=RFH-1
HYPOTHESIS=DEFER-TERMINAL-INPUT-RELEASE-WAITS
MAIN_SELECTED=YES
DIRECT_PARENT=R31B V011
PARENT_OFFICIAL_SCORE=45.16
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
ONLINE=NO
```

This approval records the Planning decision in the current C2C instruction.
It authorizes one OFAT revision only. Main-2 does not edit Candidate Kernel
code; the independent Route Agent owns implementation and validation.

## Frozen scope

- Function: `ProcessWideLowPrecision()`.
- Exact site: terminal `rel0`/`rel1` `V_MTE2` waits and matching event-ID
  releases at the end of pass 1 (V011 source around lines 3212–3221).
- Single change: defer those same waits/releases until after the existing
  per-row reduction finalization, but before `SyncVToMTE2()` and any pass-2
  input-buffer reuse.
- Target: low-precision wide multi-tile input; primary score probe is BF16
  `[1, 32768]`. Correctness must also include the selected FP16/BF16 wide
  target and a relevant control case.
- Frozen: reductions, finalizer arithmetic, precision, tile geometry, DMA
  count/bytes, UB allocation, row mapping, queue depth, event IDs/count,
  pass-2 work and all unrelated synchronization.
- Expected effect: overlap the existing terminal input-release wait with the
  reduction-finalization tail.
- Risk: event lifetime/order and input-slot reuse. Preserve the same waits;
  complete both before the existing pass-2 handoff.
- Duplicate check: distinct from ADDR expression hoisting, BRANCH guards,
  TILECOUNT loop unrolling, REDUCE-HIER reduction topology, scalar/vector
  finalizer changes, prefetch/overlap additions, and MTE3 store-drain moves.

The local result is an engineering score only, never an Official Score. No
Online submission is authorized.
