# I001 V001 Architecture Metadata

candidate: I001
revision: V001
architecture hypothesis: Wide-D streaming is dominated by GM rereads and tiny D-tiles. Retained-y for rows that fit UB, large-tile two-pass otherwise, and a low-sync D-stripe when batch is tiny and D is huge.

core mapping: row-parallel by default (contiguous row ranges per Vector Core). D-stripe mode splits the reduction dimension across cores.

blockDim strategy: min(availableCoreNum, 40, rows) for row modes. D-stripe uses min(availableCoreNum, 40, ceil(D/512)).

row ownership: each core owns a contiguous row range in row modes. D-stripe owns no full row.

D ownership: full D per row in row modes. D-stripe owns a contiguous D stripe for every row.

UB allocation estimate:
- tile T buffers x/r/g/b: 4 * tile * sizeof(T)
- FP32 work fA/fB: 2 * tile * 4
- reduce tmp: 128 B
- retained u row: D * 4 only when mode=retained
- budget target <= 184 KiB usable (192 KiB TOTAL_UB_SIZE minus TMP/sync)

TQue allocation: 4 x TQue VECIN depth 1 (x, residual, gamma, bias). TBuf for fA/fB/tmp/uRow.

parameter residency lifetime: gamma/bias loaded per D-tile (or whole row in retained mode). Not cached across rows in V001.

reduction topology: intra-core ReduceSum over sum(u*u). D-stripe adds a 2-phase core reduce via GM workspace + SyncAll.

cross-core synchronization strategy: none in row modes. D-stripe uses SyncAll twice (after partial write, after total write).

cross-core synchronization count: 0 (row modes) or 2 (D-stripe, all rows amortized).

expected x rereads: 1 (retained) or 2 (two-pass).
expected residual rereads: 1 (retained) or 2 (two-pass).
expected gamma reload groups: 1 per D-tile per row (row modes); 1 per D-tile per row in stripe.
expected bias reload groups: same as gamma.

workspace usage: D-stripe only, float[cores * R + R], device malloc, zeroed.

hot-path domain:
- retained: retainNeed <= 150 KiB (covers most D<=8192 FP32 and larger for half)
- two-pass row: retainNeed > 150 KiB and rows > 8
- D-stripe: retainNeed > 150 KiB and rows <= 8 and D >= 4096

hot-path dispatch condition: host-side in run_kernel from shape/dtype.

fallback domain: same kernels cover FP32/FP16/BF16, rank 2/3/4 via R=prod(leading), D=shape[-1], all D in 64..32768 including unaligned D via DataCopyPad + exact counts.

fallback dispatch condition: any legal input falls into one of the three modes; no separate copy of other candidates.

parameter-stripe / wide batch: V001 keeps gamma/bias tile-local. Next revision can batch 2+ rows per param tile.
