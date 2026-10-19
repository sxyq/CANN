# N01 V013 Handoff

V013 source is commit `c3d557a0fa2564f1db3561f28d4ccf3f219e4bfc`, Source SHA256 `3499280f91cd2a3069d72b08fde4cc865a99bf84238c73168e34d6847076ca32`. The only code change narrows the Pass 2 `residualFp32` view scope.

- CANN 8.5 configure/build: PASS, both exit codes `0`.
- Generated `.aicore_binary` differs from V012; `.text` is `0x1410c`. Register count is unavailable.
- Correctness: PASS, 4/4 on d5, runner exit `0`.
- Paired Local: 4 x 21 pairs on d5, runner exit `0`; all samples and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Device state: d5 HBM 91%, AICore 0%; existing `VLLMWorker_TP` at 56354 MB remained unchanged.
- Full distributions, source identity, and evidence paths are in `RESULTS.md` and `results/`.

NEXT_ACTION: commit and push V013 results, then continue the register-lifetime study with a distinct factor on d5.
