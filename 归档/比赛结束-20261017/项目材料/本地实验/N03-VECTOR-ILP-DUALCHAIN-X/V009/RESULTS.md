# N03 VECTOR-ILP-DUALCHAIN-X V009 Results

## Identity

- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V009/submission.asc`
- Candidate source commit: `8dd29687a2bd6be4cf2c100fd545392b17bb7737`
- Parent revision: V008 (`4c95711c487a576e037e601dee6430817b4505f0`)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: emit each row's BF16 full-tile `Muls -> Mul -> Add` chain immediately before its `FromFloat` and `Store`, using the existing alternating output slots and event IDs. This lets asynchronous MTE3 output transfer overlap Vector work for the next row. Arithmetic, DMA calls, event calls, tile traversal, and addresses are unchanged.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V009/8dd29687`

## Compile

- Adapter CANN 8.5.0.alpha002 configuration and `full_link`: PASS, SSH return codes `0`.
- Paired Runner CANN 8.5.0.alpha002 configuration and `paired_runner_v016` build: PASS, SSH return codes `0`.
- Paired Runner configure emitted the existing runtime search path warning for system/toolkit `libstdc++.so.6`; build passed and runtime used `LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6`.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- `bf16-cache-params-d6144`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08`; Parent/Candidate mismatches `0`.
- Runner return code: `0`.

## Local

- Same-binary Parent qualification: `paired_runner_v016 --qualify-v011`, 62 samples per case. `d6144`: median `11.930 us`, MAD/median `0.22464`, block drift `0.01509`, result `NEEDS_VALIDATION`; `d4096`: median `14.660 us`, MAD/median `0.37449`, block drift `0.19100`, result `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- Paired Local: `paired_runner_v016 --paired`, 21 Parent/Candidate pairs per case; both Parent-match checks passed.
- `d6144`: Parent median `15.580 us`, Candidate median `11.900 us`, paired-delta median `-0.520 us`; delta range `-108.440–14.240 us`; Parent/Candidate CV `1.18084/0.40085`.
- `d4096`: Parent median `16.580 us`, Candidate median `13.360 us`, paired-delta median `-2.220 us`; delta range `-17.700–110.460 us`; Parent/Candidate CV `0.40884/1.23664`.
- Both `LOCAL_SCORE=NONE`: Parent qualification did not establish stable timing, paired deltas cross zero, and both cases contain large timing outliers. No samples were removed and no speedup is claimed.
- Before paired Local, d3 showed HBM `63616/65536 MB`, AICore `4%`, temperature `37 C`, and power `123.8 W`; after Local HBM was `63617/65536 MB`, AICore `6%`, temperature `41 C`, and power `128.8 W`. Another user's Python process PID `816980` remained present at `60246 MB`; it was not modified.
- Raw evidence: `results/remote-results/correctness.raw.log`, `same-binary-control.raw.log`, `paired-local.raw.log`, and pre/post device-process snapshots. Compile commands and output are under `results/remote-logs/`.

## Status

- Correctness passed. Local performance is inconclusive; `LOCAL_SCORE=NONE`.
