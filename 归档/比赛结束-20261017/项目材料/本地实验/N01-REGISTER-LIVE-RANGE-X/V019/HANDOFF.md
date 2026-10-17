# N01 V019 Handoff

V019 source commit `405a56772fd2eddb32f416b99805048d7780d87d`, source SHA256 `be78afa95cf955825282e458c02d137fffbdc2ff36ccfb21a20d5c9357244893`. FP16 Pass 2 now uses `valueFp32` for conversion and normalization instead of `xFp32`; arithmetic order and synchronization are unchanged.

- CANN 8.5 configure/build and AICore ELF extraction: PASS, all exit codes `0`.
- The AICore ELF differs from V018; FP16 Candidate function size is 4 bytes smaller. Register count is unknown.
- Correctness: PASS, 4/4 on d5, Runner exit `0`.
- Paired Local: 4 x 21 on d5, Runner exit `0`; all raw values and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Runner Candidate label `V016` is generic; its actual input is the V019 source SHA above. The measurement Parent is fixed R31B V011.
- d5 before and after sampling: HBM 91%, AICore 0%; existing `VLLMWorker_TP` PID 3355594 at 56354 MB.
- Exact identities, distributions, and evidence paths are in `RESULTS.md`.

NEXT_ACTION: commit and push V019 results, then start a distinct FP32 temporary-lifetime hypothesis as V020 on d5.
