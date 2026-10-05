# MAIN-2 Next Track-B — Final Evidence Audit V3 — 2026-10-04

```text
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL_SCORE=45.16
AUDIT_MODE=READ_ONLY_EXACT_SOURCE_AND_API_REVIEW
MAIN_SELECTED=NONE
IMPLEMENTATION_AUTHORIZED=NO
REVISION_CREATED=NO
KERNEL_CHANGED=NO
PLANNING_DECISIONS_CHANGED=0
NEW_ROUTES_OUTSIDE_APPROVED_5=0
```

This final audit refines the three candidates in V2 against the byte-identified
R31B V011 source and local CANN API implementation. `IMPLEMENTABLE=YES` means
an isolated source-level OFAT can be specified; it is **not** a Main selection,
Planning approval, codegen proof, or permission to create a Revision. The
installed Parent source hashes to the declared SHA above. No retained V011
device assembly/IR establishes the relevant emitted instruction counts.

The new ADDR V001/H3 Official result (42.72) does not alter the Official anchor
and does not authorize a performance Revision. It is a local-screen false
positive and strengthens the requirement for trustworthy multi-run, same-shape
evidence before any future Online recommendation.

## H1 — FP16 D8192 full-row parameter DMA coalescing

```text
HYPOTHESIS_ID=MAIN2-TB-PARAM-DMA-COALESCE-D8192-FP16
EXACT_FUNCTION=ProcessFp16FullRowOutputPipelined
EXACT_CODE_SITE=线上结果/R31B/V011/submission.asc:1004-1014; dispatch:201-210
CURRENT_CODE=col loop 0..8192 step 4096; Load gamma and bias per tile (4 Load calls)
BOTTLENECK=exposed MTE2 issue/setup overhead in the per-block parameter preamble
HOT_PATH_FREQUENCY=once per active block; amortized over localRows; not per row
TARGET_SHAPE=[M,8192], localRows>1 on the selected block
TARGET_DTYPE=FP16 only
EXPECTED_LATENCY_EFFECT=small or zero per active block
EXPECTED_THROUGHPUT_EFFECT=small or zero; no bytes are removed
DMA_IMPACT=parameter Load calls 4->2; same 16KiB gamma + 16KiB bias payload
SYNC_IMPACT=none intended; keep load phase and every existing wait/event unchanged
UB_IMPACT=none intended; existing gammaBuf_/biasBuf_ each hold 8192 half elements
COMPILER_CODEGEN_RISK=HIGH until exact device codegen confirms 4 issues and legal 16KiB DataCopyPad lowering
CORRECTNESS_RISK=LOW-MEDIUM; verify full-row offsets, layout, alignment, and exact output
DUPLICATE_CHECK=RELATED; novelty decision required before any implementation
RELATED_OLD_ROUTE=COEFF-LOCALITY-X V004/NH-1; R014 parameter-DMA work
ONE_FACTOR_DIFF=replace only the two-iteration gamma/bias tile-load loop with one full-row Load for each tensor
MINIMAL_EXPERIMENT=parent codegen count -> one OFAT -> Build -> exact [M,8192] FP16 correctness -> 3-run same-shape local score
IMPLEMENTABLE=YES (conditional on codegen and Planning novelty review)
```

Evidence: `Init` dispatches the FP16 full-row path only at `rowWidth==8192`
and `localRows>1`; its gamma/bias UB buffers already have 8192-element
capacity. The current loop is at source lines 1011-1014. `Load` uses one-row
`DataCopyExtParams`/`DataCopyPad`; a 16KiB FP16 row is aligned and fits the
length field, but legal source-level arguments do not prove the compiler emits
two efficient hardware copies. The official result's 15-case map does not
identify whether this exact target shape is represented.

## H2 — exact-preserving wide UB-fit selector simplification

```text
HYPOTHESIS_ID=MAIN2-TB-WIDE-UB-FIT-SEARCH
EXACT_FUNCTION=ChooseWideFullYRows
EXACT_CODE_SITE=线上结果/R31B/V011/submission.asc:1293-1329; callers in Init:60-100
CURRENT_CODE=search rows 8 down to 2, then try N=1 with tile decrements 4096->2048 by 512
BOTTLENECK=possible residual scalar loop/control in UB geometry selection
HOT_PATH_FREQUENCY=once per active block during Init; never per row or tile
TARGET_SHAPE=existing wide path, D>8192
TARGET_DTYPE=FP32, FP16, BF16
EXPECTED_LATENCY_EFFECT=very small or zero; selector is bounded setup work
EXPECTED_THROUGHPUT_EFFECT=very small or zero
DMA_IMPACT=none
SYNC_IMPACT=none
UB_IMPACT=must be exactly unchanged; returned (wideFullYRows_,tileElems) must match for all reachable inputs
COMPILER_CODEGEN_RISK=HIGH; compiler may already fold/unroll the bounded loops
CORRECTNESS_RISK=MEDIUM-HIGH; any off-by-one changes allocation geometry and may overflow UB
DUPLICATE_CHECK=DISTINCT from body unroll; related to UB sizing / R005
RELATED_OLD_ROUTE=TILECOUNT-STATIC-UNROLL H1-H3 (body-loop expansion, separately gated); R005/UB sizing
ONE_FACTOR_DIFF=rewrite only selector representation while preserving the exact output pair for every reachable input
MINIMAL_EXPERIMENT=enumerate all reachable dtype/width/buffer-factor inputs for equivalence -> inspect exact V011 codegen -> proceed only if dynamic instructions remain
IMPLEMENTABLE=YES (source-level; performance justification currently absent)
```

Evidence: the selector is invoked only from the wide `Init` branch when
`rowWidth>8192`. Its outputs feed UB allocation sizes, so an output mismatch is
not an acceptable performance trade. TCSU H1-H3 did not establish that the
body-loop control cost exists; those results neither prove nor disprove this
selector's cost. Because the selector is initialization-only and codegen is
unavailable, this is the lowest-priority candidate and must become
`CODEGEN_NOOP` if the compiler has already removed the search overhead.

## H3 — skip unused `paramReady` event-pool allocation on resident rows

```text
HYPOTHESIS_ID=MAIN2-TB-NARROWMID-UNUSED-PARAM-EVENT
EXACT_FUNCTION=ProcessNarrowMidOverlap
EXACT_CODE_SITE=线上结果/R31B/V011/submission.asc:499-619; dispatch:223-240
CURRENT_CODE=paramReady allocated at 506-507 and unconditionally released at 618; SetFlag/WaitFlag only when !residentParams
BOTTLENECK=event-pool software bookkeeping for an event ID unused on resident-parameter blocks
HOT_PATH_FREQUENCY=one allocation + one release per active block, not per row
TARGET_SHAPE=[M,D], localRows>1; preferred D=3072 (2048<D<4096)
TARGET_DTYPE=FP16, BF16
EXPECTED_LATENCY_EFFECT=small fixed per-block saving or zero
EXPECTED_THROUGHPUT_EFFECT=small fixed per-block saving or zero
DMA_IMPACT=none
SYNC_IMPACT=no hardware flag/wait change intended; keep existing conditional SetFlag/WaitFlag exactly
UB_IMPACT=none
COMPILER_CODEGEN_RISK=MEDIUM-HIGH; exact V011 device codegen must show the allocator path survives
CORRECTNESS_RISK=MEDIUM; every allocated ID must be released once, and no ID may be consumed on residentParams=true
DUPLICATE_CHECK=DISTINCT from stage-overlap changes; adjacent to sync/resource work and requires Main API review
RELATED_OLD_ROUTE=ASYNC-OVERLAP-CHAMPION-X, ASYNC-TRIPLE-X; no parameter traffic/residency change
ONE_FACTOR_DIFF=allocate and release paramReady only when !residentParams; retain all flags, waits, DMA, math, and stores
MINIMAL_EXPERIMENT=verify D=3072 dispatch/localRows on-device -> inspect allocation codegen -> Build -> FP16/BF16 correctness -> 3-run local score
IMPLEMENTABLE=YES (conditional on exact event-lifecycle and codegen proof)
```

The installed CANN 8.5.0.alpha002 `TPipe::AllocEventID` implementation uses
`sff0` and `sbitset1`; `ReleaseEventID` uses `sbitset0`. This confirms
source-level event-pool bookkeeping, not a hardware `SetFlag`/`WaitFlag` in the
resident case. The V011 path allocates `paramReady` before checking
`residentParams`, conditionally sets/waits only on the non-resident branch, and
then releases unconditionally. D=4096 and aligned low-precision D<=2048 can
dispatch elsewhere, so the probe must confirm the actual narrow-mid path and
`localRows>1`; D=3072 is a candidate probe, not an assumed official testcase.

## Planning ranking

1. **TOP-1 H1** — clearest countable hardware-command reduction (4→2) with
   unchanged bytes and buffers. Hard gate: Planning must resolve the close
   relationship to COEFF V004/R014, and codegen/official-shape coverage remain
   unverified.
2. **TOP-2 H3** — API source confirms real software pool bookkeeping and the
   conditional use site is exact. Expected ceiling is small; compiler output,
   path reachability, and event lifecycle require validation.
3. **TOP-3 H2** — isolated and exactly testable, but runs only in initialization
   and has no demonstrated emitted instruction cost; likely below noise.

```text
TOP-1=H1
TOP-2=H3
TOP-3=H2
MAIN_SELECTED=NONE
IMPLEMENTATION_AUTHORIZED=NO
NEXT=Planning chooses at most one hypothesis; no revision before that choice
```

## Permanent Main-2 Online screening correction

ADDR V001/H3 had a historical single-shape engineering score of `-24.193320%`
with `QUALITY=POOR`, but only 1/3 historical Candidate runs were faster. Its
formal result is 15/15 PASS at Official `42.72`, below the `45.16` Champion by
2.44 points. Treat this as `LOCAL_ONLINE_FALSE_POSITIVE`; do not convert the
local latency percentage into an Official score or call it a Local Accepted
revision.

For future Online recommendations, a `POOR` single-shape Local score alone is
never sufficient. Require clean correctness, at least 2/3 faster runs in a
valid same-shape protocol, mean latency improvement, throughput improvement,
a same-shape Champion baseline, and no single extreme run dominating the
average. This is a screening gate, not a change to the numeric evaluator; any
formal evaluator revision remains subject to Planning review and historical
replay.
