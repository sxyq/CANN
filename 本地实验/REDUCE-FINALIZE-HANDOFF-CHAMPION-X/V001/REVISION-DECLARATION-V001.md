# REDUCE-FINALIZE V001 Revision Declaration

```text
ROUTE=REDUCE-FINALIZE-HANDOFF-CHAMPION-X
REVISION=V001
HYPOTHESIS_ID=RFH-1
HYPOTHESIS=DEFER-TERMINAL-INPUT-RELEASE-WAITS
MAIN_SELECTED=YES
DIRECT_PARENT=R31B V011
PARENT_OFFICIAL_SCORE=45.16
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SOURCE_COMMIT=43a1049a1e08e518c88e354a754fdebb85a96f99
BASELINE_SUBMISSION_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
SINGLE_HYPOTHESIS=Move only the terminal rel0/rel1 V_MTE2 waits and matching
  event-ID releases from immediately after pass 1 to after the existing
  per-row reduction finalizer and before SyncVToMTE2/pass-2 reuse.
EXACT_FUNCTION=ProcessWideLowPrecision()
EXACT_CODE_SITE=V011 submission.asc approximately lines 3212-3221; finalizer
  lines 3224-3241; existing SyncVToMTE2/pass-2 handoff lines 3243-3265.
CURRENT_BEHAVIOR=Both terminal input-slot release waits and event-ID releases
  complete before the existing RMS finalizer.
TARGET_SHAPE=Primary score probe BF16 [1,32768]; wide low-precision multi-tile
  correctness coverage includes relevant FP16/BF16 cases and a control.
TARGET_DTYPE=BF16 primary; FP16/BF16 correctness coverage.
EXPECTED_EFFECT=Overlap already-pending terminal input-release waits with the
  existing reduction-finalization tail; no extra DMA or compute is introduced.
WHY_NOT_DUPLICATE=Does not alter dynamic address/index expressions, generic
  guards, N=2 static-loop control, reduction topology, scalar/vector finalizer,
  prefetch depth, buffer count, or MTE3 store drain.
EXPECTED_RISK=Event-ID lifetime/order; unsafe xBuf_/residualBuf_ reuse or a
  deadlock if waits are not complete before the existing pass-2 handoff.
CONTEXT_CLASS=FRESH_WAVE2_ROUTE_AGENT
SOURCE_SHA256_BEFORE_IMPLEMENTATION=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
IMPLEMENTATION_STATUS=NOT_STARTED_AT_DECLARATION
ONLINE=NO
```

The baseline `submission.asc` in this directory is an exact copy of the
R31B V011 submission and must remain unchanged until the Route Agent begins
the one approved implementation. This declaration does not authorize any
other source or performance change.
