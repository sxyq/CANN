# N01 V010 Handoff

V010 source is commit `33ded88d38088d696d3f208d906d92d871686669`, Source SHA256 `389e01bf37813293d4477d711c03c6419602297906e3be9fdaf59d2c80870f7b`. The V010 result files record the exact remote Candidate and Parent compile inputs.

- CANN 8.5 configure/build: PASS, both exit codes `0`.
- Generated `.aicore_binary` differs from V009; `.text` is 28 bytes shorter. Register count is unavailable.
- Correctness: PASS, 4/4 on d5, runner exit `0`.
- Paired Local: 4 x 21 pairs on d5, runner exit `0`; all raw values and outliers retained. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`.
- Device state: d5 HBM 91%, AICore 0%; existing `VLLMWorker_TP` at 56354 MB remained unchanged.
- Evidence and paired statistics: `RESULTS.md` and `results/`.

NEXT_ACTION: commit and push the V010 result files, then begin V011 by shortening only the Pass 2 `valueFp32` view lifetime in the non-half per-row path. Do not alter Scalar address generation or repeat an earlier view-lifetime change.
