# TINY-FIXED-OVERHEAD-CHAMPION-X V001

STATUS: `MAIN_SELECTED=YES`; `SOURCE_WRITTEN=YES`
ROUTE: `TINY-FIXED-OVERHEAD-CHAMPION-X`
REVISION: `V001`
CONTEXT_CLASS: `HISTORICAL_DERIVED`
DIRECT_PARENT: `R31B-V011`
PARENT_SOURCE_PATH: `线上结果/R31B/V011/submission.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
PARENT_OFFICIAL_SCORE: `45.16`; result `15/15 PASS`
PLANNING_SELECTION: `TINY-H2-ROW-OWNERSHIP-FASTFORM`

## Single Hypothesis

Only when `rowCount == blockCount`, assign:

```cpp
beginRow = blockIdx;
localRows = 1;
```

For every other input, retain the exact Parent ownership formula. Do not change `blockCount`, core count, path dispatch, buffers, arithmetic, DMA, or synchronization.

## Why Distinct

R016 changes D-banded rows-per-task and launch block scheduling. SCHED-ROWGROUP-X adds 32-byte row-group ownership and removes its unaligned single-core rule. V001 retains Parent launch dimensions and task ownership; it only removes quotient/remainder ownership arithmetic in the equality case. It is falsified if the target compiler already emits the direct mapping or if the candidate does not reduce device instructions.

## Proxy Correctness Cases

All source/test labels must include `PROXY`. No proxy asserts Official case 1/3/5 shape, dtype, or dispatch.

Read live vector-core count `A` after device setup through the existing `ACL_DEV_ATTR_VECTOR_CORE_NUM` runtime API. Define `M=max(2,floor(A/2))` and `B=M-1`. If `A < 2` or `M > A`, report that the proxy pair cannot be formed and stop before correctness.

| Label | Shape and dtype | Wrapper `availableCoreNum` | Expected ownership branch |
|---|---|---:|---|
| `PROXY-TINY-V001-EQUAL-FASTFORM` | `[M,256]`, FP32 | `A` | `rowCount=M`, `blockCount=M`; H2 fastform |
| `PROXY-TINY-V001-NON-EQUAL-PARENT-FALLBACK` | Same `[M,256]`, FP32; reuse the same x/residual/gamma/bias allocations | `B=M-1` (explicit cap) | `rowCount=M`, `blockCount=B`; exact Parent formula fallback |

The runner must report numeric `A`, `M`, `B`, the toolchain, and each branch condition reached. Both labels and the source label must contain `PROXY`. These are legal research proxies, not Official case 1/3/5 inputs. Their correctness runs use the same allocated tensors; the explicit cap changes launch width, so their timings cannot be compared as evidence for V001. Any performance measurement requires Main's timing lease and a Parent comparison with the same launch cap. The standalone V011-derived test does not establish whether this proxy is selected by SELECTIVE-FASTPATH H3 or whether any Official case overlaps its allowlist.

## Execution Order

1. Copy the exact Parent source and apply only the ownership fast form plus a `PROXY` source label.
2. Review the source diff and commit the source and this declaration before Build.
3. After Main preflight/assignment, proceed with Build and the two proxy Correctness cases only if one device has `FREE_HBM >= 100 MB` and server job limits permit. Record source/executable identities and full results in this V001 directory.
4. Do not run same-binary or Parent/Candidate timing without Main's timing lease.
5. Do not submit Online or edit shared ledgers.
