# UB-LIVENESS-X V001 Architecture Metadata

candidate: UB-LIVENESS-X-V001
architecture hypothesis: Phase-role UB liveness/aliasing — one physical pool; pass1 INGEST/FUSE/REDUCE, pass2 reuses same slot bytes as EMIT (alias=1) or permanent out (alias=0 control).
core mapping: row-parallel; usedCores=min(rows, availableCoreNum, launched blocks); no cross-core reduction.
blockDim strategy: launched blocks = min(availableCoreNum, rows).
row ownership: contiguous chunk per core (blockIdx).
D ownership: full D per row (two-pass stream over D in tiles).
UB allocation estimate: anchor [gammaF|biasF|rdst|rwork 8KiB] + pool [x0 x1 res0 res1 formF mulF]; 184 KiB cap in planner EstBytes.
TQue allocation: none (raw DataCopyPad + MTE2_V events); depth-2 logical slots x0/x1 res0/res1 in pool.
parameter residency lifetime: fullParam when 2*Align32(D*4)+tmp+pool fits 184KiB else per-tile LoadParamChunk.
reduction topology: in-core scalar SumSq + Sqrt reciprocal (V001 correctness-first; ReduceSum workspace reserved).
cross-core synchronization strategy: none (row ownership).
cross-core synchronization count: 0.
expected x rereads: 2 (two-pass) per row.
expected residual rereads: 2.
expected gamma reload groups: 1 if fullParam else ceil(D/tile).
expected bias reload groups: same as gamma.
workspace usage: no GM workspace.
hot-path domain: legal shapes with planner fit (typical mid D).
hot-path dispatch condition: PlanAndAlloc always; two-pass process; fullParam optional.
fallback domain: same two-pass scalar path covers full legal domain (hot and fallback share body in V001).
fallback dispatch condition: n/a (unified path).

## V001 result
- Compile/link PASS on server3.
- Correctness FAIL on sampled domain → LOCAL_REJECTED.
- Isolation note: identity out=u emit matched golden; full inv*gamma+bias still wrong.
