# N01 V017 Handoff

V017 source commit `b0801b847d1d523fddb4c8e37221b8ab59040e2c`, Source SHA256 `24918945466c2f1d848f96c610582b45ba8c2011ea72be7f65321f91dc4cdcad`. In BF16 Pass 1, after residual is added to the retained y tile, V017 overwrites the now-dead `residualFp32` tile with y squared and uses `xFp32` as the ReduceSum work tile. This tests reuse of the expired converted-input buffer as a later-stage value; the formula, operation order, DMA, synchronization, tiling, and core assignment are unchanged.

- CANN 8.5 configure/build: PASS, both exit codes `0`.
- Generated device code differs from V016; `.text=0x14108`; BF16 Candidate function is 14512 bytes. Register count is unavailable.
- Correctness: PASS, 4/4 on d5, Runner exit `0`.
- Paired Local: 4 x 21 on d5, Runner exit `0`; all raw samples and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Device state: HBM 91%, AICore 0%; existing `VLLMWorker_TP` PID 3355594 at 56354 MB was present before and after the run.

NEXT_ACTION: commit and push V017 results, then continue a distinct wide-row FP32 temporary lifetime hypothesis as V018 on d5.
