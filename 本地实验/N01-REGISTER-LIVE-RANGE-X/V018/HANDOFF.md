# N01 V018 Handoff

V018 source commit `07f0d3b603f966ebca9b4efc8d9a66087bdee458`, source SHA256 `bc6b799d822b31cd3ee9344e3ad8f77165a7a2826f5593321b645c72781dd0ce`. In the FP16 path, the FP32 square input is read from cached `yTile` after the tile has been written there, allowing `xLocal` to finish its Vector use earlier. Only one source line differs from V017.

- CANN 8.5 configure/build and device-code extraction: PASS, all exit codes `0`.
- Correctness: PASS, 4/4 on d5, Runner exit `0`.
- Paired Local: 4 x 21 on d5, Runner exit `0`; outliers remain in raw evidence. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Runner label `V016` is generic; its Candidate input is the V018 source SHA above. The measurement Parent is R31B V011.
- d5 before and after sampling: HBM 91%, AICore 0%; existing `VLLMWorker_TP` PID 3355594 at 56354 MB.
- Complete identities, distributions, and evidence paths are in `RESULTS.md`.

NEXT_ACTION: commit and push V018 results, then continue with a distinct single-factor register-lifetime hypothesis as V019 on d5. Do not repeat V018.
