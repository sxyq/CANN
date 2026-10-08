# ROW-SCALE-HOIST-X V049

- Direct parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`); this is a sibling, not a continuation of V048.
- Focus axis: FP32 row-scale placement relative to the parameter-ready wait in the BF16 one-row `ProcessNarrowMidOverlap` path.
- Single change: for BF16 only when `residentParams == false`, move the existing FP32 row-scale `Muls` on `valueLocal` ahead of the parameter-ready wait; retain the post-gamma `Muls` unchanged for resident multi-row execution.
- Rationale: overlap the existing row-scale vector operation with the already-issued gamma/bias DMA for one-row BF16 dispatch.
- Target: BF16 `[40,3072]`, 40 vector cores/blocks, one row/core; `residentParams == false`.
- Expected dispatch: `ProcessNarrowMidOverlap` BF16 branch.
- Duplicate boundary: V022 changes BF16 scale/gamma order without targeting the one-row parameter-wait boundary; V043 changes resident multi-row BF16 gamma-operand placement; V042 and V048 move FP32 and FP16 one-row scale operations ahead of the wait. No prior V026 sibling isolates this wait placement for BF16 one-row.
- Unchanged: exact V026 parent base, reduction and inverse-RMS formula, BF16 dtype and FP32 intermediate precision, scale value, gamma/bias math, resident multi-row path, dispatch, buffers, events, and synchronization identities.
- Compile: PASS for `device` and `submission` on `hwnput3`, Ascend910B3 / `dav-2201`, toolkit `8.5.0.alpha002`.
- Correctness: Parent and Candidate PASS on BF16 `[40,3072]`, 40 blocks, one row/core, `resident_params=false`; Candidate source delta executed.
- Local: descriptive ratio-of-pooled-medians score `93.3085528412`, delta `+7.1713116912%` (Candidate slower); classified `MEASUREMENT_BLOCKED` due to Parent block-median drift and mixed paired direction.
- Current Local Best: V026
- Online: NOT_RUN / NOT_AUTHORIZED
