# N02 V004 Handoff

- Candidate source: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V004/submission.asc`
- Candidate commit: `2746c556ae46b9a67fd9b326701a3e48c2934306`
- Support bind commit: `54d920ed00a8d6a6bdf748ec6df4424362c1d2c6`
- d6 runner commit: `92acba5c697133269e8cf4e05001b46d95fdb2d2`
- Parent source: `线上结果/R31B/V011/submission.asc`
- Compile: Parent/Candidate `full_link` PASS; paired runner build PASS after adding the existing C++ include paths to `CPATH`.
- Correctness: PASS on d6 for four FP16/BF16 cases.
- Local: runner returned 0 for four 21-pair cases; `LOCAL_SCORE=NONE` due large outliers and active VLLM load.
- Device code: FP16 hot symbol grew 44 B and BF16 hot symbol shrank 8 B; total hot-symbol size grew 36 B. Device object and instruction evidence are saved under `results/`.
- Remote run: `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V004/2746c556ae46b9a67fd9b326701a3e48c2934306`
- Next independent hypothesis: test a control-flow layout change in pass 2 parameter prefetch, separate from the pass 1 first-tile preload used here.
