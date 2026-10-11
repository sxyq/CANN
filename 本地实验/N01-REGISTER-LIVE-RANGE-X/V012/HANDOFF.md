# N01 V012 Handoff

V012 source is commit `beae4dcca02b5bf90550d3d290188e8f98cba518`, Source SHA256 `4e6e4b1f13f00bffbff9102c65f9ab0e8b788adcbd6a653331faeac8c0da5a17`. The only source change narrows the Pass 2 `xFp32` view scope.

- CANN 8.5 configure/build: PASS, both exit codes `0`.
- Generated `.aicore_binary` differs from V011; `.text` is `0x14148`. Register count is unavailable.
- Correctness: PASS, 4/4 on d5, runner exit `0`.
- Paired Local: 4 x 21 pairs on d5, runner exit `0`; all raw samples and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Device state: d5 HBM 91%, AICore 0%; existing `VLLMWorker_TP` at 56354 MB remained unchanged.
- Full distribution statistics, input identity, and evidence paths are in `RESULTS.md` and `results/`.

NEXT_ACTION: commit and push V012 evidence, then test a distinct temporary-view lifetime hypothesis using d5 only.
