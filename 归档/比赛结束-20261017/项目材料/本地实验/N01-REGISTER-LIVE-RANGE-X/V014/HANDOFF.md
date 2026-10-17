# N01 V014 Handoff

V014 source commit `b25f528e8d165cabde0bdd14268674cc718e1ee1`, Source SHA256 `4efece873de93237048d54c0658eac05891a9c88ec3b2a097aa6f9ab40cc54d4`. It shortens the Pass 1 non-half `residualFp32` view scope by acquiring the view inside the active tile branch.

- CANN 8.5 configure/build: PASS, both exit codes `0`.
- Generated device code differs from V013; `.text=0x14158`, register count unavailable.
- Correctness: PASS, 4/4 on d5, runner exit `0`.
- Paired Local: 4 x 21 on d5, runner exit `0`; raw samples and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Device state: d5 HBM 91%, AICore 0%; existing `VLLMWorker_TP` PID 3355594 at 56354 MB remained present.
- Exact source and binary identities, runner-slot labels, distributions, and evidence paths are in `RESULTS.md` and `results/`.

NEXT_ACTION: commit and push V014 results. Continue with one substantive BF16 Pass 2 FP32 temporary-reuse hypothesis: use one tile temporary for gamma and bias in consecutive phases, preserving per-element arithmetic order and existing DMA synchronization.
