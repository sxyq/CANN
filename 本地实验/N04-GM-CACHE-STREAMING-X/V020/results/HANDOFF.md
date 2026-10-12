# N04 V020 Handoff

Candidate commit `e2a5b36cf`, source SHA-256 `d17d17335443581bcdb64cb8c1625f8d2f1b1e9295c5cabea95b8d726e3ed5c4`. Relative to direct parent V019 commit `1c80adf17`, it changes only the wide low-precision `biasGm_` hint from `PERSISTENT` to `NORMAL`.

The existing CANN 8.5 `paired_runner_v016` entry compiled successfully. Paired Champion SHA-256 is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; executable SHA-256 is `bd47685567583fc9f46c2e07fc4b6c8d4ad3bffa470b0b0eaacca76e079f4b4f`. Correctness passed 4/4 on d7 with return code 0.

Local was not run: the latest pre-Local d7 reading showed AICore 6% and AIVector 15%, with shared PID 439848 using 21,260 MiB. `LOCAL_SCORE=NONE`; that process was left untouched. No Judge request was made.
