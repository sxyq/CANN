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
- The N03 same-binary raw logs, including V002 reruns, contain only BF16 cases with 16 rows and widths 6144 and 4096. No other BF16 shape has qualification evidence. d6144 had earlier isolated `PASS` results, followed by `NEEDS_VALIDATION` and blocked results; the latest V011 control is blocked for both d6144 and d4096. Neither shape was retimed for this result.
- The Official case shapes are unknown, so no existing Local case can be matched to their score or bestTime changes. This round has no credible Local result mappable to an Official case; `LOCAL_SCORE=NONE`.

## Next ILP Hypothesis

- Source-level dependency evidence in `submission.asc` is: independent BF16 input `ToFloat` calls feed `Add`, then `Mul`, then `ReduceSum`; the full-tile output chain is `Muls -> Mul -> Add`. V001/V003 and V019 cover the independent conversion orders; V010-V018 cover row stage grouping, wave sizes, and issue order; V020/V021 cover one- and two-row affine waves.
- The compiled device object is ELF `hiipu`; the available generic `llvm-objdump` reports instructions as unavailable, and CANN `msobjdump` exposes ELF metadata rather than a readable instruction listing. Therefore this is a source-level issue hypothesis, not a claim about decoded hardware instructions.
- Next candidate hypothesis, design only: split each 4096-element affine Vector `Mul` and `Add` into two contiguous 2048-element VECCALC segments. Within the existing two-row wave, issue both rows' segments for `Mul`, retain the existing stage wait, then issue both rows' segments for `Add` and retain its existing wait. This changes only affine vector issue granularity; elementwise arithmetic count, per-element `Muls -> Mul -> Add`, Scalar addresses, outer row/tile traversal, DMA, and event synchronization stay unchanged.
- This hypothesis was not implemented or tested in this turn. No new Candidate or device run was started.

## Status

- Adapter and paired Runner builds passed; BF16 Correctness passed for both cases. Local is `NONE`, with no credible mapping to an Official case shape. The next source-level issue hypothesis is recorded above; it has not been implemented or measured.
