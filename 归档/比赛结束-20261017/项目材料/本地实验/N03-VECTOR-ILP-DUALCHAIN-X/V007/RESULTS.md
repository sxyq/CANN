# N03 VECTOR-ILP-DUALCHAIN-X V007 Results

## Identity

- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V007/submission.asc`
- Candidate source commit: `bef53c218131c1deeb5cef39129f88d0cf690989`
- Parent revision: V006 (`af1d1aa0`)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: interleave independent full-tile BF16 row `Muls` and affine `Mul` emissions row by row; bias `Add` remains grouped. No arithmetic, DMA, event calls, tile traversal, or address changes.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`)
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V007/bef53c21`

## Compile

- Adapter CANN 8.5.0.alpha002 configuration and `full_link`: PASS, return code `0`.
- Paired Runner CANN 8.5.0.alpha002 configuration and `paired_runner_v016` build: PASS, return code `0`.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- `bf16-cache-params-d6144`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08`; Parent/Candidate mismatches `0`.
- Runner return code: `0`.

## Local

- Same-binary Parent qualification: `paired_runner_v016 --qualify-v011`, 62 samples per case. `d6144`: median `29.660 us`, MAD/median `0.67532`, block drift `0.45314`, result `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; `d4096`: median `13.940 us`, MAD/median `0.55022`, block drift `1.01004`, result `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- Paired Local: `paired_runner_v016 --paired`, 21 Parent/Candidate pairs per case; both Parent-match checks passed.
- `d6144`: Parent median `15.000 us`, Candidate median `15.520 us`, paired-delta median `+0.220 us`; paired-delta range `-16.400–16.400 us`; Parent/Candidate CV `0.50474/0.35865`.
- `d4096`: Parent median `12.760 us`, Candidate median `12.440 us`, paired-delta median `+0.200 us`; paired-delta range `-13.140–23.100 us`; Parent/Candidate CV `0.44480/0.49759`.
- Both `LOCAL_SCORE=NONE`: Parent qualification did not establish stable timing, paired deltas cross zero, and sample variation is high. No samples were removed and no speedup is claimed.
- `ASCEND_RT_VISIBLE_DEVICES=3` maps the runner's ACL device `0` to assigned physical NPU 3 (`d3`). Before paired Local, d3 showed HBM `61115/65536 MB`, AICore `12%`, temperature `32 C`, and power `137.6 W`; after Local, HBM remained `61115/65536 MB` and AICore was `9%`. Another user's Python process PID `816980` remained present at `57744 MB`; it was not modified.
- Raw evidence: `results/remote-results/correctness.raw.log`, `same-binary-control.raw.log`, `paired-local.raw.log`, and the pre/post device-process snapshot logs. Compile commands and output are under `results/remote-logs/`.

## Status

- Correctness passed. Local performance is inconclusive; `LOCAL_SCORE=NONE`.
