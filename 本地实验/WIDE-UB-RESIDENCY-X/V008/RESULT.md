# WIDE-UB-RESIDENCY-X V008

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V008
PARENT: `本地实验/WIDE-UB-RESIDENCY-X/V007/submission.asc`
PARENT_COMMIT: `bd2e5f7f`
RESULT_COMMIT_PARENT: `b321a307`
BRANCH: `route/new-n02-wide-ub-residency-x-20261016`

## HYPOTHESIS

For the existing rows=2, D=12288 FP16/BF16 cases, the wide low-precision UB
planner can retain at least two complete `y` rows per core. The current launch
uses two blocks for two rows, so each block reads the complete `gamma` and
`bias` vectors from GM. Launching one block lets its existing Pass2 batch loop
reuse each parameter tile across both rows.

This differs from V006, which removes a Pass2 synchronization pair without
changing the launch or GM traffic, and V007, which reuses a BF16 output slot
and increases a tile size without changing the two-block assignment for these
shapes. V008 changes only block assignment for low-precision rows=2,
D=12288. D=32768 remains unchanged.

## ACCESS AND SPACE ESTIMATE

At D=12288, each dtype's parameter vectors contain 12,288 elements. Gamma and
bias together are 49,152 logical bytes for FP16 and 49,152 logical bytes for
BF16: `2 vectors * 12,288 elements * 2 bytes`. With two blocks, each vector
is loaded once per block: 98,304 logical parameter bytes per launch. With one
block, each vector is loaded once total: 49,152 bytes, a source-level
reduction of 49,152 bytes (50%). At tile=4096, this removes six Pass2
`DataCopyPad` calls per launch (three gamma and three bias tile loads).

The two-row `x` and `residual` reads are unchanged at 98,304 bytes total, so
total logical GM read payload falls from 196,608 to 147,456 bytes (25%). The
output write remains 49,152 bytes. These are source-level copy payloads, not
physical HBM transactions.

This counts logical GM-to-UB payload from the `Load` calls, not physical HBM
transactions. The two-block launch performs each row's Pass1 inputs and output
store on its original core. The one-block launch does not change the total
input or output payload; it serializes both rows on one core. The existing
V007 planner uses tile=4096 at D=12288. Its FP16 allocation for three retained
rows is 164,032 bytes: 73,728 for `y`, 32,768 for double-buffered inputs,
8,192 for output, 49,152 for three FP32 work tiles, and 192 for reduction
partials; this is within 176 KiB (180,224 bytes). BF16 retains two rows in
163,968 bytes: 98,304 for FP32 `y`, 32,768 for double-buffered inputs,
32,768 for two FP32 work tiles, and 128 for reduction partials; this is
within 177 KiB (181,248 bytes). Therefore both D=12288 rows fit in one
existing batch without allocating more UB. No vector instruction is added.
The main tradeoff is one active block rather than two, so measured benefit
remains unproven.

## CHANGE

In the existing Host dispatch, set `requestedBlocks=1` only when
`rowWidth==12288`, `rowCount==2`, and dtype is FP16 or BF16. Existing
`ProcessWideLowPrecision` batch assignment, Pass1, Pass2 and parameter loads
remain unchanged. The paired Runner source now includes V008 and its existing
CMake default points at the V008 directory; no new runner or script is
introduced.

## COMPILE / DEVICE

COMPILE: NOT_RUN.
CORRECTNESS: NOT_RUN.
LOCAL: NOT_RUN.
NOT_RUN_REASON: Main's cross-route read-only comparison found that V008 changes
`blockCount` from two to one for rows=2, making each core process two rows to
reuse Gamma/Bias. N01 V001-V003 already explored reducing blocks and increasing
rows per core, so this is the same row-to-core mapping mechanism. V008 remains
preserved as an unvalidated record; no build or device run was started.
LOCAL_SCORE: NONE.
DEVICE: NOT_REQUESTED.
DEVICE_RESET: NO.
PUSH: NO.
