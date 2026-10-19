# N03 VECTOR-ILP-DUALCHAIN-X V012 Results

## Identity

- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V012/submission.asc`
- Candidate source commit: `cb7b93f28a1e9f4ae31006d1f2822902df73af23`
- Candidate SHA-256: `1be243a51dd7a7f45d051051b766549d3a71f302f5de7ff596d2b91c59259c66`
- Parent revision: V011 (`95d330e1d366b44561ed465bc8b94915141e7d58` source)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: in the BF16 full-tile batch path, affine `Mul` and `Add` instructions issue in two-row waves after the existing row normalization stage. Every row retains `Muls -> Mul -> Add`. Arithmetic, conversion, DMA, event handling, traversal, and addresses are unchanged.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent`, configured as `R31B_V011_SOURCE_DIR`.
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V012/cb7b93f2/candidate`, configured as `R31B_V016_SOURCE_DIR`.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V012/cb7b93f2`.
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

- `LOCAL_SCORE=NONE`; V012 paired Local was not run.
- The existing V011 same-binary qualification is reused for these same shapes: d6144, 62 samples, MAD/median `0.29922`, block drift `0.13643`; d4096, 62 samples, MAD/median `0.38067`, block drift `0.25378`. Both were labeled `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- V011 control raw is retained at `../V011/results/remote-results/same-binary-control.raw.log`. No qualification or paired timing was repeated for V012.
- Before Correctness, d3 showed HBM usage `97%`, AICore `6%`, AIVector `3%`; after Correctness it showed HBM usage `97%`, AICore `10%`, AIVector `8%`. Another user's Python process PID `816980` used `60246 MB` in both snapshots and was left untouched.
- Raw evidence: `results/remote-results/correctness.raw.log` and pre/post device/process snapshots. Compile commands and output, plus source SHA confirmation, are under `results/remote-logs/`.

## Status

- Compile and Correctness passed. Local timing is unsupported for the two existing shapes; `LOCAL_SCORE=NONE`.
- Normal push advanced `origin/route/n03-vector-ilp-dualchain-x-20261011` to `fff0755bd` after this evidence commit.
