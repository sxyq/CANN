# N03 VECTOR-ILP-DUALCHAIN-X V015 Results

## Identity

- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V015/submission.asc`
- Candidate source commit: `fb7f0b69b9bfe7d9dae4d4a2233329a5a95981e0`
- Candidate SHA-256: `a67102cd4d9c2d0a4d0e1f973c37b48635c3fe363a2af6933c7c6db4d95fbc6e`
- Parent source revision: V014 (`5fd43e14a74e93b78e4ba20c43f186720ed06b8e`)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: combine the per-row Vector waits at BF16 normalization and affine wave stage boundaries. The `Muls -> Mul -> Add` dependency order and count are preserved. Scalar address generation, row/tile traversal, DMA, hard-event use, and arithmetic are unchanged.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent`.
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V015/fb7f0b69/candidate`.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote result root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V015/fb7f0b69`.
- Local and remote Candidate SHA-256 values match.

## Compile

- CANN `8.5.0.alpha002` Adapter configure and `full_link`: PASS, SSH return codes `0`.
- Existing paired Runner configure and `paired_runner_v016` build: PASS, SSH return codes `0`.
- Paired configure printed the existing CMake runtime-search-path warning for system/toolkit `libstdc++.so.6`; the target built successfully.
- Raw command/output files: `results/remote-logs/compile-adapter-configure.{command,log}`, `compile-adapter-build.{command,log}`, `compile-paired-configure.{command,log}`, and `compile-paired-build.{command,log}`.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- BF16 Runner settings: `rtol=0.015625`, `atol=0.015625`.
- `bf16-cache-params-d6144`: Parent Golden PASS, Candidate Golden PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent Golden PASS, Candidate Golden PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08`; Parent/Candidate mismatches `0`.
- Total: 4/4 Parent/Candidate golden comparisons passed; Runner return code `0`.
- Raw output and pre/post d3 process snapshots: `results/remote-results/correctness.raw.log`, `pre-correctness-snapshot.log`, and `post-correctness-snapshot.log`.

## Local

- `LOCAL_SCORE=NONE`; no V015 paired Local command was run.
- Reused V011 same-binary qualification for the same shapes: d6144, 62 samples, MAD/median `0.29922`, block drift `0.13643`; d4096, 62 samples, MAD/median `0.38067`, block drift `0.25378`. Both are labeled `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- Control raw output remains at `../V011/results/remote-results/same-binary-control.raw.log`. No timing was repeated for V015.
- Before Correctness, d3 showed HBM usage `97%`, AICore `6%`, AIVector `5%`; after Correctness HBM usage remained `97%`, AICore/AIVector were `7%/3%`. Python PID `816980` used `60246 MB` in both snapshots and was left untouched.

## Status

- Compile and Correctness passed. The existing same-binary control does not support Local timing on either shape; `LOCAL_SCORE=NONE`.
