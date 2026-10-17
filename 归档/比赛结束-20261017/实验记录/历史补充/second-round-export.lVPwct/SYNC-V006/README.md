# SYNC-BARRIER-ELISION-X V006

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V006`
- Exact source: `67c804313245d07978f663c052ec4e24b3a06ef4:本地实验/SYNC-BARRIER-ELISION-X/V006/submission.asc`
- Candidate source SHA-256: `acff22e50cc4aa2ddea833e6ef8da6625800740ac71cc77841e63cb11c6d6581`
- Direct Parent: `R31B V011`; Parent source SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Scoped evidence commit: `28e8122e`

## Source change

Delete one `PIPE_V` barrier immediately after `Muls` in `ProcessNarrowMidOverlap` on the FP16 path. This is one synchronization-operation deletion; no other kernel operation or pipeline behavior is claimed changed.

## Local

Current retest scope: FP16 rows `8`, width `2048`, core count `8`, `localRows=1`, exact V006 dispatch on device 3; 62 interleaved Parent/Candidate device-event pairs. Formula: `local_score_pct = 100 * (geomean_i(P_device_i/C_device_i) - 1)`. Pooled retest signal was `+0.065484922%`; repeat results were `+13.255631386%` and `-11.588491008%`. Pooled medians were Parent/Candidate `17.11/16.57 us`; CV was `0.731/0.673`. Verdict: `LOCAL_RETEST_REJECTED_NOISY` under VLLM load; no Official-equivalent score and no Local Best promotion.

## Target path and correctness

Target path executed: YES. The runner was labeled exact V006 and reached `ProcessNarrowMidOverlap` for the stated FP16 rows-8 width-2048 case. Compile: PASS. Correctness: PASS over FP16 widths `128, 256, 1024, 2048, 4096`, with zero mismatch in all 5/5 local width checks. Official-suite coverage is unknown; no official score is claimed.

## Risks and Official history

- Risk: repeat directions disagree, the retest is explicitly noisy/rejected, VLLM load was present, and correctness coverage is local rather than official-suite coverage.
- Previous-round route Official score: none recorded. The prior V002 package used the R31B V011 Official anchor `45.16`, but the SYNC route itself had no Official result.
- Online: NO; no Online action was executed.
