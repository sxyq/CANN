# A001 Task Tree

Evidence: `phase4/control/results.tsv`, `phase4/online/A001/*/result.json`, Route Agent handoffs, Git commits.

```
A001 baseline (row-resident adapter S1)
└─ V001 23.47 15/15
    └─ V002 25.50 15/15  increase large-D streaming chunk
        └─ V003 25.74 15/15  aligned DataCopy 32B
            └─ V004 25.56 15/15  remove xFloat, enlarge chunks (regression vs V003)
                └─ V005 25.97 15/15  reuse inverse RMS
                    └─ V006 27.30 15/15  single-slot queues, larger chunks
                        └─ V007 27.24 15/15  direct x conversion into u
                            └─ V008 28.10 15/15  squareFloat residual + enlarge chunks  ← early Fresh Best
                                ├─ V009 27.72 15/15  FP32 D>=16384 retained-u  REGRESSION; do not continue from here
                                └─ V010 33.25 15/15  full-u UB + gamma/bias cache + 2D D-split  ← architecture rewrite
                                    ├─ V011 RE 3/15  FastKernel + double-buffer  (T01 WA 100%, T05 RE)
                                    └─ V012 34.38 15/15  FastKernel param-once + tight gate + V010 fallback
                                        ├─ V013 RE 4/15  param-once + 2M gate too wide  (T05 RE)
                                        └─ V014 35.89 15/15  param-once + D<=4096 && R*D<=262144 && fastUb<=160000
                                            ├─ V015 RE 4/15  Resident single-GetValue + double-buffer + D-split together
                                            └─ V016 36.35 15/15  single GetValue only  ← one-delta isolation green
                                                └─ V017 36.41 15/15  x/res double-buffer issue-ahead  ← FRESH BEST AT PARK
                                                    └─ V018 ONLINE_READY not submitted  host-only wideFew D-split
```

## Node fields

| rev | parent/base | hypothesis | main change | compile | online | pass | score | important delta | decision |
|-----|-------------|------------|-------------|---------|--------|------|-------|-----------------|----------|
| V001 | init | row-resident adapter S1 | initial terminal | PASS | done | 15 | 23.47 | baseline | continue |
| V002 | V001 | larger large-D chunk | streaming chunk | PASS | done | 15 | 25.50 | T15 improved | continue |
| V003 | V002 | aligned 32B DataCopy | DataCopy full 32B | PASS | done | 15 | 25.74 | T14 118631 us | continue |
| V004 | V003 | enlarge chunk via remove xFloat | buffer/chunk | PASS | done | 15 | 25.56 | T14 regressed 124886 | keep V003 insight |
| V005 | V004 | reuse invRms across chunks | invRms reuse | PASS | done | 15 | 25.97 | modest | continue |
| V006 | V005 | single-slot queues + larger chunks | queue/chunk | PASS | done | 15 | 27.30 | smaller-D improved | continue |
| V007 | V006 | direct x into u | convert path | PASS | done | 15 | 27.24 | flat | continue |
| V008 | V007 | squareFloat residual + chunks | residual convert | PASS | done | 15 | 28.10 | T14 118184 | early Fresh Best |
| V009 | V008 | retained-u output-GM wide | FP32 D>=16384 path | PASS | done | 15 | 27.72 | T14 124619 | REGRESSION; base on V008 |
| V010 | V008 | remove second x+r read; param cache; D-split when R small | full-u UB + 2D split | PASS | done | 15 | 33.25 | T14 53177 (-55%), T04 23.66, T07 69.75 | architecture validated |
| V011 | V010 | small FastKernel + single GetValue + DB | FastKernel + DB | PASS | done | 3 | — | T01 WA 100%, T05 RE | isolate next |
| V012 | V010 | FastKernel param-once + safe gate | FastKernel sync + fallback | PASS | done | 15 | 34.38 | T01 3.97 T02 3.80 | continue |
| V013 | V012 | widen FastKernel 2M elems | gate 2M | PASS | done | 4 | — | T05 RE again | tighten gate |
| V014 | V012 | param-once + tight gate | D<=4096 && R*D<=262144 && fastUb<=160k | PASS | done | 15 | 35.89 | T04 18.53 | continue |
| V015 | V014 | Resident single-GetValue + DB + D-split | three deltas together | PASS | done | 4 | — | T05 RE | one delta per run |
| V016 | V014 | single GetValue only | BuildUKeepSum + InvRmsFromSum | PASS | done | 15 | 36.35 | T14 50830 | green isolation |
| V017 | V016 | x/res 2-slot issue-ahead | double-buffer only | PASS | done | 15 | 36.41 | T14 50050 | FRESH BEST |
| V018 | V017 | host-only wideFew D-split | host dispatch only | PASS | NOT_SUBMITTED | — | — | unfinished at park | park |

## Unfinished at park
- V018 compiled ONLINE_READY (`submission_v018.asc`, commit `d7cac20`) but not submitted to CANNJudge.
