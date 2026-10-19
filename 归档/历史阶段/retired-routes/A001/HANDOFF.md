# A001 Final Handoff

ROUTE: A001
STATUS: PARKED
BEST_REVISION: V017
BEST_SCORE: 36.41
BEST_PASS_COUNT: 15/15

## ARCHITECTURE
Fresh Phase4 Control / clean-room streaming AddRmsNormBias.
Late architecture (V010+): FastKernel for small shapes (D<=4096, R*D<=262144, exact UB budget with 160KiB headroom) with gamma/bias loaded once per core through the param queue; ResidentKernel for the rest with full-u FP32 residency in UB, optional gamma/bias cache, optional 2D row×col split with output-tail partial RMS reduce, single GetValue per row (V016), x/res 2-slot issue-ahead (V017). V008-era path was two-pass chunked streaming of x+residual.

## WHAT_WORKED
- V010 full-u residency + param cache + 2D D-split: 28.10 → 33.25. Removed second x+residual pass; T14 118184→53177.
- V012/V014 FastKernel with param-once and conservative UB gate: small cases T01-T04 reached 3.5–19 us class.
- V016 single post-reduce GetValue: 35.89 → 36.35; T14 53260→50830.
- V017 x/res double-buffer issue-ahead: 36.35 → 36.41 without T05 crash.
- Isolation rule: one Resident delta per online run after V011/V013/V015 multi-change REs.

## WHAT_FAILED
- V009 retained-u output-GM wide path: score regression 28.10→27.72; T14 worse.
- V011 FastKernel + double-buffer + single-GetValue together: T01 100% WA (Init-time param MTE without queue sync), T05 RE (UB overflow near D=8192).
- V013 2M element FastKernel gate: T05 RE again (gate too wide despite UB check).
- V015 Resident single-GetValue + double-buffer + D-split together: T05 RE (multi-delta stack).
- Chunk/queue/DataCopy-only tweaks (V002–V008) plateaued at 28.10; not enough for 40+.

## UNFINISHED
- V018 host-only wideFew D-split (`D>=8192 && R<=2*cores`): compiled ONLINE_READY, not submitted. Commit `d7cac20`.
- T14 remains ~50050 us (r=13.3) at park — dominant remaining gap vs best 3750.

## REUSABLE_IDEAS
- Full-u FP32 UB residency avoids second x+residual GM read (MIX-A donor).
- Gamma/bias load-once-per-core through a queue (not Init-time raw MTE).
- Exact UB budget with explicit headroom (160KiB vs 184KiB) before enabling a fast path.
- T05 is the Resident canary: first case outside FastKernel, first to die on queue/UB bugs.
- Host-only dispatch changes are the safe way to enable a dark kernel path after kernel is green.
- FastKernel domain: small D/R with one-shot row in UB; ResidentKernel for wide/large-R.

## DO_NOT_REPEAT
- Stack multiple Resident deltas in one online shot after a multi-change RE.
- Widen FastKernel element gate without exact UB + headroom check.
- Init-time TBuf writes via raw MTE without queue EnQue/DeQue before vector reads.
- Continue from a scored regression parent (V009) instead of the last Fresh Best.
- Pure chunk 4096→8192 style tweaks as the main architecture change.

## LAST_AGENT
general-3 (A001 fresh control), session actor of this phase4 continuous-search loop.

## LAST_COMMIT
`68d6a12` phase4: record A001-V017 new Fresh Best 36.41
Source park state also includes `d7cac20` (V018 source, unsubmitted).
