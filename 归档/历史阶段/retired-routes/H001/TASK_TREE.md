# H001 Task Tree

Evidence: `phase4/control/results.tsv`, `phase4/online/H001/*/result.json`, Route Agent handoffs, Git commits.

```
H001 baseline (Fresh Small-D / High-R)
└─ V001 CE  online CE (local bf16 cast fixed earlier; online still CE)
    └─ V002 CE  npu_kernel_dev template rewrite  still online CE
        └─ V003 9/15 WA  B001-shaped skeleton  FIRST ONLINE RUN
            └─ V004 15/15 12.54  wide two-pass RMS + uint64 offsets  ← FIRST FULL CORRECTNESS
                ├─ V005 TLE 0/15  fused + ReduceSum + depth-2 DMA  deadlock
                └─ V006 15/15 12.55  fused ApplyRow only  (flat vs V004)
                    └─ V007 15/15 23.29  ReduceSum hot sum(u*u)  ← first big speed jump
                        └─ V008 15/15 29.04  ReduceSum wide ChunkSumSquares  ← H001 BEST AT PARK
                            └─ V009 ONLINE_READY not submitted  wide resident gamma/bias
```

## Node fields

| rev | parent | hypothesis | main change | compile | online | pass | score | important delta | decision |
|-----|--------|------------|-------------|---------|--------|------|-------|-----------------|----------|
| V001 | init | small-D multi-row + BF16 ToFloat fix | B001-ish kernel | local PASS | CE | 0 | — | online CE | template mismatch |
| V002 | V001 | judge template shape | npu_kernel_dev rewrite | local PASS | CE | 0 | — | still CE | ABI/exotic constructs |
| V003 | V002 | B001 15/15 skeleton shape | TQue + 3 entries + GM tiling | local PASS | done | 9 | calc 12.59 | T05/T09/T11/T12/T13/T15 ~99.9% WA | fix wide path |
| V004 | V003 | chunk RMS wrong for wide | two-pass full-row RMS + uint64 offsets | local PASS | done | 15 | 12.54 | FIRST 15/15 | baseline |
| V005 | V004 | fuse ApplyRow + ReduceSum + DB | three deltas | local PASS | done | 0 | — | TLE | deadlock class |
| V006 | V004 | fused ApplyRow only | single Cast of u | local PASS | done | 15 | 12.55 | flat | next lever |
| V007 | V006 | ReduceSum hot sum(u*u) | vector ReduceSum + 8KiB tmp | local PASS | done | 15 | 23.29 | T03 20→4.4, T04 71→14.4, T06 353→24, T08 2059→95, T14 178ms→23ms | continue |
| V008 | V007 | ReduceSum wide ChunkSumSquares | same on wide pass1 | local PASS | done | 15 | 29.04 | T05 531→36.8, T09 2602→220, T11 4765→419, T12 5667→353, T13 13044→1156, T15 386854→23771 | H001 BEST |
| V009 | V008 | wide resident FP32 gamma/bias | param cache when fit 80KiB | local PASS | NOT_SUBMITTED | — | — | unfinished | park |

## Unfinished at park
- V009 compiled ONLINE_READY (`logs/compile-12.log`), not submitted.

## Small-D win vs Champion
- After V007/V008, H001 small/mid cases (T03/T04/T06/T07/T08/T10) are in a competitive band, but **no sustained case-level beat of R31 Champion times** was recorded as a formal “H001 wins this testcase vs Champion” in r31-mode-experiments (that table is R31-only). Against A001 Fresh Best some mid cases are close; against R31A-V010 champion H001-V008 is still behind on aggregate (29.04 vs 44.09) though T05 36.77 is notable vs champion’s 10.89–15 us class on T05 — actually H001 T05 36.77 is slower than R31A T05 10.89. **No clear specialist win over Champion on the scored set at park.** Family paused mainly because exploration slots rotate for architecture diversity, not because small-D is solved.
