# MAIN Review: R31A V021

Date: 2026-09-27

## Current Evidence

- Direct parent: R31A-V016, source SHA-256 `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0`.
- Candidate source SHA-256: `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063`; server3 copy matches.
- Server3 device and submission targets returned RC 0. Their `.alink` SHA-256 values are `96fd4babb50710c5489071aa4861bc89bd07dc8fa32d309de781cd05fb7c7a4f` and `78cd7c977222adf1ebec22e408a885426f097ac37e0a8f8829877a023f268f7f`.
- Parent and Candidate modules are source-bound: `11d7a2cb60912ce23b05a768c9076abcf2c513ad8b0844cc062d158082a4d5ea` and `9bacd8cd72b8db5c6b8a1574ce199c674b18bc2bcf858cfb91ade29f71d24674`. Current correctness runner SHA-256 is `7dbde489de951be883fe38d6cbc991800c0451a97931af8ac036c339f0e61f9a`.
- Exact-source NPU correctness passed for both versions at D=32768 and D=24576. The runner reported `timing_samples=0`.

## Disposition

`BUILD=PASS`; `CORRECTNESS=PASS`; `EXECUTABLE_IDENTITY=PASS`. A fresh MAIN-1 lease used d7 because live HBM was 3431/65536 MB before the run and 3432/65536 MB after the run; d7 had no running process. AICore was recorded as 96.2% before and 95.6% after, and did not affect device admission.

Parent same-binary qualification used the existing device-event runner, 45 warmups, and 21 samples in each of two blocks. D=32768 returned block medians 18.940000/36.580000 us, MAD/median 0.012672/0.024057, and block drift 0.635447: `NOT_QUALIFIED`. D=24576 returned block medians 18.279999/28.780000 us, MAD/median 0.065646/0.325921, and block drift 0.446239: `NOT_QUALIFIED`.

No Parent/Candidate timing was run because neither exact shape passed its Parent noise-floor qualification. `SAME_BINARY=FAIL`; `TIMING=MEASUREMENT_BLOCKED` due to shape-specific Parent instability, not HBM, AICore, VLLM, or a lease conflict. Earlier contaminated P/C values remain excluded. `LOCAL_BEST` and Online state are unchanged. Evidence is under `support/results/d7-20260927T054149Z/`.

Next action: retain V021 as the current Candidate and continue the remaining non-timing dispositions; do not create V022 or submit Online.
