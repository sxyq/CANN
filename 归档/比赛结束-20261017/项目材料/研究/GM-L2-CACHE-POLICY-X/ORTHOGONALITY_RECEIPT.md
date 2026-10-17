# ORTHOGONALITY_RECEIPT

CURRENT_PARENT
FROZEN R31B V011; source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; exact parent is `归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc`; Official 45.16, 15/15 PASS. Base commit: `ef272e8ce2424c8f70ff373498d7fea7cc336726`.

CURRENT_MAIN1_OVERLAP
No material overlap found. Main-1 CASE47/CASE14, SELECTIVE-FASTPATH, SMALLMID-DATAFLOW, STORE-EPILOGUE, EPILOGUE-ARITH, SYNC-TOPOLOGY, and TINY-FIXED-OVERHEAD operate on dispatch, data movement, UB/staging, output/store, arithmetic, or synchronization boundaries. This route is restricted to a supported global-memory cache-control or cache-observation mechanism and does not change any of those boundaries. Evidence reviewed in `调度/当前任务.tsv`, `技术路线/技术路线图.md`, and the W2/Main-1 route worktrees listed by the canonical worktree registry.

HISTORICAL_MECHANISM_OVERLAP
W3 CROSSROW-FULL-PIPELINE and MULTIROW-PANEL use cross-row scheduling, panel/UB residency, and MTE data movement; UB-BANK-LAYOUT changes local UB placement; ADAPTIVE-CORE-OWNERSHIP changes core assignment. R001-R029 and the W4 R01-R15 archive cover reduction, row scale, staging lifetime, synchronization, dispatch cutoffs, aligned DataCopy, parameter locality, and related pipeline changes. None provides a verified L2 cache-policy control for this exact direct-invocation kernel. The W4 archive and `技术路线/全项目成绩与技术路线盘点.md` were checked for cache-control claims; ordinary UB/parameter/tile changes are explicitly out of scope here.

TARGET_FUNCTIONS
`AddRmsNormBiasKernel::Process`, `ProcessWideFp32`, `ProcessWideLowPrecision`, and the existing GM load/store helpers in `归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc`. The target is only the global-memory cache behavior observable for the unchanged kernel path; no arithmetic, dispatch, UB, DMA, or synchronization function may be altered.

MUTATION_BOUNDARY
Research/feasibility evidence only until a legal, target-supported cache interface and a measurable bottleneck are proven. If enabled, one cache-policy or cache-control OFAT at the host/device interface is allowed; no UB bank adjustment, parameter residency, DataCopy/DataCopyPad change, tile-size change, core-ownership change, barrier change, or RMS/math change. No Candidate edit is authorized by this receipt alone.

INDEPENDENT_HYPOTHESIS
Ascend910B3 may expose a supported global-memory cache-control/prefetch policy, and the existing GM traffic may have an observable L2 bottleneck that can be changed without changing kernel semantics or substituting a UB/data-movement optimization. This is a capability hypothesis, not a performance claim.

ORTHOGONALITY_STATUS
PASS_FOR_RESEARCH; NO_MATERIAL_DUPLICATE_FOUND; CAPABILITY_GATE_REQUIRED. The route remains research-only until `CACHE_CAPABILITY_RECEIPT` proves an applicable supported interface, legal behavior change, observability, and a target bottleneck.
