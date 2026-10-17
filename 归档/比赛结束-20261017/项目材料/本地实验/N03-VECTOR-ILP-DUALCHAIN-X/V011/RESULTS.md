# N03 VECTOR-ILP-DUALCHAIN-X V011 Results

## Identity

- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V011/submission.asc`
- Candidate source commit: `95d330e1d366b44561ed465bc8b94915141e7d58`
- Candidate SHA-256: `03435328db74b013f0d15c169d2f29e7a49f62d7200d88e66edacc14a3c3a813`
- Parent revision: V010 (`204775a3d16e6f649a9147579ee3140331b63852` source)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: in the BF16 full-tile batch path, emit all independent row normalization `Muls`, then all row `Mul` operations, then all row `Add` operations. Each row retains the same `Muls -> Mul -> Add` value dependency. Arithmetic, conversion, DMA, events, traversal, and addresses are unchanged.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V011/95d330e1`
- Local and remote Candidate SHA-256 values match.

## Compile

- CANN `8.5.0.alpha002` Adapter configure and `full_link`: PASS, SSH return codes `0`.
- Existing paired Runner configure and `paired_runner_v016` build: PASS, SSH return codes `0`.
- Paired Runner configure printed the existing CMake runtime-search-path warning about system and toolkit `libstdc++.so.6`; the build completed and runtime used `LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6`.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- BF16 comparison used Runner settings `rtol=0.015625`, `atol=0.015625`.
- `bf16-cache-params-d6144`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08`; Parent/Candidate mismatches `0`.
- Runner return code: `0`.

## Local

- Same-binary Parent qualification (`--qualify-v011`), 62 samples per shape:
- `d6144`: median `12.900 us`, MAD/median `0.29922`, block drift `0.13643`, result `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- `d4096`: median `11.900 us`, MAD/median `0.38067`, block drift `0.25378`, result `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- `LOCAL_SCORE=NONE` / measurement protocol unsupported for both shapes. This qualification outcome is the route result; no further sample run is requested for these shapes.
- A paired Local run had already completed before the latest instruction not to start paired Local. Its raw output is preserved without rerun: 21 pairs per case, both Parent-match checks passed. d6144 Parent/Candidate medians `37.300/35.240 us`, paired-delta median `+3.320 us`, range `-49.380–64.560 us`; d4096 `12.080/15.420 us`, paired-delta median `+0.440 us`, range `-367.260–28.920 us`. These noisy observations do not support a performance claim and do not override the unsupported qualification status.
- Before paired Local, d3 showed HBM usage `97%`, AICore `15%`, AIVector `9%`; after Local HBM usage remained `97%`, AICore/AIVector were `7%/4%`. Another user's Python PID `816980` used `60246 MB` in snapshots and was left untouched.
- Raw evidence: `results/remote-results/correctness.raw.log`, `same-binary-control.raw.log`, `paired-local.raw.log`, and device/process snapshots. Compile commands and output are under `results/remote-logs/`.

## Status

- Compile and Correctness passed. `LOCAL_SCORE=NONE`; both tested shapes are marked `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` by the same-binary qualification. No repeat measurement was started after the latest instruction.
