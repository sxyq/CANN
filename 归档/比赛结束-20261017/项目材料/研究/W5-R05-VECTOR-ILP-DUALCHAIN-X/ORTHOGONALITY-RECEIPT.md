# W5-R05 VECTOR-ILP-DUALCHAIN-X Orthogonality Receipt

```text
AGENT_ID = W5-R05-VECTOR-ILP-DUALCHAIN-X-ROUTE-AGENT
ROUTE = W5-R05 VECTOR-ILP-DUALCHAIN-X
WORKTREE = /home/data4t2/lelinfeng/cann-w5-r05-vector-ilp
BRANCH = research/w5-r05-vector-ilp
DATE = 2026-10-09
STAGE = research receipt; no Candidate Revision created
ONLINE = NO
CHAMPION_KERNEL_MIGRATION = NO
```

## Mission

Run one controlled micro-experiment that compares equal-compute dependent Vector
instruction chains with two independent Vector chains. The only intended
variable is Vector dependency topology / available instruction-level parallelism
(ILP). Memory traffic, synchronization, dtype, shape, and timing boundary must
remain identical. This receipt is the design and de-duplication gate; it does
not claim a performance result.

## Evidence consulted

- `研究/EPILOGUE-FUSE-X/VECTOR-MATH-PROPOSAL.md` defines VECTOR-MATH as
  denominator arithmetic (`invD` / mean / epsilon / sqrt / rsqrt / reciprocal)
  and explicitly excludes scheduling, DMA, and wide-path changes.
- `本地实验/VECTOR-MATH-X/V001/source-meta.json` records the tested scalar
  `meanSquare` to vector `Muls+Adds` change and says reduction, scheduling,
  DMA, dtype, and epilogue-store changes were excluded.
- `本地实验/VECTOR-MATH-X/V002/source-meta.json` records the tested scalar
  broadcast `Muls` to `Duplicate+Mul` change and says the V-side `Div` was
  reverted after synchronization errors; this route will not reuse either
  arithmetic or broadcast change.
- `研究/主代理/MAIN-1/ONLINE-RECOMMENDATION-R31A-V028.md` records a separate
  two-`PipeBarrier` deletion and the finding that a pre-`SetFlag` barrier was
  non-load-bearing. R05 will retain the parent barrier count and placement.
- `技术路线/全项目成绩与技术路线盘点.md` identifies R013 / `FULL-R013` as
  MTE2/V/MTE3 double buffering and labels double buffering only partial
  evidence. R05 will not change DMA staging, buffer count, or overlap.
- `研究/主代理/MAIN-1/campaign-status.md` defines Main-1 lane boundaries for
  store/writeback, affine/Rsqrt/path selection, shape-conditioned tiling, RMS
  post-store arithmetic, and Champion-path microarchitecture. R05 changes none
  of those mechanisms.

## Orthogonality matrix

| Existing mechanism | Existing change | R05 boundary | Isolation verdict |
|---|---|---|---|
| VECTOR-MATH-X | Denominator arithmetic, scalar/vector handoffs, or broadcast form | Keep the production arithmetic formula and handoffs unchanged; vary only dependency topology in an equal-op micro-sequence | Orthogonal |
| Barrier deletion | Removes selected `PipeBarrier` operations | Keep identical barrier instructions, locations, and ordering in both variants | Orthogonal |
| DMA double-buffer | Changes MTE2/MTE3 staging and overlap | Keep identical GM-to-UB / UB-to-GM requests, bytes, buffer count, and copy order | Orthogonal |
| Main-1 store/writeback | Output transaction and store drain behavior | Same output transaction and store boundary | Orthogonal |
| Main-1 affine/Rsqrt/path selection | Address, path, or approximate-math selection | Same addresses, path selection, and exact arithmetic; no Rsqrt substitution | Orthogonal |
| Main-1 shape/tiling | Tile size, shape dispatch, or UB tradeoff | Same shape, tile, block mapping, and UB allocation | Orthogonal |
| Main-1 epilogue arithmetic | RMS-to-store arithmetic ordering/fusion | No production epilogue rewrite; micro-sequence uses equal-compute stand-ins only | Orthogonal |

## Controlled experiment contract

Both variants must use the same existing project harness/runner and the same
kernel ABI. They must have the same:

- shape, dtype, block/core mapping, input/output allocation, and UB allocation;
- GM-to-UB and UB-to-GM transaction count and byte volume;
- Vector instruction count and operation classes, with only source dependency
  edges changed;
- `PipeBarrier` count, locations, and synchronization scope;
- warmup, repeat count, device-event timing boundary, and host setup outside the
  timed region;
- alternating Parent/Candidate order and retained raw samples.

The dependent variant will make the equal-count Vector operations consume the
previous result through a recurrence. The independent variant will split the
same operation budget across two independently addressable Vector chains. The
two variants must write the same observable output, or the micro-test is not a
valid identity comparison. Any compiler-generated instruction difference will
be inspected before interpreting timing.

## Decision state

```text
ORTHOGONALITY = PASS (design-level, pending source inspection)
HYPOTHESIS = FEASIBILITY_UNPROVEN
COMPILE = NOT_RUN
CORRECTNESS = NOT_RUN
LOCAL_MEASUREMENT = NOT_RUN
LOCAL_VERDICT = NONE
OFFICIAL = NOT_APPLICABLE
```

The hypothesis is that two independent Vector chains can expose issue-level
overlap hidden by a dependent recurrence, producing a repeatable reduction in
the same timed kernel. A null result is a legitimate result: it would indicate
that the tested Vector issue slot is not latency-bound under this fixed memory
and synchronization envelope. No result will be promoted to the Champion
Kernel or submitted Online from this route.

## Next action and stop conditions

1. Locate the existing project harness/runner and its accepted source/ABI
   contract; do not create a second runner or execution chain.
2. Build one route-owned micro-test source variant pair under the existing
   harness only after the source contract is confirmed.
3. Inspect generated instructions if the toolchain emits them, then run
   correctness/identity and interleaved local relative timing only when the
   existing runner and device permit it.
4. Record raw evidence and classify the result as `LOCAL_ACCEPTED`,
   `LOCAL_REJECTED`, `MEASUREMENT_BLOCKED`, or `FEASIBILITY_UNPROVEN`.

If the runner, device, or identity path cannot support this contract, stop with
the exact command and error evidence and use `MEASUREMENT_BLOCKED` or
`FEASIBILITY_UNPROVEN`; do not infer a score.
