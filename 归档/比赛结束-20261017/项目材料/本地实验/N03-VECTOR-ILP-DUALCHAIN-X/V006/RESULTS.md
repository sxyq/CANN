# N03 VECTOR-ILP-DUALCHAIN-X V006 Results

## Identity

- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V006/submission.asc`
- Candidate source commit: `af1d1aa0c41023c49b71bf032775e3707ed5d0be`
- Parent revision: V005 (`26c7fad0`)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: group independent BF16 full-tile bias `Add` emissions across rows before output conversion; no arithmetic, DMA, event calls, tile traversal, or address changes.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`)
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V006/af1d1aa0`

## Compile

- Adapter CANN 8.5 configuration and `full_link`: PASS, return codes `0`.
- Paired Runner CANN 8.5 configuration and `paired_runner_v016` build: PASS, return codes `0`.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 via `ASCEND_RT_VISIBLE_DEVICES=3`.
- `bf16-cache-params-d6144`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08` against reference, Parent/Candidate mismatches `0`.
- Runner return code: `0`.

## Local

- Same-binary Parent qualification: 62 samples per case. `d6144`: median `11.870 us`, MAD/median `0.21062`, block drift `0.21230`, result `NEEDS_VALIDATION`; `d4096`: median `15.180 us`, MAD/median `0.25758`, block drift `0.03557`, result `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- Paired Local: 21 Parent/Candidate pairs per case; both passed output matching.
- `d6144`: Parent median `15.120 us`, Candidate median `12.240 us`, paired-delta median `-0.740 us`; raw delta range `-11.480–8.540 us`.
- `d4096`: Parent median `10.180 us`, Candidate median `7.200 us`, paired-delta median `-0.280 us`; raw delta range `-17.460–16.100 us`.
- Both `LOCAL_SCORE=NONE`. Parent qualification did not establish stable timing, and paired deltas varied across positive and negative values. No samples were removed and no speedup is claimed.
- `ASCEND_RT_VISIBLE_DEVICES=3` maps the runner's ACL index `0` to assigned physical NPU 3 (`d3`). Before Correctness d3 showed HBM `59870/65536 MB` and AICore `40%`; before the Parent control AICore was `44%`, and before paired Local AICore was `39%` with HBM `59871/65536 MB`. Another user's Python training process remained present and unchanged.
- Raw evidence: `results/remote-results/correctness.raw.log`, `same-binary-control.raw.log`, `paired-local.raw.log`, and `*-snapshot.log` files.

## Status

- Correctness passed. Local performance remains inconclusive, so score is `NONE`.
