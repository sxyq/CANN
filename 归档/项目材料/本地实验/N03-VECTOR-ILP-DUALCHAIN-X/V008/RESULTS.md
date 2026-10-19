# N03 VECTOR-ILP-DUALCHAIN-X V008 Results

## Identity

- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V008/submission.asc`
- Candidate source commit: `4c95711c487a576e037e601dee6430817b4505f0`
- Parent revision: V007 (`bef53c218131c1deeb5cef39129f88d0cf690989`)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: in the BF16 full-tile output path, issue each row's `Muls`, affine `Mul`, and bias `Add` before the next row. Relative to V007, only the independent bias `Add` emission moved into the row loop; arithmetic, DMA, event calls, tile traversal, and addresses are unchanged.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V008/4c95711c`

## Compile

- Adapter CANN 8.5.0.alpha002 configuration and `full_link`: PASS, SSH return codes `0`.
- Paired Runner CANN 8.5.0.alpha002 configuration and `paired_runner_v016` build: PASS, SSH return codes `0`.
- Paired Runner configure emitted a CMake runtime-search-path warning about system and toolkit `libstdc++.so.6`; the build completed and runtime used `LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6`.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- `bf16-cache-params-d6144`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08`; Parent/Candidate mismatches `0`.
- Runner return code: `0`.

## Local

- Same-binary Parent qualification: `paired_runner_v016 --qualify-v011`, 62 samples per case. `d6144`: median `16.790 us`, MAD/median `0.22633`, block drift `0.19893`, result `NEEDS_VALIDATION`; `d4096`: median `9.040 us`, MAD/median `0.35730`, block drift `0.01549`, result `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- Paired Local: `paired_runner_v016 --paired`, 21 Parent/Candidate pairs per case; both Parent-match checks passed.
- `d6144`: Parent median `15.000 us`, Candidate median `10.660 us`, paired-delta median `-2.420 us`; paired-delta range `-18.020–10.100 us`; Parent/Candidate CV `0.38626/0.31701`.
- `d4096`: Parent median `13.140 us`, Candidate median `12.740 us`, paired-delta median `-3.100 us`; paired-delta range `-27.580–19.980 us`; Parent/Candidate CV `0.55029/0.45793`.
- Both `LOCAL_SCORE=NONE`: d4096 Parent qualification was blocked, d6144 needs validation, and paired deltas cross zero in both cases. No samples were removed and no speedup is claimed.
- Before paired Local, d3 showed HBM `61114/65536 MB`, AICore `3%`, temperature `39 C`, and power `118.8 W`; after Local HBM was `61115/65536 MB`, AICore `7%`, temperature `41 C`, and power `132.8 W`. Another user's Python process PID `816980` remained present at `57744 MB`; it was not modified.
- Raw evidence: `results/remote-results/correctness.raw.log`, `same-binary-control.raw.log`, `paired-local.raw.log`, and pre/post device-process snapshots. Compile commands and output are under `results/remote-logs/`.

## Status

- Correctness passed. Local performance is inconclusive; `LOCAL_SCORE=NONE`.
