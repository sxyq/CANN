# N02 V006 Handoff

- Candidate source: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V006/submission.asc`
- Candidate source commit: `8eefd778133b449e4e10ca3c35bb6da0d708d099`
- Paired binding: `5bb7d4ec884ed9da20e96c6db9d8884a6ded4e42`
- Full-link binding: `c02b1c5f8884339a60d5042fa651cdd76be38c75`
- Compile: final Parent/Candidate `full_link` and existing paired Runner build all returned 0 under CANN 8.5.0.alpha002.
- Correctness: first launch returned 1 before case execution because the loaded HCC `libstdc++.so.6` lacked `GLIBCXX_3.4.29`; attempt 2 used the system GCC 11 library and Parent/Candidate passed 4/4 cases on d6. Both status/log pairs are retained.
- Local: 4 cases x 21 pairs returned 0. Large timing outliers and active d6 VLLM use make the medians diagnostic only; `LOCAL_SCORE=NONE` and `LOCAL_DELTA=NONE`.
- Device result: object size 334040 B to 333912 B; FP16 hot symbol unchanged; BF16 hot symbol 0x1248 B to 0x1214 B. Linked files have equal size but differ in content.
- Remote run: `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V006/8eefd778133b449e4e10ca3c35bb6da0d708d099`
- Full raw results and device artifacts are in `V006/results/`.
- Next: test a distinct device-side control-flow or block-layout change, independent of duplicate conversion removal and parameter-load scheduling; keep host dispatch unchanged.
