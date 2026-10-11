# N03 VECTOR-ILP-DUALCHAIN-X V010 Results

## Identity

- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V010/submission.asc`
- Candidate source commit: `204775a3d16e6f649a9147579ee3140331b63852`
- Candidate SHA-256: `1d17d9abdbdce2216e5dce1ca3ff8be5cff49829ee588227d292a9597e3d6faa`
- Parent revision: V009 (`8dd2968794b32917b90053fa834c27d64e188c02` source; `2f65a69b8ba6a12d52320f467a4b4a8769a31a5f` evidence)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: in the BF16 full-tile batch path, issue each independent row's normalization `Muls` before issuing the per-row affine `Mul` and `Add` chains. Arithmetic, tensor values, conversion, DMA, event handling, tile traversal, and addresses are unchanged. This changes Vector issue scheduling only.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V010/204775a3`

## Compile

- CANN `8.5.0.alpha002` Adapter configure and `full_link`: PASS, SSH return codes `0`.
- Existing paired Runner configure and `paired_runner_v016` build: PASS, SSH return codes `0`.
- Paired Runner configure printed the existing CMake runtime-search-path warning about system and toolkit `libstdc++.so.6`; the build completed and runtime used `LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6`.
- The remote Candidate SHA-256 matched the local source SHA-256.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- BF16 comparison used the Runner's configured `rtol=0.015625`, `atol=0.015625`.
- `bf16-cache-params-d6144`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08`; Parent/Candidate mismatches `0`.
- Runner return code: `0`.

## Local

- Same-binary Parent qualification: existing `paired_runner_v016 --qualify-v011`, 62 samples per case.
- `d6144`: median `14.730 us`, MAD/median `0.28174`, block drift `0.24576`, result `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- `d4096`: median `15.660 us`, MAD/median `0.13091`, block drift `0.08940`, result `NEEDS_VALIDATION`.
- Paired Local: existing `paired_runner_v016 --paired`, 21 Parent/Candidate pairs per case; both Parent-match checks passed.
- `d6144`: Parent median `12.040 us`, Candidate median `10.320 us`, paired-delta median `-1.900 us`; paired-delta range `-41.580–13.500 us`; Parent/Candidate CV `0.74155/0.38006`.
- `d4096`: Parent median `18.560 us`, Candidate median `11.480 us`, paired-delta median `-3.560 us`; paired-delta range `-48.320–52.280 us`; Parent/Candidate CV `0.66247/0.81822`.
- Both `LOCAL_SCORE=NONE`: same-binary d6144 timing was blocked, d4096 needs validation, and both paired-delta distributions cross zero with large outliers. No samples were removed and no speedup is claimed.
- Before paired Local, d3 showed HBM usage `97%`, AICore `6%`, AIVector `3%`, HBM bandwidth `1%`. After Local it showed HBM usage `97%`, AICore `20%`, AIVector `26%`, HBM bandwidth `4%`. Another user's Python process PID `816980` used `60246 MB` in both snapshots and was left untouched.
- Raw evidence: `results/remote-results/correctness.raw.log`, `same-binary-control.raw.log`, `paired-local.raw.log`, and pre/post device/process snapshots. Compile commands and output are under `results/remote-logs/`.

## Status

- Compile and Correctness passed. Local performance is inconclusive; `LOCAL_SCORE=NONE`.
