# N03 VECTOR-ILP-DUALCHAIN-X V021 Results

## Identity

- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V021/submission.asc`
- Candidate source commit: `882a0b0bc49c6e17844236c8b9501118b32b23bf`
- Candidate SHA-256: `3742d0979b631cc3b4b174161bc72c5bec4b9fa32331bb5c101250b295793927`
- Parent source revision: V020 (`082c91a893dd8b3f92e6ca70a62665c07be8d9af`)
- Measurement reference: R31B V011 (`1c51a5b2`)
- Change: increase BF16 affine issue-wave width from one row to two, retaining descending row issue order within each stage. Per-row arithmetic and dependency order, Scalar address generation, traversal, DMA, events, and synchronization remain unchanged.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent`.
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V021/3742d097/candidate`.
- Local and remote Candidate SHA-256 values match.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote result root: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V021/3742d097`.

## Compile

- CANN `8.5.0.alpha002` Adapter configure: PASS, SSH return code `0`.
- Adapter `full_link`: PASS; complete output ends with `[100%] Built target full_link`. The SSH return status was not captured after the interrupted tool session.
- Existing paired Runner configure and `paired_runner_v016` build: PASS, SSH return codes `0`.
- Paired configure emitted the existing CMake runtime-search-path warning for system/toolkit `libstdc++.so.6`; the target built successfully.
- Commands and complete output are under `results/remote-logs/`.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 through `ASCEND_RT_VISIBLE_DEVICES=3`: PASS, SSH return code `0`.
- BF16 Runner tolerance: `rtol=0.015625`, `atol=0.015625`.
- `bf16-cache-params-d6144`: Parent Golden PASS, Candidate Golden PASS, exact match PASS; failures `0`, maximum absolute error `0`.
- `bf16-full-tile-d4096`: Parent Golden PASS, Candidate Golden PASS, exact match PASS; failures `0`, maximum absolute error `1.4901161e-08`.
- Total: 4/4 Parent/Candidate Golden comparisons passed; Parent/Candidate mismatches `0`.
- Raw output and pre/post d3 snapshots are under `results/remote-results/`.

## Local

- `LOCAL_SCORE=NONE`; no V021 paired Local command was run for d6144 or d4096.
- V019 records the existing same-binary controls for both shapes as `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`, so the protocol does not support timing them. Original controls remain at `../V011/results/remote-results/same-binary-control.raw.log`; these historical samples are not V021 performance data.

## Status

- Adapter and paired Runner builds passed; BF16 Correctness passed for both cases. Local remains `NONE` under the existing shape controls.
