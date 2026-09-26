# MAIN Review: R31A V021

Date: 2026-09-27

## Current Evidence

- Direct parent: R31A-V016, source SHA-256 `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0`.
- Candidate source SHA-256: `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063`; server3 copy matches.
- Server3 device and submission targets returned RC 0. Their `.alink` SHA-256 values are `96fd4babb50710c5489071aa4861bc89bd07dc8fa32d309de781cd05fb7c7a4f` and `78cd7c977222adf1ebec22e408a885426f097ac37e0a8f8829877a023f268f7f`.
- Parent and Candidate modules are source-bound: `11d7a2cb60912ce23b05a768c9076abcf2c513ad8b0844cc062d158082a4d5ea` and `9bacd8cd72b8db5c6b8a1574ce199c674b18bc2bcf858cfb91ade29f71d24674`. Current correctness runner SHA-256 is `7dbde489de951be883fe38d6cbc991800c0451a97931af8ac036c339f0e61f9a`.
- Exact-source NPU correctness passed for both versions at D=32768 and D=24576. The runner reported `timing_samples=0`.

## Disposition

No local performance verdict has been formed. `BUILD=PASS`; `CORRECTNESS=PASS`; `EXECUTABLE_IDENTITY=PASS`; `READY_FOR_SAME_BINARY=true`. `TIMING=MEASUREMENT_BLOCKED` is the timing-stage state only: d4-d6 currently have heavy VLLM HBM use, and d7 is excluded from performance timing. Earlier contaminated P/C values are not used. No Local Best change and no Online submission.

Next action: with a fresh exclusive performance lease, run Parent same-binary qualification on the selected exact shape before any paired samples.
