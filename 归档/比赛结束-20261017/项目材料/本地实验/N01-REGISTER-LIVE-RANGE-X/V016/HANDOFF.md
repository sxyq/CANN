# N01 V016 Handoff

V016 source commit `b292bf75f6566eb927666b774022855a72ef6d1c`, Source SHA256 `f53b21a9f5ed77c82d26b469d71f4385374fefa47b6f13378ed5f555143a7f21`. In BF16 Pass 1, the Candidate converts `x` directly into the retained `valueTile`, adds the converted residual in place, and acquires `xFp32` only for the subsequent square. This removes one separate live FP32 tile during the widened add. Other paths, arithmetic order, buffer allocation, DMA, synchronization, tiling, and core assignment are unchanged.

- CANN 8.5 configure/build: PASS, both exit codes `0`.
- Generated device code differs from V015; `.text=0x14110`; BF16 Candidate function size is 14520 bytes. Register count is unavailable.
- Correctness: PASS, 4/4 on d5, Runner exit `0`.
- Paired Local: 4 x 21 on d5, Runner exit `0`; raw samples and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Device state: HBM 91%, AICore 0%; existing `VLLMWorker_TP` PID 3355594 at 56354 MB remained present before and after both device runs.
- Exact source, Runner, and device binary identities, slot labels, distributions, and evidence paths are in `RESULTS.md` and `results/`.

NEXT_ACTION: commit and push V016 results, then continue with an independent single-factor wide-row FP32 temporary lifetime hypothesis as V017 on d5. Do not repeat V001 dtype changes or V007-V016 hypotheses.
