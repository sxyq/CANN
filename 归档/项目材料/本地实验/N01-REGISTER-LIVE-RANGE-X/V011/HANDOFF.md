# N01 V011 Handoff

V011 source is commit `d3341d74208cb73a49c939fb28e960e3d1eca84d`, Source SHA256 `c0f3ed3ccb2f31f1618549e3f98d5ef2f01f92f16b11e5a9f94439fb5208eb26`. The only code change narrows the Pass 2 `valueFp32` LocalTensor view to each non-half row.

- CANN 8.5 configure/build: PASS, both exit codes `0`.
- Generated `.aicore_binary` differs from V010; `.text` is `0x14104`. Register count is unavailable.
- Correctness: PASS, 4/4 on d5, runner exit `0`.
- Paired Local: 4 x 21 pairs on d5, runner exit `0`; all raw samples and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Device state: d5 HBM 91%, AICore 0%; existing `VLLMWorker_TP` at 56354 MB remained unchanged.
- Evidence and paired statistics: `RESULTS.md` and `results/`.

NEXT_ACTION: commit and push V011 results; continue the register-lifetime study with a distinct hypothesis and keep d5 as the only N01 device.
