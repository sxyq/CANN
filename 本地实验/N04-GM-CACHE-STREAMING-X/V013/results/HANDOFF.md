# N04 V013 Handoff

V013 directly follows N04 V012 and changes only the wide low-precision `xGm_` hint from `CACHE_MODE_PERSISTENT` to `CACHE_MODE_DISABLE`. Candidate commit is `3b12f050`; source SHA-256 is `173e9233b51f07d2666130cf27409a23431eac61ac3fe353804a5200f1ee6c6b`.

Using the existing server3 `paired_runner_v016`, Compile passed, Champion/Candidate Correctness passed 4/4 on d7, and paired Local completed 4x21 with return code 0. The full sample set shows one clear per-shape signal at FP16 D=32768, two tail distributions crossing zero, and extreme two-sided outliers at BF16 D=32768 alongside changing d7 load. Overall `LOCAL_SCORE=NONE`; retain all cases and raw samples. These measurements are Local only and do not represent Official results. Details and source identities are in `RESULTS.md` and adjacent evidence files.

Next single-factor trial: derive V014 from V013 and change only `residualGm_` from `CACHE_MODE_NORMAL` to `CACHE_MODE_PERSISTENT`; leave x disabled, output disabled, and all compute and dataflow behavior unchanged. This tests a distinct residual-stream L2 hint with the V013 x policy. Reuse the existing paired entry, use R31B V011 Champion, and measure on d7 only.
