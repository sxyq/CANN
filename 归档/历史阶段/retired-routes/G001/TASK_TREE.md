# G001 Task Tree

Evidence: `phase4/control/results.tsv`, `phase4/online/G001/*/result.json`, Route Agent handoffs, Git commits.

```
G001 baseline (Fresh One-Read / Resident-Y)
└─ V001 6/15 WA calc 9.95  Main-authored integration (ABI + scalar sqrt)
    └─ V002 11/15 WA calc 9.69  FP32 intermediate + BF16 bitcast + judge dtype  ← BEST CORRECTNESS
        ├─ V003 7/15  native-u quantize + invRms  REGRESSION (T12/T14/T15 ~20% err)
        └─ V004 10/15  restore V002 math + RNE output cast
            └─ V005 6/15  native cast before bias  REGRESSION (T12/T14/T15 ~20%)
                └─ V006 9/15  gamma order u*gamma/rms
                    └─ V007 6/15  u/rms intermediate cast  REGRESSION (T12/T14/T15 ~20%)
                        └─ V008 10/15  pairwise sumSquares
                            └─ V009 9/15  restore V002 math + gamma/bias UB cache  (T11/T13 0.15% WA)
```

## Node fields

| rev | parent | hypothesis | main change | compile | online | pass | score | important delta | decision |
|-----|--------|------------|-------------|---------|--------|------|-------|-----------------|----------|
| V001 | init | one-read resident y | resident u row + Main ABI/sqrt integration | PASS | done | 6 | calc 9.95 | T12/T14/T15 err 100%; T02/T04/T06/T07 2–6% | correctness first |
| V002 | V001 | FP32 math + BF16 bitcast + judge dtype | FromFloat>>16; FP32 u; encoding 0=FP32/1=FP16/2=BF16 | PASS | done | 11 | calc 9.69 | T12/T14/T15 Pass; remaining T02/T04/T06/T07/T11 | BEST CORRECTNESS |
| V003 | V002 | native-u quantize + invRms matches golden | AddNativeU + invRms chain | PASS | done | 7 | calc 10.12 | T12/T14/T15 fail 18–21% | revert class |
| V004 | V002 math | RNE output cast | explicit RNE FromFloat | PASS | done | 10 | calc 9.51 | T11 0.011% WA nick | not the 2–6% fix |
| V005 | V002 | native cast of norm before bias | NormPlusBias | PASS | done | 6 | calc 9.58 | T12/T14/T15 ~20% again | revert class |
| V006 | V002 | gamma apply order | u*gamma/rms+bias | PASS | done | 9 | calc 9.49 | T02/T04/T06/T07 frozen | neutral |
| V007 | V002 | intermediate cast of u/rms | T(T(u/rms)*gamma+bias) | PASS | done | 6 | calc 9.59 | large-D ~20% again | revert class |
| V008 | V002 | pairwise reduction | binary-counter sumSquares | PASS | done | 10 | calc 9.17 | T02/T04/T06/T07 frozen; T11 nick | levers exhausted |
| V009 | V002 | restore math + param cache for speed | V002 arithmetic + gamma/bias UB cache | PASS | done | 9 | calc 9.51 | T11/T13 0.15% WA; T02/T04/T06/T07 still 2–6% | PARK |

## Unfinished at park
- None required for loop closure. V009 is last online revision. Scalar GetValue family not continued.

## Notes
- Original V002 file is the correctness reference (11/15). Subsequent numerical A/B never improved T02/T04/T06/T07 and several broke T12/T14/T15.
