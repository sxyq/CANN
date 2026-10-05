# Main-2 Canonical Revision Tree

Edges use exact parent-source identity and committed metadata, not revision chronology.
Git commit ancestry is a storage/rollback history; it is not automatically Kernel inheritance.
R31B/V011 is the scoring anchor for all rows; this does not change their Direct Parents.

```text
PURE-R009-V001-ALIGNED-DATACOPY
  └── ALIGN-TAIL-X/V001 [f573d16d39fb]
R31B/V011
  └── ASYNC-OVERLAP-CHAMPION-X/V001 [0fc71e8e62e3]
  └── ASYNC-OVERLAP-CHAMPION-X/V002 [f8f65e09570e]
  └── ASYNC-OVERLAP-CHAMPION-X/V003 [d23593ce88fd]
  └── ASYNC-OVERLAP-CHAMPION-X/V004 [1a0b6e5841ef]
  └── COEFF-LOCALITY-X/V001 [6e47a9a0ad50]
  └── COEFF-LOCALITY-X/V003 [4be7c2287e8e]
  └── COEFF-LOCALITY-X/V004 [848165b62159]
  └── DTYPE-SPECIAL-X/V001 [e2717055199f]
  └── EPILOGUE-FUSE-X/V001 [89868a52b59f]
  └── EPILOGUE-FUSE-X/V002 [3d10417497b3]
  └── EPILOGUE-FUSE-X/V003 [1f94721a3671]
  └── HOTLOOP-ADDR-HOIST-CHAMPION-X/V001 [26aa65a2e131]
  └── HOTLOOP-ADDR-HOIST-CHAMPION-X/V002 [40b1548eb086]
  └── HOTLOOP-BRANCH-HOIST-CHAMPION-X/V001 [4dc1973ef198]
  └── HOTLOOP-BRANCH-HOIST-CHAMPION-X/V002 [1aa6d8ecaa1a]
  └── INTEGRATION-X/V001 [52f3a329703e]
  └── MULTIROW-DMA-CHAMPION-X/V001 [5dea0eaf1752]
  └── MULTIROW-DMA-CHAMPION-X/V002 [2c23ce327524]
  └── REDUCE-FINALIZE-HANDOFF-CHAMPION-X/V001 [83d569d2968b]
  └── REDUCE-HIER-X/V001 [b9c618b3b53f]
  └── REDUCE-HIER-X/V002 [e4a80f182758]
  └── REDUCE-HIER-X/V003 [79f910e4450a]
  └── REDUCE-HIER-X/V004 [1926a2f91491]
  └── REDUCE-HIER-X/V005 [5fa325263d18]
  └── SCHED-CHAMPION-X/V001 [ed232872fa18]
  └── SCHED-CHAMPION-X/V002 [de1e93c74338]
  └──     └── SCHED-CHAMPION-X/V003 [d2db679827e4]
  └── STORE-EPILOGUE-X/V001 [06564134e493]
  └── STORE-EPILOGUE-X/V002 [59fb8eada4da]
  └──     └── STORE-EPILOGUE-X/V003 [0cdef265459d]
  └── UB-LIFETIME-SAFE-CHAMPION-X/V001 [9b73bb5626b5]
  └── VECTOR-MATH-X/V001 [dbe776f9165a]
  └──     └── VECTOR-MATH-X/V002 [06095762d6ff]
  └── VECTOR-MATH-X/V003 [f019d099973b]
ASYNC-TRIPLE-X-SEED (fresh route seed from canonical ae46d7c; R013-derived two-stage pipeline)
  └── ASYNC-TRIPLE-X/V001 [2defc6c270e8]
A001-V017 (R014 parameter-residency verified checkpoint / A001-style param cache)
  └── BATCH-RESIDENT-X/V001 [ad961c583797]
FULL-R006-V001-REDUCTION-ARCH
  └── REDUCE-INVSCALE-X/V001 [f017935d840b]
  └──     └── REDUCE-INVSCALE-X/V002 [bef271b62a2c]
R016-V001-COMPILEFIX-A-SCHEDULING-ARCH
  └── SCHED-ROWGROUP-X/V001 [0fae0a42e394]
NONE (fresh route; PARENT_SEED ae46d7c; PARENT_UNRESOLVED=true)
  └── UB-LIVENESS-X/V001 [1a6ae30a5eff]
  └──     └── UB-LIVENESS-X/V002 [888bd60c1efb]
  └──     └──     └── UB-LIVENESS-X/V003 [2eb9b5d08726]
```

## Research-only / declared but not implemented

- COEFF-LOCALITY-X/V002: RESEARCH_ONLY_NO_IMPLEMENTATION; /home/data4t2/lelinfeng/cann/worktrees/w2/m2/hotloop-addr/本地实验/COEFF-LOCALITY-X/V002/local-result.json;/home/data4t2/lelinfeng/cann-w2-m1-epi/本地实验/COEFF-LOCALITY-X/V002/NO_UB_BUDGET.md;/home/data4t2/lelinfeng/cann-w2-main2-control/技术路线/全版本记录.tsv
- CROSSROW-PIPELINE-CHAMPION-X/NONE: RESEARCH_ONLY_NO_IMPLEMENTATION; 研究/主代理/MAIN-2-W2/campaign-status.md
- INTERPASS-PIPELINE-CHAMPION-X/NONE: RESEARCH_ONLY_NO_IMPLEMENTATION; 研究/主代理/MAIN-2-W2/campaign-status.md
- PARAM-RESIDENCY-CHAMPION-X/NONE: RESEARCH_ONLY_NO_IMPLEMENTATION; 研究/主代理/MAIN-2-W2/campaign-status.md
- ROW-OCCUPANCY-CHAMPION-X/NONE: RESEARCH_ONLY_NO_IMPLEMENTATION; 研究/主代理/MAIN-2-W2/campaign-status.md
- TILECOUNT-STATIC-UNROLL-CHAMPION-X/NONE: RESEARCH_ONLY_NO_IMPLEMENTATION; 研究/主代理/MAIN-2/CAMPAIGN-STATUS.md;研究/主代理/MAIN-2-W2/campaign-status.md
- UB-CHAMPION-X/NONE: RESEARCH_ONLY_NO_IMPLEMENTATION; 归档/历史控制文件/main2-r2-route-registry.md

## Source and parent audit

Full SHAs, original source locations, Git objects and evidence links: `MAIN2-CANONICAL-ROUTE-REGISTRY.tsv`.
Corrections to the older inventory: `../本地实验/MAIN2-CANONICAL-V1/census/corrections.tsv`.
UNKNOWN parents remain unknown; source copies do not invent ancestry. No new performance Revision was created.
