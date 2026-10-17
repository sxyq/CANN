# W4-R06 MTE2-XRES-ISSUE-X — DUPLICATE_AUDIT

ROUTE = W4-R06 MTE2-XRES-ISSUE-X
REVISION = V001
STAGE = DUPLICATE_AUDIT
VERDICT = SAME_MECHANISM_ALREADY_TESTED
DECISION = ROUTE_DUPLICATE_BLOCKED
EDIT_PERFORMED = NO
CANDIDATE_WRITTEN = NO
SERVER3_ACTION = NONE

## Parent verification

PARENT = R31B V011
PARENT_PATH = 归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
PARENT_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SHA256_EXPECTED = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SHA256_RESULT = MATCH
SUBMISSION_COPY = 线上结果/R31B/V011/submission.asc
SUBMISSION_COPY_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
BYTE_IDENTICAL = YES
PARENT_CONFLICT = NO

## Mechanism under test

TARGET = pass-1 input MTE2 issue order between the x Load and the residual Load
in ProcessWideLowPrecision. Two adjacent, independent DataCopy transfers per
(row, tile) unit, issued x-first. V001 would test residual-first for the same
pair, leaving every other variable unchanged.

SOURCE_LOCATION = 归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc lines 3141-3142
  Load(xLocal, xGm_, rowOffset + col, valid);
  Load(residualLocal, residualGm_, rowOffset + col, valid);
SOURCE_LOCATION_PREFETCH = same file lines 3172-3173
  Load(xNext, xGm_, nRowOffset + nCol, nValid);
  Load(rNext, residualGm_, nRowOffset + nCol, nValid);

## Prior route with the same mechanism

PRIOR_ROUTE = CROSSROW-FULL-PIPELINE-CHAMPION-X (R5)
PRIOR_REVISION = V010
PRIOR_DIRECT_PARENT = R31B-V011
PRIOR_COMMIT = 188d6c6e ("perf(crossrow): record V010 input dma order result")
PRIOR_CANDIDATE_PATH = origin/w3/m1/crossrow-full-pipeline:
  本地实验/CROSSROW-FULL-PIPELINE-CHAMPION-X/V010/Candidate.asc
PRIOR_PARENT_PATH = same directory, Parent.asc
PRIOR_PARENT_SHA256 = a06841eaf2d335318518aa71c1480e5f94bd9a4a4f9d111a681a53fde74be469

PRIOR_HYPOTHESIS = "Issuing the residual tile DMA before the x tile DMA in the
  FP16 retained-y pass-1 may improve MTE2 scheduling overlap across adjacent
  rows while leaving the same input buffers, ready events, waits, and
  arithmetic dependencies."
PRIOR_SCOPE = one input DMA ordering variable; residual-first for FP16
  pass-1; applied to the initial unit and each prefetched next unit; event
  allocation, waits, barriers, traversal order, parameter prefetch, batch
  size, tile width, retained-y layout, buffer sizes, arithmetic order, and all
  non-FP16 paths unchanged.

PRIOR_DIFF_EQUIVALENCE = the V010 Candidate diff against its Parent is a
  byte-for-byte match of the change this Route would make: both Load pairs
  wrapped in `if constexpr (std::is_same<T, half>::value)` with residual
  emitted first and x second, and no other hunk.

## Activation-condition comparison

CONDITION = ProcessWideLowPrecision pass-1, reached only when
  widePath_ is true, i.e. rowWidth > kCacheElems (kCacheElems = 8192),
  and T is half or bf16; the non-wide and FP32 branches are untouched.

PARENT_ROUTING_SOURCE = identical between R31B V011 and the R5 V010 Parent.
  Init() region lines 40-75 byte-identical.
  widePath_ constants identical: kCacheElems = 8192,
  kWideFullYTileElems = 4096, kWideFullYMaxRows = 8,
  kWideFullYReduceStride = 16, kTileElems = 4096.

PASS1_BLOCK_IDENTITY = lines 3105-3221 of the R5 V010 Parent produce
  79c85efb0e8ef628302b2b07d71c7bdcef877c60ce4f7fd5f848cdc584b63f37,
  identical to the same line range of R31B V011. The code under test is the
  same code.

PROBE_SHAPE_PRIOR = rows=16, width=16384, dtype=fp16, blocks=8, device=1.
  16384 > 8192, so the prior run entered exactly this Route's code path.

ACTIVATION_MATCH = YES

## Parent comparison

PARENT_MATCH = YES. R5 V010 names R31B-V011 as its Direct Parent, and the
  pass-1 region of its retained Parent.asc is byte-identical to the R31B V011
  file this Route names as its parent.

## Prior measurements

R5 V010, device 1, 16x16384 fp16, blocks=8, warmups=5, 21 paired samples,
alternating PC/CP order, device event primary:
  CORRECTNESS = PASS, bit_differences=0, max_abs=0, tolerance_failures=0
  median_parent = 16.519999 us
  median_candidate = 17.220000 us
  median_paired_delta = +0.399999 us
  median_delta_pct = +1.9881 percent
  VERDICT = LOCAL_NO_PROMOTION, best remains V001

R5 V021, same mechanism repeated on R5 V012, same shape and device:
  CORRECTNESS = PASS, bit_differences=0, max_abs=0
  parent samples n=22 median 10.460000 us MAD 0.190000 MAD/median 0.0182
    max/min 24.82, three samples above 2x median
  candidate samples n=21 median 10.300000 us MAD 0.180000 MAD/median 0.0175
    max/min 2.15, one sample above 2x median
  paired deltas median -0.080000 us, -0.7648 percent
  direction 11 favour candidate / 10 favour parent
  VERDICT = LOCAL_NO_PROMOTION, best remains V012
  INTERPRETATION = direction split 11/10 with a -0.76 percent median sits
    inside the observed same-binary spread; not a reproducible direction.

Both prior runs reproduce as a single measurement each. Neither produced a
stable direction, and neither was promoted.

## Related order-sensitivity evidence on the same parent

MULTIROW-PANEL-RMS-CHAMPION-X V003 (commit 706db2a1), Direct Parent R31B-V011,
changed pass-1 traversal from row-major (batchRow, tile) to panel-major
(tile, batchRow) in the same function:
  PROBE = FP16 rows=128 width=12288 device=7 warmups=45 blocks=4
  samples 21 per block, 84 pairs
  side median delta = +1.4500005 us / +3.9413 percent
  paired median delta = -0.800000 us / -2.1745 percent
  per-block medians = -0.100001, +0.580000, -8.020000, -0.619998 us
  RESULT = LOCAL_MIXED_NO_PROMOTION_ORDER_SENSITIVE
  CORRECTNESS = PASS on both sides, max_abs_error 0.001953125

Reordering input traffic in this function is recorded as order-sensitive
with no stable sign.

## Ruled out as the same mechanism

MULTIROW-DMA-CHAMPION-X V001 = stride multi-row DataCopy, reduces MTE2 command
  count. Different mechanism. LOCAL_REJECTED (+8.4 percent on 48x16384 FP16).
MULTIROW-DMA-CHAMPION-X V002 = Pad to non-Pad single-burst DataCopy form.
  Different mechanism, terminal inside noise.
ALIGN-TAIL-X = tail-block aligned DataCopy. Different mechanism.
COEFF-LOCALITY-X V003 = parameter MTE2 timing. Different code path (pass-2
  gamma/bias), refuted at constant tile width.
R5 CROSSROW V016 = pass-2 gamma/bias DMA order bias-first. Different buffers,
  different pass.
R5 CROSSROW V019 = alternating pass-2 parameter DMA order by tile parity.
  Different pass.
R5 CROSSROW V020 = initial FP16 pass-1 double-buffer slot phase, then
  alternate. Slot selection, not x/residual order. +0.1901 percent.
ASYNC-OVERLAP-CHAMPION-X V002 = inter-row issue reorder in NarrowMid, hoisting
  the next row load above the invRms tail. Different path and different move.

None of the above is a duplicate of this Route. V010 and V021 are.

## Duplicate verdict

CRITERION_MECHANISM = SAME. Same function, same two DataCopy calls, same
  swap, same dtype guard, no other hunk.
CRITERION_ACTIVATION = SAME. Same rowWidth > 8192 non-FP32 wide condition, same
  constants, byte-identical pass-1 block, and the prior probe shape provably
  entered this path.
CRITERION_PARENT = SAME. R31B V011 is the named Direct Parent in both.

SAME_MECHANISM_ALREADY_TESTED = YES
ROUTE_DUPLICATE_BLOCKED = YES

The Route stops before any Candidate edit. The mechanism was measured twice
on the same parent code, once as a 1.99 percent regression and once as a
0.76 percent split-direction result inside noise. Neither established a
reproducible gain.

## Local matrix

NOT_PARSED. Local matrix parsing is reached only when the duplicate verdict
is NO_GAIN_FOR_RETRY. The Route stopped at the duplicate check, so no shape
was selected and no measurement was designed.

## Safety

HISTORICAL_BRANCHES = read-only. origin/w3/m1/crossrow-full-pipeline and
  origin/main were read with git show / git ls-tree only. No build, no write,
  no commit, no push in any W2/W3 worktree or branch.
WRITES_THIS_ROUTE = this file only, inside
  /Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R06-mte2-xres-issue-x
SERVER3 = not contacted. No Compile, no Correctness, no Local, no Profile.
SHARED_RECORDS = not written. Route Agent does not write 全版本记录.tsv,
  路线成绩表.tsv, 当前任务.tsv, 技术路线图.md, or any Dashboard.
ONLINE = PAUSED. No Judge submission.

## Repository vocabulary check

This file uses only plain statements. It contains none of the prohibited
style words listed in AGENTS.md.
