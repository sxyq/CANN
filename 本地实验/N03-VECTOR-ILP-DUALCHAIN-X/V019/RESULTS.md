# N03 VECTOR-ILP-DUALCHAIN-X V019 Results

## Identity

- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V019/submission.asc`
- Candidate source commit: `c3787b92db9fd889a3195b78ecc7dfc9eb3aeb2c`
- Candidate SHA-256: `dd74b9390932974c1ce390a2e8b94e24be046d67c1a78707f9d9d135ac1212c1`
- Parent source revision: V018 (`686e6bce93cf1812e835917675b595843f0ccd0d`)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: reverse the two independent BF16 input conversion issue calls (`ToFloat(residual)` before `ToFloat(x)`). Per-row arithmetic and dependency order remain unchanged. Scalar address generation, row/tile traversal, DMA, events, and synchronization remain unchanged.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent`.
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V019/c3787b92/candidate`.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote result root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V019/c3787b92`.
- Local and remote Candidate SHA-256 values match.

## Compile

- CANN `8.5.0.alpha002` Adapter configure and `full_link`: PASS, SSH return codes `0`.
- Existing paired Runner configure and `paired_runner_v016` build: PASS, SSH return codes `0`.
- Paired configure printed the existing CMake runtime-search-path warning for system/toolkit `libstdc++.so.6`; the target built successfully.
- Command/output records: `results/remote-logs/compile-adapter-configure.{command,log}`, `compile-adapter-build.{command,log}`, `compile-paired-configure.{command,log}`, and `compile-paired-build.{command,log}`.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- BF16 Runner tolerance: `rtol=0.015625`, `atol=0.015625`.
- `bf16-cache-params-d6144`: Parent Golden PASS, Candidate Golden PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent Golden PASS, Candidate Golden PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08`; Parent/Candidate mismatches `0`.
- Total: 4/4 Parent/Candidate golden comparisons passed; Runner return code `0`.
- Raw output and pre/post d3 process snapshots: `results/remote-results/correctness.raw.log`, `pre-correctness-snapshot.log`, and `post-correctness-snapshot.log`.

## Local

- `LOCAL_SCORE=NONE`; no V019 paired Local command was run.
- Reused V011 same-binary qualification for these shapes: d6144, 62 samples, MAD/median `0.29922`, block drift `0.13643`; d4096, 62 samples, MAD/median `0.38067`, block drift `0.25378`. Both report `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- Control raw output remains at `../V011/results/remote-results/same-binary-control.raw.log`. No timing was repeated for V019.
- Before Correctness, d3 showed HBM usage `97%`, AICore `14%`, AIVector `8%`; after Correctness HBM usage remained `97%`, AICore/AIVector were `5%/6%`. Python PID `816980` used `60246 MB` in both snapshots and was left untouched.

## Status

- Compile and Correctness passed. Existing same-binary controls do not support Local timing on either shape; `LOCAL_SCORE=NONE`.
