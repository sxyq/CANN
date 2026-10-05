# ROW-OCCUPANCY-CHAMPION-X V001

ROUTE=ROW-OCCUPANCY-CHAMPION-X
REVISION=V001
HYPOTHESIS_ID=ROW-OCC-H1-BOUNDED-LOGICAL-BLOCKS
DIRECT_PARENT=R31B/V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
BASELINE_OFFICIAL_SCORE=45.16
EXACT_FUNCTION=run_kernel
EXACT_CODE_SITE=本地实验/ROW-OCCUPANCY-CHAMPION-X/V001/submission.asc:3532-3547

## Current behavior

The parent launches one logical block per available physical core, clamped to
the row count. The device receives that count and retains its existing
contiguous `baseRows/extraRows` row ownership.

## Single hypothesis

Use a bounded two-logical-block ceiling when `rowCount` exceeds the current
launch width. The selected count is `min(rowCount, 2 * max(availableCoreNum,
1))`, with the parent UINT32 ABI clamp retained.

## One-factor change

Only the host-side `requestedBlocks` calculation in `run_kernel` changes.
There is no change to the device kernel, row partition equations, math, dtype
policy, tile geometry, DMA, event lifetime, reduction, store, UB, or parameter
layout.

## Target and expected effect

TARGET_CORE_CASES=C12,C13,C14,C16
TARGET_SHAPES=canonical suite; especially multi-row cases where rowCount exceeds the available core count
TARGET_DTYPES=BF16,FP16
EXPECTED_EFFECT=additional static scheduling boundaries may smooth packet/tail variation while physical core count stays fixed
EXPECTED_RISK=extra logical-block initialization and scheduling overhead; no arithmetic risk expected

## Gate records

ROW_OCC_H1_SOURCE_AUDIT=PASS
DUPLICATE_WITH=NONE
ROW_OCC_H1_GATE=PASS
SINGLE_CHANGE_AUDIT=PASS
NO_ONLINE=YES

## Required execution order

`source commit -> build -> correctness -> canonical local score`.

The canonical scorer is `MAIN2-CANONICAL-V1` with anchor `R31B/V011` and
core cases `C12,C13,C14,C16`. A numerical score above 100 is not by itself a
reliable positive; per-case quality and consistency remain part of the final
verdict.
