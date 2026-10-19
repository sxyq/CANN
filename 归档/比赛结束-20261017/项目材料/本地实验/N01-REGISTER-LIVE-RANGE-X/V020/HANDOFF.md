# N01 V020 Handoff

V020 source commit `934e9c99ef531799c4cb9645f45372909bef5248`, source SHA256 `2add7f609bb93ffb84bed7040f1526ce02aab2b1049eed14ad12ef867d1cf7dc`. FP16 Pass 1 now uses `residualFp32` as the tile `ReduceSum` scratch instead of `xFp32`; the reduction input and result tile are unchanged.

- CANN 8.5 configure/build and device-code extraction: PASS, all exit codes `0`.
- Device ELF differs from V019; function sizes and `.text` size are unchanged. Register count is unknown.
- Correctness: PASS, 4/4 on d5, Runner exit `0`.
- Paired Local: 4 x 21 on d5, Runner exit `0`. Three cases are noisy; FP16 D=32768 is comparatively stable but slightly slower. `LOCAL_SCORE=NONE`.
- Runner Candidate slots `V001`/`V016` are generic labels; actual input is the V020 source SHA above. The measurement Parent is fixed R31B V011.
- d5 before and after sampling: HBM 91%, AICore 0%; existing `VLLMWorker_TP` PID 3355594 at 56354 MB.
- A non-N01 d5 Runner visible in one pre-Correctness snapshot had exited by the next read. No process was interrupted.
- Full input identity, distributions, and raw paths are in `RESULTS.md`.

NEXT_ACTION: commit and push V020 results, then continue a distinct wide-row FP32 temporary-lifetime hypothesis as V021 on d5.
