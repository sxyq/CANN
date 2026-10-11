# N03 VECTOR-ILP-DUALCHAIN-X V013 Results

## Identity

- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V013/submission.asc`
- Candidate source commit: `6ec7cd1d52d95c9e3dcb5140ef3a1cf9f036d674`
- Candidate SHA-256: `3eb0b8ac3084cf488e4ccfb125da16525d55c83ec48f0849f547c59614d4d2a7`
- Parent revision: V012 (`cb7b93f28a1e9f4ae31006d1f2822902df73af23` source)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: in the BF16 full-tile batch path, issue affine `Mul` and `Add` in waves of up to three rows after normalization. Each row retains `Muls -> Mul -> Add`. Arithmetic, conversion, DMA, event handling, traversal, and addresses are unchanged.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent`, configured as `R31B_V011_SOURCE_DIR`.
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V013/6ec7cd1d/candidate`, configured as `R31B_V016_SOURCE_DIR`.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V013/6ec7cd1d`.
- Local and remote Candidate SHA-256 values match.

## Compile

- CANN `8.5.0.alpha002` Adapter configure and `full_link`: PASS, SSH return codes `0`.
- Existing paired Runner configure and `paired_runner_v016` build: PASS, SSH return codes `0`.
- Paired Runner configure printed the existing CMake runtime-search-path warning about system/toolkit `libstdc++.so.6`; the target built successfully.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- BF16 comparison used Runner settings `rtol=0.015625`, `atol=0.015625`.
- `bf16-cache-params-d6144`: Parent Golden PASS, Candidate Golden PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent Golden PASS, Candidate Golden PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08`; Parent/Candidate mismatches `0`.
- Total: 4/4 Parent/Candidate golden comparisons passed; Runner return code `0`.

## Local

- `LOCAL_SCORE=NONE`; V013 paired Local and qualification were not run.
- Reused V011 same-binary qualification for these same shapes: d6144, 62 samples, MAD/median `0.29922`, block drift `0.13643`; d4096, 62 samples, MAD/median `0.38067`, block drift `0.25378`. Both were labeled `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- V011 control raw is retained at `../V011/results/remote-results/same-binary-control.raw.log`. No timing run was repeated for V013.
- Before Correctness, d3 showed HBM usage `97%`, AICore `0%`, AIVector `67%`; after Correctness HBM usage remained `97%`, AICore/AIVector were `15%/12%`. Another user's Python process PID `816980` used `60246 MB` in both snapshots and was left untouched.
- Raw evidence: `results/remote-results/correctness.raw.log` and pre/post device/process snapshots. Compile commands, output, and source SHA evidence are under `results/remote-logs/`.

## Status

- Compile and Correctness passed. The existing control marks both shapes unsupported for Local timing; `LOCAL_SCORE=NONE`.
