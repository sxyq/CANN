# N04 V020 Handoff

Candidate commit `e2a5b36cf`, source SHA-256 `d17d17335443581bcdb64cb8c1625f8d2f1b1e9295c5cabea95b8d726e3ed5c4`. Relative to direct parent V019 commit `1c80adf17`, it changes only the wide low-precision `biasGm_` hint from `PERSISTENT` to `NORMAL`.

The existing CANN 8.5 `paired_runner_v016` entry compiled successfully. Paired Champion SHA-256 is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; executable SHA-256 is `bd47685567583fc9f46c2e07fc4b6c8d4ad3bffa470b0b0eaacca76e079f4b4f`. Correctness passed 4/4 on d7 with return code 0.

Local was not run. The earlier pre-Local d7 reading showed AICore 6% and AIVector 15%, with shared PID 439848 using 21,260 MiB. A fresh reading at `2026-10-12T07:37:10+00:00` later showed d7 idle (AICore/AIVector 0%, HBM 5%, no device process), but `/home/data4t2` had only 2.0 MB free at 100% use. The existing Same-binary qualification and paired Local were therefore not started, and no samples were collected. `LOCAL_SCORE=NONE`. Snapshot: `pre-local-recheck-d7.txt`. No Judge request was made.

Latest read-only resume snapshot (`2026-10-12T11:50:28+00:00`) records `/home/data4t2` at 100% displayed capacity with 6,815,252 KiB available. NPU d7 reported AICore 19% and HBM 60,613/65,536 MiB; Python PID 3954164 used 57,243 MiB on that device. The process was left untouched, and V020 Local/profile were not started. Snapshot: `pre-local-resume-20261012T115028Z.txt`.

The existing V020 paired object contains an embedded `.aicore_binary`, but no device instruction listing or msprof evidence is saved for this route. Main's verified Official comparison is N04 V017 41.85 vs R31B V011 45.16; testcase dtype/Rows/D remain UNKNOWN, so no Official Case is assigned to a Local shape. No evidence currently selects a new cache Hint factor. Resume V020 Local and collect route-level profile evidence when d7 and the output volume permit it; use that evidence before changing another source factor. Do not submit this route to Judge; a qualified identity belongs with N06.
