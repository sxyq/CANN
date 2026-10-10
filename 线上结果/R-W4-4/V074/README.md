# R-W4-4 V074 (MODE)

- Route: `R-W4-4` (MODE dispatch lane)
- Revision: `V074`
- Exact source: `843f6cee1ebf0caca9ff4539bef51e1c1273112c:本地实验/R-W4-4/V074/submission.asc`
- Candidate source SHA-256: `d3a471ccd4e82bb1244ecb0b991f693d52e5e674cd2def36a19f4b62a34d8592`
- Direct Parent: `R31B V011`; Parent source SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Scoped evidence commit: `fe8af2d5d237cdb8cb8287bae6c4c33e263da2b7`

## Source change

Use the V074 mode-dispatch cutoff at `8192` for the tested FP32 small-batch path. The receipt identifies this as the cutoff candidate; no additional source mechanism is asserted by this export.

## Local

Current retest scope: common Local dispatch FP32 shape `128x8192`; six interleaved pairs, 45 warmups, 31 samples in two batch-64 blocks. Pair formula: `pair_speedup = arithmetic_mean_i(P_i/C_i)` and `pair_speedup_pct = 100 * (pair_speedup - 1)`. The arithmetic pair result was `1.000850137413x` / `+0.085013741%`; 4/6 pairs were faster and 6/6 were within combined MAD. Pooled median reduction was `100 * (P_median-C_median)/P_median = +0.358056988%` from `21.2536/21.1775 us`; pooled mean reduction was `+0.475024625%` from `21.062348925/20.962297581 us`. CV was `12.776%/11.730%`. Verdict: `NOISY/NEEDS_ONE_MORE_LOCAL`; no Official-equivalent score.

## Target path and correctness

Target path executed: YES. The `8192` Parent/Candidate pair returned `rc0` with `bad0`; the neighboring `8184` Candidate correctness probe returned `rc3`, and both sides failed at `8200`. Compile: PASS after the existing CPLUS/LD environment fix. Correctness is therefore limited to the cutoff-boundary probes and is not an official-suite result.

## Risks and Official history

- Risk: noisy near-neutral Local signal, narrow cutoff boundary, and failed neighboring probes at `8184` and `8200`; the environment fix is part of the recorded setup.
- Previous-round route Official score: none recorded. The prior V096 package had `official_score=null`; its Parent R31B V011 anchor was Official `45.16`.
- Online: NO; no Online action was executed.
