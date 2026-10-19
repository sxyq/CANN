# N05 V029

- Parent: R31B V011 Champion, `线上结果/R31B/V011/submission.asc`
- Parent SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate: `Candidate.asc`
- Candidate SHA-256: `419cc1b3968e7a4a763cd9644ac5c5b234a0f41fc3ebd3c23b1da46311a1d008`
- Target: `AddRmsNormBiasKernel::ProcessWideLowPrecision`, Pass1 unit loop. Parent lines 3125-3211; Candidate lines 3126-3213.
- Hypothesis: replacing per-unit `u / tileCount` and `(u + 1) / tileCount` coordinate recovery with nested row/tile counters reduces scalar address-generation work. Row-major order, alternating buffers, prefetch point, DMA calls, vector operations, and event order are retained.
- Target workload: diagnostic `rows=128,width=12416`, FP16 and BF16. Official shape mapping: UNKNOWN.
- CANN 8.5 Full-link: PASS in `/tmp/cann-n05-v029-18eskH`; Parent and Candidate remote source SHA matched the values above. Parent/Candidate object SHA-256 `a8d7738508bf4d6a61eb6651104396ce85be36a8834e7505aaf8fbb77d200b14` / `8b3e0651507ef548e602b7c5ef42a99fab15e593882795a173a8b54dc9bdcfb4`; Runner SHA-256 `5f96ff61ad5752ba0f4abe375eebdb491aab6dc799f2e604269fecaf22f3bee5`.
- Correctness: Parent and Candidate both PASS at 128x12416 BF16 and FP16; all four runner return codes were 0.
- Local used 45 warmups, 21 samples per block, four paired blocks, and 42 same-binary samples per side. BF16 paired Parent/Candidate median was `22.78/22.90 us`, median paired delta `-0.180%`, Candidate faster in `44/84` pairs; block deltas `-0.341%, -0.609%, +0.360%, +6.472%`. Same-binary Parent/Candidate median/MAD was `23.98/2.22` and `24.71/2.15 us`.
- FP16 paired Parent/Candidate median was `28.48/30.34 us`, median paired delta `+4.787%`, Candidate faster in `34/84` pairs; block deltas `-4.396%, +19.521%, +8.000%, 0.000%`. Same-binary Parent/Candidate median/MAD was `20.77/0.46` and `62.85/26.71 us`, indicating severe variability in the Candidate control.
- d2 HBM usage stayed at 98% (about 871 MB free); AICore/AIVector was 18%/22% before and 0%/58% after, and HBM bandwidth 10%/9%. Shared activity and mixed block directions dominate this measurement.
- `LOCAL_SCORE=NONE`; no repeatable improvement is established. Raw Correctness, paired samples, controls, and device snapshots are under `logs/correctness-20261014T180818Z/` and `logs/local-20261014T181200Z/`.
- Next: inspect scalar instruction evidence from the built device code before considering a different Row/Tile experiment. Do not repeat this loop-counter change on the same shapes without new evidence.
