# ROW-OCC-H1 V001 Source Audit

ROUTE=ROW-OCCUPANCY-CHAMPION-X
HYPOTHESIS_ID=ROW-OCC-H1-BOUNDED-LOGICAL-BLOCKS
DIRECT_PARENT=R31B/V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_OFFICIAL_SCORE=45.16
AUDIT_STATUS=PASS
ROW_OCC_H1_GATE=PASS
AUDIT_DATE=2026-10-05

## Source identity

The audited parent is the exact retained `线上结果/R31B/V011/submission.asc`.
Its SHA-256 is
`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`,
matching `source-meta.json` and the frozen canonical anchor record. No older
R31B source or Local-best source is used as the parent.

## Exact implementation site

`run_kernel` in `线上结果/R31B/V011/submission.asc` is the host-side launch
entry at lines 3479-3554. The current launch policy is:

```text
requestedBlocks = availableCoreNum > 0 ? availableCoreNum : 1
requestedBlocks = min(requestedBlocks, rowCount)
blockCount = uint32(requestedBlocks)
```

The exact H1 change is limited to this launch-width calculation. It will
boundedly choose `min(rowCount, 2 * max(availableCoreNum, 1))`, with the
existing UINT32 clamp retained. No kernel ABI or launch arguments change.

## Reached device path

`AddRmsNormBiasKernel<T>::Process` at lines 159-226 receives `blockCount` and
uses `GetBlockIdx()` plus the contiguous `baseRows/extraRows` partition at
lines 171-176 for the generic path. The wide helpers independently repeat the
same partition with the same `blockCount` input at lines 1856-1865,
2082-2093, 2248-2259, 2383-2392, and the later FP16/BF16 wide helpers.

The dispatch and computation paths remain unchanged:

- generic, narrow/mid, and wide mode predicates are unchanged;
- `baseRows`, `extraRows`, `beginRow`, and `localRows` remain the parent
  contiguous row-unit ownership rule;
- UB allocation and lifetime are unchanged;
- MTE2/MTE3 transfers and event dependencies are unchanged;
- vector arithmetic, reduction order, dtype selection, and output stores are
  unchanged.

The only intended hardware-visible difference is the number of logical blocks
submitted to the device. Physical core count remains the runtime's
`availableCoreNum`; extra logical blocks are scheduled in waves by the device.

## Current behavior -> one change -> expected effect

`one logical block per available physical core` -> `bounded two-logical-block
ceiling when rows exceed the current launch width` -> `test whether additional
static scheduling boundaries smooth row-packet startup/tail variation while
preserving the parent row ownership and arithmetic`.

This is an occupancy/launch-width hypothesis, not a row-group, tile, DMA,
reduction, store, synchronization, or parameter-residency change.

## Duplicate audit

`DUPLICATE_WITH=NONE`

The audit covered the retained R001-R029/FULL-R001-FULL-R029 records, R016,
SCHED-ROWGROUP-X, SCHED-CHAMPION-X, R31A/R31B scheduling variants, MIX-A,
Wave-1 M2 scheduling work, ASYNC, UB, ADDR, REDUCE, TILE, STORE, EPILOGUE,
and MULTIROW evidence. The closest neighbors are R016 and the SCHED routes,
but their mechanisms are D-band/group ownership, alignment, active-core
gating, or task extent policies. None tests this fixed bounded logical-block
oversubscription while retaining the parent row-unit split.

The following are explicitly out of scope for H1: 32B group ownership,
cyclic/dynamic task assignment, divisor selection, small-row under-subscription,
tile changes, reduction changes, DMA aggregation, event changes, UB reuse,
store changes, arithmetic changes, dtype policy, and parameter residency.

## Codegen and hardware gate

The parent source has already passed the frozen canonical exact-source build
and link using `NPU_ARCH=dav-2201` on `SOC=Ascend910B3`; the retained parent
executable identity is recorded in the canonical V1 build evidence. The H1
site is host-side code in the same `run_kernel` compilation unit, and the
device compiler sees the unchanged kernel body and ABI. A fresh V001 build
will be required after the one-line launch-policy change.

The local hardware reports Ascend910B3 devices and the normal HBM guard is
usable for compile/correctness scheduling (`FREE_HBM >= 100 MB` is the only
hard allocation block). Formal timing will still require an exclusive device
lease and one comparison at a time. Existing processes will not be killed,
paused, or migrated.

## Gate decision

| Gate | Result | Evidence |
|---|---|---|
| Real hot path | PASS | `run_kernel` launch policy feeds every `Process` path |
| Mechanism not already covered | PASS | duplicate audit above; `DUPLICATE_WITH=NONE` |
| Single-variable change | PASS | host launch width only |
| Math/dtype/store/parameter policy unchanged | PASS | source-site and diff boundary above |
| Correctness risk controllable | PASS | row coverage remains the parent contiguous partition |
| ROW_OCC_H1_GATE | PASS | eligible to declare V001 and implement |

No Revision was implemented before this audit. No Online action is authorized
by this document.
