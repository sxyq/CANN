# N03 VECTOR-ILP-DUALCHAIN-X V003 Results

## Identity

- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V003/submission.asc`
- Candidate source SHA256: `faac3b1c85969849782400528a333d9235fa1ec5813a2ccaeb772b81743e9d26`
- Candidate commit: `b6cd00ca`
- Parent commit: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Kernel change: in `ProcessBf16FullTileBatchedOutputPipelined`, swap only the independent `biasFp32` and `gammaFp32` `ToFloat` emission order. Scalar address generation, Tile traversal, DMA calls, synchronization, arithmetic, and data volume are unchanged.
- Device: `cann-server3`, `hwnput3`, device `d0` / runtime device index `0`
- Remote run root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca`
- Support commits: `3ff6771e`, `8fb972fb`, `08060862`

## Compile and Correctness

- Adapter CANN 8.5 full-link compile: PASS, configure `0`, build `0`.
- Paired Runner CANN 8.5 compile: PASS, configure `0`, build `0`.
- Correctness entry: existing `paired_runner_v016 --correctness`, two BF16 cases.
- `bf16-cache-params-d6144`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, matched ratio `1.000000000`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, matched ratio `1.000000000`, max absolute error `1.4901161e-08`.
- BF16 tolerance: `rtol=0.015625`, `atol=0.015625`, `max_abs_limit=1`.

## Local

- Same-binary Parent qualification: completed for both cases, 62 samples each. `d6144`: median `10.560 us`, MAD/median `0.10701`, block drift `0.12121`; `d4096`: median `7.640 us`, MAD/median `0.10340`, block drift `0.09424`. Both reported `NEEDS_VALIDATION`.
- Paired Local: completed with 21 Parent/Candidate pairs per case. `d6144`: Parent median `10.480 us`, Candidate median `11.080 us`, paired delta median `+0.180 us`; `d4096`: Parent median `7.040 us`, Candidate median `7.120 us`, paired delta median `-0.220 us`.
- Both Local scores: `NONE`. The raw paired samples contain substantial excursions (including `35.860 us` on d6144 and `343.980 us` / `200.480 us` on d4096); control MAD/block drift also indicates unstable measurement. No samples were removed.
- Raw control, paired samples, and pre/post device/process snapshots are preserved under `results/remote-results/`.
- The old 0.001-gate failures remain preserved in V001/V002; the corrected mixed-tolerance correctness output is in `results/remote-results/correctness.raw.log`.

## Next action

Treat V003 as correctness-passing with inconclusive Local performance. Do not claim a speedup from either case. A later revision must remain a distinct Vector ILP emission-order hypothesis.
