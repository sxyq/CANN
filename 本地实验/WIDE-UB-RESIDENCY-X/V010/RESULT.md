# WIDE-UB-RESIDENCY-X V010

RESEARCH_TRACK: N02 CODE-FOOTPRINT-ICACHE-X
REVISION: V010
PARENT: `本地实验/WIDE-UB-RESIDENCY-X/V007/submission.asc`
PARENT_SOURCE_COMMIT: `bd2e5f7f7c8f475f8057c16211f52959d3b7a33d`
BRANCH: `route/new-n02-wide-ub-residency-x-20261016`

## HYPOTHESIS

The four existing paired cases use widths 12288 and 32768. `Init()` selects
`widePath_` when `rowWidth > kCacheElems` (8192), and `Process()` returns from
the wide dispatch before reaching the mid-size branch. `ProcessNarrowMidOverlap()`
is called only for `128 < rowWidth <= kTileElems` (4096); its implementation
occupies about 120 source lines and is declared `__aicore__ inline`.

If the compiler currently inlines this cold mid-size helper into `Process()`,
outlining it may reduce the dispatch body and instruction-fetch competition for
the wide FP16/BF16 cases, without adding a helper call to those tested paths.
The helper call remains on mid-size rows. This tests cold-path code layout,
distinct from V009, which outlined the selected `ProcessWideLowPrecision()`
helper. The current source and build logs do not establish whether the compiler
inlined `ProcessNarrowMidOverlap()` before this change.

## ONE CHANGE

Add `__attribute__((noinline))` only to the existing
`ProcessNarrowMidOverlap()` declaration. Its body, dispatch conditions,
arithmetic, data movement, synchronization, buffers, tiles, and block assignment
remain unchanged. V010 starts from the V007 source; the failed V009 attribute
on `ProcessWideLowPrecision()` is not included. V008 remains uncompiled and
unrun.

## SOURCE EVIDENCE

- In V007 `submission.asc`, `widePath_` is set for `rowWidth > 8192` at lines
  60-61; `Process()` takes the wide helper and returns at lines 166-172.
- The call to `ProcessNarrowMidOverlap()` is guarded by
  `128 < rowWidth <= 4096` at lines 240-243. Its body is at lines 502-623.
- V007's paired cases are FP16/BF16 at D=12288 and D=32768; all exceed the
  wide-path threshold.
- V010's source difference from V007 is the function attribute on this helper.

## BUILD AND RUNNER

Reuse the existing `paired_runner_v016` target. Its candidate include and
`R31B_V016_SOURCE_DIR` now point to V010; the Parent remains R31B V011. No new
Runner or script was added.

COMPILE: NOT_RUN.
CORRECTNESS: NOT_RUN.
LOCAL: NOT_RUN; no new timing samples exist.
DEVICE_QUERY: NOT_RUN.
NOT_RUN_REASON: The current request limits all tool and device work to
`/Users/sunyiyang/Desktop/Project/cann`; the CANN toolchain and NPU execution
are outside that allowed filesystem scope. No outside path was accessed.

PREVIOUS_N02_STATUS: V007 Compile and Correctness passed; its four Local
P10-P90 intervals crossed zero. V009 Compile passed, Candidate Correctness
failed in the first case, and its timing is not interpreted. V008 remains
uncompiled and unrun.

STATUS: SOURCE_CANDIDATE_UNVALIDATED
PUSH: NO
