# REDUCE-FINALIZE-HANDOFF-CHAMPION-X — Planning handoff

```text
ROUTE=REDUCE-FINALIZE-HANDOFF-CHAMPION-X
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
MAIN_SELECTED=NONE
IMPLEMENTATION_APPROVED=NO
REVISION=NONE
ONLINE=NO
POOL_STATE=ONE_INDEPENDENT_RESEARCH_LEAD
```

## RFH-1 — DEFER-TERMINAL-INPUT-RELEASE-WAITS

- **Exact function/code site:** `ProcessWideLowPrecision()` at V011 `:3076`;
  terminal `rel0`/`rel1` waits and matching ID releases at `:3212–3221`;
  existing finalizer at `:3224–3241`; existing `SyncVToMTE2()` and pass-2
  input reuse begin at `:3243–3265`.
- **Single variable:** keep the same event IDs, SetFlag calls, wait count,
  input buffers, reductions, scalar finalizer, precision, tile geometry and
  pass-2 handoff, but move only the two terminal input-release waits/releases
  after the finalizer and before `SyncVToMTE2()`/pass-2 loads.
- **Target shapes:** FP16/BF16 wide low-precision multi-tile probes such as
  D=16384 and D=32768, with one-row and row-batched forms; shape mapping is
  not an Official-case claim.
- **Expected effect:** overlap otherwise idle terminal input-release waits
  with the existing final reduction/invRms tail.
- **Risk:** event lifetime/order and input-buffer reuse; a timing-dependent
  change can cause silent corruption or deadlock.  Correctness and event-ID
  audit are mandatory before timing.
- **Duplicate audit:** distinct from ADDR address/index expressions, BRANCH
  guards, TILECOUNT N=2 unrolling, REDUCE-HIER reduction topology, VECTOR-MATH
  scalar/vector work, ASYNC prefetch/overlap, and R31B V017 MTE3 store drain.

## Planning decision required

RFH-1 is a research lead only.  It is not a Main-selected implementation and
  has no Revision, Build, Correctness, Local score, or Online recommendation.
Planning/Review must decide whether the event-lifetime ambiguity is worth an
approved one-factor experiment.  No RFH-2/3/4/5 variant is admissible under
this route because those are duplicate or out-of-scope mechanisms.

```text
SELECTED_HYPOTHESIS=NONE
IMPLEMENTATION=FORBIDDEN_PENDING_PLANNING
NEXT_ACTION=Planning review of RFH-1 event-lifetime feasibility
```
