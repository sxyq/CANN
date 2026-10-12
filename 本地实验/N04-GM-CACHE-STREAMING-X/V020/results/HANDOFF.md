# N04 V020 Handoff

Candidate commit `e2a5b36cf`, source SHA-256 `d17d17335443581bcdb64cb8c1625f8d2f1b1e9295c5cabea95b8d726e3ed5c4`. Relative to direct parent V019 commit `1c80adf17`, it changes only the wide low-precision `biasGm_` hint from `PERSISTENT` to `NORMAL`.

The existing CANN 8.5 `paired_runner_v016` entry compiled successfully. Paired Champion SHA-256 is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; executable SHA-256 is `bd47685567583fc9f46c2e07fc4b6c8d4ad3bffa470b0b0eaacca76e079f4b4f`. Correctness passed 4/4 on d7 with return code 0.

Local was not run. The earlier pre-Local d7 reading showed AICore 6% and AIVector 15%, with shared PID 439848 using 21,260 MiB. A fresh reading at `2026-10-12T07:37:10+00:00` later showed d7 idle (AICore/AIVector 0%, HBM 5%, no device process), but `/home/data4t2` had only 2.0 MB free at 100% use. The existing Same-binary qualification and paired Local were therefore not started, and no samples were collected. `LOCAL_SCORE=NONE`. Snapshot: `pre-local-recheck-d7.txt`. No Judge request was made.
