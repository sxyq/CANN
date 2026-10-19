# N01 V015 Handoff

V015 source commit `964b0f3b3302d0072bdf69178d90b04b2b33ae42`, Source SHA256 `43852ba9c9d122c39d15c47752a103275c225f05ca1042d2e6f39cc3968130ca`. In the BF16 wide-row Pass 2, the Candidate reuses one FP32 tile temporary: convert gamma, apply the multiply, overwrite it with converted bias, then add. Arithmetic order, allocated buffers, DMA order, synchronization, row/tile schedule, and core allocation remain unchanged. Per-row conversions are repeated when the batch contains multiple rows.

- CANN 8.5 configure/build: PASS, both exit codes `0`.
- Generated device code differs from V014. `.text=0x14120`; BF16 Candidate function is 14536 bytes, 56 bytes smaller than V014. Register count is unavailable.
- Correctness: PASS, 4/4 on d5, Runner exit `0`.
- Paired Local: 4 x 21 on d5, Runner exit `0`; all raw samples and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Device state before Correctness and Local: HBM 91%, AICore 0%, existing `VLLMWorker_TP` PID 3355594 at 56354 MB. The same HBM/process state remained after Local.
- Source and build identities, runner labels, distributions, and evidence paths are recorded in `RESULTS.md` and `results/`.

NEXT_ACTION: commit and push V015 results, then test one independent wide-row temporary live-range hypothesis in V016 on d5. Do not revisit V001 dtype changes or V007-V014 view-scope changes.
