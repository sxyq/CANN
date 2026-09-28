# I001 Task Tree

Evidence: `phase4/control/results.tsv`, `phase4/online/I001/*/result.json`, Route Agent handoffs, Git commits.

```
I001 baseline (Fresh Wide-D Specialist) — workspace created empty
└─ V001 CE  first wide-D implementation  online CE
    └─ V002 CE  judge-template rewrite  still online CE
        └─ V003 RE 10/15 ran  ABI-correct template two-pass  FIRST ONLINE RUN
            └─ V004 15/15 15.53  unaligned pad + ReduceSum tmp  ← FIRST / BEST FULL SCORE
                └─ V005 WA 13/15 calc 25.3  retained-y + large-tile + D-split  (T02/T03 ~100% WA)
                    └─ V006 WA 13/15 calc 25.3  padded uKeep slack  (T02/T03 still WA)
                        └─ V007 ONLINE_READY not submitted  tiny D<256 forced to V004 two-pass
```

## Node fields

| rev | parent | hypothesis | main change | compile | online | pass | score | important delta | decision |
|-----|--------|------------|-------------|---------|--------|------|-------|-----------------|----------|
| V001 | init | retained-y / D-stripe wide | first kernel.asc family | local PASS | CE | 0 | — | online CE | template |
| V002 | V001 | self-contained template ABI | no acl.h; GM_ADDR ABI | local PASS | CE | 0 | — | still CE | ABI mismatch |
| V003 | V002 | judge run_kernel ABI exact | GM_ADDR + const TensorGroupInfo& + int64_t + aclrtStream | local PASS | done | 10 | — | T02/T03/T11 WA; T13 RE | first run |
| V004 | V003 | unaligned DataCopyPad + ReduceSum tmp | rightPadding; 8KiB tmp; store n | local PASS | done | 15 | 15.53 | FIRST 15/15 | correctness baseline |
| V005 | V004 | retained-y + large tile + D-split | three wide modes | local PASS | done | 13 | calc 25.34 | T02/T03 ~100% WA; huge speedups T09 185 T11 258 T15 14323 | fix tiny WA |
| V006 | V005 | uKeep sized to padded workN | +32 float slack | local PASS | done | 13 | calc 25.34 | T02/T03 still WA | hypothesis incomplete |
| V007 | V006 | tiny D keep-path broken | force D<256 to V004 two-pass | local PASS | NOT_SUBMITTED | — | — | unfinished | park |

## Unfinished at park
- V007 compiled ONLINE_READY (`logs/build_summary_V007_20260922_163550.log`), not submitted.

## Wide T14 gap
- I001-V004 T14 ~120974 us vs best 3750 (r≈32). V005 T14 still ~118321. Massive gap vs R31 Champion T14 ~16400 and vs best 3750. Wide specialist did not close T14 at park.
