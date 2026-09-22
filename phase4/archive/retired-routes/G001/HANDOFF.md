# G001 Final Handoff

ROUTE: G001
STATUS: PARKED
BEST_REVISION: V002
BEST_SCORE: official null; calc 9.69 (11/15)
BEST_PASS_COUNT: 11/15

## ARCHITECTURE
Fresh One-Read / Resident-Y. u = x + residual computed once and kept (native then FP32) in UB; RMS and norm+bias consume the same u so x/residual are not re-read. Scalar GetValue/SetValue-heavy element loops. Hot path when 4*dim fits ~160KiB else generic two-pass FP32 fallback.

## WHAT_WORKED
- V002 FP32 intermediate + correct BF16 bitcast (`bits>>16`) + judge dtype encoding end-to-end: fixed 100% err cases; T12/T14/T15 became Pass. Best correctness of the family.
- `__builtin_cce_sqrtf` for scalar sqrt (sqrtf does not link on dav-2201).
- Isolated one-numerical-lever-per-revision methodology (clean attribution).

## WHAT_FAILED
- Native-u quantize / invRms chain (V003): large |u| FP16 overflow → T12/T14/T15 18–21% err.
- Native cast of norm before bias (V005) and u/rms intermediate cast (V007): same large-D blow-up class.
- RNE bit FromFloat (V004): nicked T11 0.011% without fixing T02/T04/T06/T07.
- Gamma apply order (V006) and pairwise sumSquares (V008): T02/T04/T06/T07 error frozen at 2–6%.
- Param UB cache (V009) alongside restored math: T11/T13 0.15% WA.
- Eight numerical levers exhausted without clearing T02/T04/T06/T07.

## UNFINISHED
- T02/T04/T06/T07 residual 2–6% never root-caused (possibly multi-row or reduction-width / tail-D class).
- Performance on the 11-pass set never optimized after numerical stop (T14 ~0.6–1.9 ms class).

## REUSABLE_IDEAS
- One-read resident-y concept (u computed once) for MIX / G002 successor.
- BF16 FromFloat must return `uint16_t(bits>>16)` on little-endian.
- Keep RMS/norm intermediates in FP32; only final output cast to native — do not cast large-magnitude intermediates.
- Judge dtype 0=FP32, 1=FP16, 2=BF16 end-to-end.
- `__builtin_cce_sqrtf` for aicore scalar sqrt.
- Per-case error deltas are the cheapest ablation signal.

## DO_NOT_REPEAT
- Intermediate native casts of `u`, `u/rms*gamma`, or `u/rms` on data paths that include large-D cases.
- Stacking multiple numerical experiments in one revision.
- Continuing scalar GetValue-family patches as the main G001 line — future one-read work should be a new G002 Fresh Vectorized Route.
- Treating V001 as purely Child-authored (see PROVENANCE.md).

## LAST_AGENT
general-4 (G001 resident-y one-read); later cancelled at park.

## LAST_COMMIT
`c139a33` phase4: record G001-V009 online result
