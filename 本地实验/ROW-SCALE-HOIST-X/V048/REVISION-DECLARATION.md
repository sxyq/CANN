# ROW-SCALE-HOIST-X V048

- Direct parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Focus axis: row-scale placement relative to the parameter-ready wait, FP16 one-row `ProcessNarrowMidOverlap`
- Single change: move the existing native-half row-scale Muls on the output ahead of the parameter-ready wait; keep the gamma Mul after that wait and preserve bias placement.
- Rationale: overlap the existing row-scale vector operation with the already-issued gamma/bias DMA in the one-row FP16 path.
- Target: FP16 `[40,3072]`, 40 vector cores/blocks, one row/core; `residentParams == false`.
- Expected dispatch: `ProcessNarrowMidOverlap_FP16`.
- Compile: PASS for `device` and `submission` on `hwnput3`, Ascend910B3 / `dav-2201`, toolkit `8.5.0.alpha002`.
- Correctness: Parent and Candidate PASS on FP16 `[40,3072]`; selected `ProcessNarrowMidOverlap_FP16`, one row/core, `resident_params=false`. Candidate source delta executed.
- Local: 128 samples per arm, four interleaved blocks; descriptive score `119.552054493`, delta `-16.3544278481%`; `MEASUREMENT_BLOCKED` for high variance and Candidate spikes.
- Current Local Best: V026
- Device assignment for Correctness/Local: device 0 exclusive assignment received; pre-correctness and pre-Local snapshots confirm 0% AICore, 3432/65536 MB HBM used, no process observed on NPU 0. Explicitly released at `2026-10-08T02:46:17Z` after numeric capture; post-snapshot shows 0% AICore, 3432/65536 MB used, no process on NPU 0.
- Online: NOT_RUN / NOT_AUTHORIZED
