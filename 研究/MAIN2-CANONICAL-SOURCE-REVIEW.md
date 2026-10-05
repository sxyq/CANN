# Main-2 Canonical source-level review

Scope: five highest-scored Candidate revisions (anchor excluded) and the best representative of each of the three highest-scored routes.
The numeric order is a view of the canonical scoreboard, not causal evidence or a new Parent selection.
Method: exact source/parent diff, reached path, producer/consumer lifetime, and the fixed Core vector. No Kernel was edited.
Directive priority: the task requires this written planning evidence; the quick-review skill is used for its targeted review method only.

## Interpretation boundary

All listed ratios are Candidate/frozen-fresh-V011 latency; <1 is a numerical gain. Paired-parent drift and POOR quality prevent reliable claims.
An unchanged reached path cannot be credited with the advertised mechanism solely from a score difference.
This is not a comprehensive API/correctness certification. Seven-case preflight and historical global failures remain separately recorded.

## Reviews

### ASYNC-OVERLAP-CHAMPION-X/V001

- VERSION: ASYNC-OVERLAP-CHAMPION-X/V001
- SOURCE_SHA: 0fc71e8e62e39ed3b894b7e8fb8cabb6372f802c0a4c342eeb246629f8e4efe4
- EXACT_FUNCTION: AddRmsNormBiasKernel<T>::ProcessWideLowPrecision
- EXACT_CODE_SITE: submission.asc:3224-3267; pass-2 prologue and tile loop
- WHAT_CHANGED: Hoisted pass-2 gamma/bias tile-0 Load and MTE2_V ready flag before the invRms scalar loop; removed the old late SyncVToMTE2 and duplicate tile-0 Load.
- WHY_IT_HELPS: Only if parameter DMA can overlap the scalar normalization tail; DMA bytes, tile geometry and arithmetic are unchanged.
- HOT_PATH_SCOPE: Wide FP16/BF16 retained-y path; once per retained batch, tile-0 only.
- DMA_EFFECT: Same gamma/bias transactions and bytes; issue point moves earlier.
- SYNC_EFFECT: MTE2_V and V_MTE2 event lifecycle crosses the reduction tail; removed one inter-pass helper wait.
- UB_EFFECT: Reuses xBuf_/residualBuf_ after pass-1 release; no allocation change.
- VECTOR_EFFECT: None intended; invRms and affine order unchanged.
- STORE_EFFECT: Unchanged.
- SINGLE_CHANGE_AUDIT: PASS
- REVIEW_CONCLUSION: Mechanism is real and distinct from generic parameter residency, but numerical leader is fragile: Core ratios C12=1.030395, C13=0.955769, C14=0.976117, C16=1.010624; 3/4 Core cases POOR and paired drift is not a reliable gain.
- DIRECT_PARENT: R31B/V011
- PARENT_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- WHICH_CORE_CASES_GAIN: C13,C14
- WHICH_CORE_CASES_REGRESS: C12,C16
- CORE_RATIOS: C12=1.030395;C13=0.955769;C14=0.976117;C16=1.010624
- LOCAL_SCORE: 100.725142
- RELIABLE_LOCAL_GAIN: NO
- DIFF_PATH: 研究/MAIN2-CANONICAL-SOURCE-REVIEW/diffs/ASYNC-OVERLAP-CHAMPION-X__V001.patch

| Core case | Anchor us | Candidate us | C/A ratio | Paired-parent drift | Quality | Without this case score |
|---|---:|---:|---:|---:|---|---:|
| C12 | 6.580000 | 6.780000 | 1.030395 | 1.009119 | POOR | 101.980808 |
| C13 | 20.800000 | 19.880000 | 0.955769 | 0.983654 | GOOD | 99.456888 |
| C14 | 25.960000 | 25.340000 | 0.976117 | 0.974576 | POOR | 100.157734 |
| C16 | 15.060000 | 15.220000 | 1.010624 | 1.021248 | POOR | 101.324331 |

### ASYNC-OVERLAP-CHAMPION-X/V003

- VERSION: ASYNC-OVERLAP-CHAMPION-X/V003
- SOURCE_SHA: d23593ce88fd7011a604f50cfba3948904f4f97f66641346626143dd16af3548
- EXACT_FUNCTION: AddRmsNormBiasKernel<T>::ProcessNarrowMidOverlap
- EXACT_CODE_SITE: submission.asc:561-574
- WHAT_CHANGED: Replaced two V/S round trips for invRms tail with scalar std::sqrt after the first SyncVToS.
- WHY_IT_HELPS: May remove one vector broadcast, one vector sqrt and one V/S round trip in the narrow/mid path.
- HOT_PATH_SCOPE: Narrow/mid overlap path only; not wide and not generic all-shape code.
- DMA_EFFECT: None.
- SYNC_EFFECT: V/S handoff count decreases from two round trips to one.
- UB_EFFECT: No allocation or lifetime change.
- VECTOR_EFFECT: Vector Duplicate/Sqrt removed from this tail; scalar sqrt added.
- STORE_EFFECT: Unchanged.
- SINGLE_CHANGE_AUDIT: PASS
- REVIEW_CONCLUSION: Clean arithmetic/synchronization OFAT, but fixed suite is outside its principal narrow/mid target, so the 99.759996 score is not evidence for this mechanism; C12 is neutral and wide cases regress or are unchanged.
- DIRECT_PARENT: R31B/V011
- PARENT_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- WHICH_CORE_CASES_GAIN: C13
- WHICH_CORE_CASES_REGRESS: C14,C16
- CORE_RATIOS: C12=1.000000;C13=0.994231;C14=1.006163;C16=1.009296
- LOCAL_SCORE: 99.759996
- RELIABLE_LOCAL_GAIN: NO
- DIFF_PATH: 研究/MAIN2-CANONICAL-SOURCE-REVIEW/diffs/ASYNC-OVERLAP-CHAMPION-X__V003.patch

| Core case | Anchor us | Candidate us | C/A ratio | Paired-parent drift | Quality | Without this case score |
|---|---:|---:|---:|---:|---|---:|
| C12 | 6.580000 | 6.580000 | 1.000000 | 1.012158 | GOOD | 99.680123 |
| C13 | 20.800000 | 20.680000 | 0.994231 | 1.003846 | GOOD | 99.488061 |
| C14 | 25.960000 | 26.120000 | 1.006163 | 1.000000 | GOOD | 99.884491 |
| C16 | 15.060000 | 15.200000 | 1.009296 | 1.011952 | POOR | 99.988052 |

### EPILOGUE-FUSE-X/V001

- VERSION: EPILOGUE-FUSE-X/V001
- SOURCE_SHA: 89868a52b59fcaf1d68220398698ef033827a50b1559df9fc69ffcab105dd597
- EXACT_FUNCTION: AddRmsNormBiasKernel<T>::ApplyScaleFold and all affine call sites
- EXACT_CODE_SITE: submission.asc:423-468, 575-605, 3388-3410
- WHAT_CHANGED: Replaced value*invRms followed by gamma multiply with gamma*invRms temporary then value multiply; bias add remains.
- WHY_IT_HELPS: Attempts to reduce the affine sequence or expose a fused scale operation while preserving algebraic intent.
- HOT_PATH_SCOPE: Generic, narrow/mid, batched and wide affine paths; broad source blast radius.
- DMA_EFFECT: None.
- SYNC_EFFECT: Adds/retains vector barriers inside ApplyScaleFold; no event topology change.
- UB_EFFECT: Consumes reduceFp32Buf_ as scale staging; no declared allocation increase.
- VECTOR_EFFECT: Adds Muls(gamma, invRms), then Mul(value, scale), then Add bias.
- STORE_EFFECT: Unchanged.
- SINGLE_CHANGE_AUDIT: PASS
- REVIEW_CONCLUSION: Most coherent high-ranked arithmetic candidate, but score 99.626812 has C12 regression 1.057751 and only C13/C14 improvement; no reliable local gain. Broad path coverage increases regression risk.
- DIRECT_PARENT: R31B/V011
- PARENT_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- WHICH_CORE_CASES_GAIN: C13,C14,C16
- WHICH_CORE_CASES_REGRESS: C12
- CORE_RATIOS: C12=1.057751;C13=0.975962;C14=0.984592;C16=0.998672
- LOCAL_SCORE: 99.626812
- RELIABLE_LOCAL_GAIN: NO
- DIFF_PATH: 研究/MAIN2-CANONICAL-SOURCE-REVIEW/diffs/EPILOGUE-FUSE-X__V001.patch

| Core case | Anchor us | Candidate us | C/A ratio | Paired-parent drift | Quality | Without this case score |
|---|---:|---:|---:|---:|---|---:|
| C12 | 6.580000 | 6.960000 | 1.057751 | 1.042553 | FAIR | 101.382444 |
| C13 | 20.800000 | 20.300000 | 0.975962 | 0.987500 | GOOD | 98.698952 |
| C14 | 25.960000 | 25.560000 | 0.984592 | 0.993837 | GOOD | 98.989021 |
| C16 | 15.060000 | 15.040000 | 0.998672 | 1.015936 | POOR | 99.458658 |

### SCHED-CHAMPION-X/V002

- VERSION: SCHED-CHAMPION-X/V002
- SOURCE_SHA: de1e93c74338802e646258ada08a8cac1031496a9985e585a5a89015b81bc990
- EXACT_FUNCTION: AddRmsNormBiasKernel<T>::Process
- EXACT_CODE_SITE: submission.asc:171-218 and narrow BF16 release sequence near 635
- WHAT_CHANGED: Added 32B row-group ownership with a half-parallelism gate; also moved a BF16 MTE3 drain before V_MTE2 release in the same source.
- WHY_IT_HELPS: The row-group gate can preserve active-core parallelism while making ownership byte-aligned; the drain prevents reuse of an output source before store completion.
- HOT_PATH_SCOPE: Narrow dispatch ownership plus a BF16 output/store lifetime path; not a single isolated mechanism in the byte diff.
- DMA_EFFECT: Row ownership changes timing only; BF16 store DMA ordering is preserved and fenced.
- SYNC_EFFECT: Adds SyncMTE3ToV before releasing the input buffer in the BF16 branch.
- UB_EFFECT: No size change; safer source lifetime for BF16 output alias.
- VECTOR_EFFECT: No arithmetic change.
- STORE_EFFECT: Store order/lifetime changed in BF16 narrow path.
- SINGLE_CHANGE_AUDIT: REVIEW_REQUIRED_SOURCE_HAS_ADJACENT_CORRECTNESS_FIX
- REVIEW_CONCLUSION: 99.335036 is numerically near anchor but all four Core ratios are >1 and 3 Core cases are POOR. The score does not establish a scheduling gain; route should remain a planning recommendation only.
- DIRECT_PARENT: R31B/V011
- PARENT_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- WHICH_CORE_CASES_GAIN: NONE
- WHICH_CORE_CASES_REGRESS: C12,C13,C14,C16
- CORE_RATIOS: C12=1.006079;C13=1.003846;C14=1.011556;C16=1.005312
- LOCAL_SCORE: 99.335036
- RELIABLE_LOCAL_GAIN: NO
- DIFF_PATH: 研究/MAIN2-CANONICAL-SOURCE-REVIEW/diffs/SCHED-CHAMPION-X__V002.patch

| Core case | Anchor us | Candidate us | C/A ratio | Paired-parent drift | Quality | Without this case score |
|---|---:|---:|---:|---:|---|---:|
| C12 | 6.580000 | 6.620000 | 1.006079 | 1.021277 | GOOD | 99.314799 |
| C13 | 20.800000 | 20.880000 | 1.003846 | 1.013462 | POOR | 99.241272 |
| C14 | 25.960000 | 26.260000 | 1.011556 | 1.010786 | POOR | 99.494700 |
| C16 | 15.060000 | 15.140000 | 1.005312 | 1.001328 | POOR | 99.289556 |

### HOTLOOP-ADDR-HOIST-CHAMPION-X/V002

- VERSION: HOTLOOP-ADDR-HOIST-CHAMPION-X/V002
- SOURCE_SHA: 40b1548eb0862691e92b10f242e3f98ccf4d232596a2704ddbc25eab7b3aa3f3
- EXACT_FUNCTION: AddRmsNormBiasKernel<T>::ProcessWideLowPrecision
- EXACT_CODE_SITE: submission.asc:3125-3212
- WHAT_CHANGED: Introduced a rolling rowBase for rowOffset and next-row offset calculation; tile, valid and GM address semantics remain explicit.
- WHY_IT_HELPS: May remove repeated batchBegin+batchRow multiplication in the wide pass-1 loop.
- HOT_PATH_SCOPE: Wide FP16/BF16 pass-1, once per (row,tile) unit.
- DMA_EFFECT: Same transfers and valid lengths; only address arithmetic changes.
- SYNC_EFFECT: None.
- UB_EFFECT: None.
- VECTOR_EFFECT: None.
- STORE_EFFECT: None.
- SINGLE_CHANGE_AUDIT: PASS
- REVIEW_CONCLUSION: Canonical score 99.256484; C12/C14 regress, C13 is near neutral, C16 is slightly faster. Source mechanism is plausible but no consistent Core gain.
- DIRECT_PARENT: R31B/V011
- PARENT_SHA: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- WHICH_CORE_CASES_GAIN: C13,C16
- WHICH_CORE_CASES_REGRESS: C12,C14
- CORE_RATIOS: C12=1.033435;C13=0.997115;C14=1.003852;C16=0.996016
- LOCAL_SCORE: 99.256484
- RELIABLE_LOCAL_GAIN: NO
- DIFF_PATH: 研究/MAIN2-CANONICAL-SOURCE-REVIEW/diffs/HOTLOOP-ADDR-HOIST-CHAMPION-X__V002.patch

| Core case | Anchor us | Candidate us | C/A ratio | Paired-parent drift | Quality | Without this case score |
|---|---:|---:|---:|---:|---|---:|
| C12 | 6.580000 | 6.800000 | 1.033435 | 1.060790 | POOR | 100.101255 |
| C13 | 20.800000 | 20.740000 | 0.997115 | 0.998077 | GOOD | 98.914583 |
| C14 | 25.960000 | 26.060000 | 1.003852 | 0.993066 | GOOD | 99.136845 |
| C16 | 15.060000 | 15.000000 | 0.996016 | 1.014608 | POOR | 98.878214 |
