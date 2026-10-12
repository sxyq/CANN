# N03 V021 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V020, source commit `082c91a893dd8b3f92e6ca70a62665c07be8d9af`
- Candidate source commit: `882a0b0bc49c6e17844236c8b9501118b32b23bf`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V021/submission.asc`
- Candidate SHA-256: `3742d0979b631cc3b4b174161bc72c5bec4b9fa32331bb5c101250b295793927`
- Device: `cann-server3`, host `hwnput3`, physical NPU 3 (`d3`), exposed with `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote result root: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V021/3742d097`.

V021 changes the BF16 affine issue-wave width from one row to two. Descending row issue order within each stage is retained. Arithmetic count and per-row dependency order, Scalar address generation, row/tile traversal, DMA, events, and synchronization remain unchanged.

## Run Status

- Candidate source commit `882a0b0bc` was pushed to the existing N03 branch.
- CANN `8.5.0.alpha002` Adapter configure and `full_link`: PASS. The build log reaches `[100%] Built target full_link`; its SSH return status was not captured after the interrupted tool session.
- Existing paired Runner configure and `paired_runner_v016` build: PASS, SSH return codes `0`. CMake emitted its existing `libstdc++.so.6` runtime-search-path warning; the target built successfully.
- BF16 Correctness: PASS for both cases; Parent and Candidate Golden comparisons were 4/4 with zero mismatches.
- Local: `LOCAL_SCORE=NONE`. Existing N03 same-binary records contain only BF16 widths 6144 and 4096; the latest controls mark both unsupported for timing. No other qualified BF16 shape exists in this route's control data, and the Official case shape is unknown, so no credible Local can be mapped to its case score/bestTime. No V021 timing was run.
- Before Correctness, d3 showed HBM usage 97%, AICore/AIVector usage 3%/7%; after Correctness they were 97%, 6%/5%. PID `816980` used 60,246 MB in both snapshots and was left untouched. Snapshots and raw output are under `results/remote-results/`.
- Next design hypothesis: split the two-row affine 4096-element Vector `Mul`/`Add` issue into two contiguous 2048-element segments while retaining existing stage waits and event use. It is based on source-level call dependencies; the available tools did not decode the `elf64-hiipu` object into hardware instructions. This remains unimplemented and unmeasured; no new revision or device run was started.

## Continuation Resource Record

- Read-only server3 snapshot at `2026-10-12T11:54:31Z`: the filesystem containing `/home/data4t2/lelinfeng/cann/server_runs` reported 3.5T total, 3.3T used, 6.6G available, and 100% use.
- Physical NPU 3 reported HBM 91% of 65,536 MB, AICore 8%, AIVector 8%, and HBM bandwidth 31%.
- The existing V021 directory retains `build-adapter/device.alink`, `build-adapter/submission.alink`, and `build-paired/paired_runner_v016`. No saved profile or disassembly output appeared in the N03 V021 artifacts. The existing V021 result records that available `llvm-objdump` and `msobjdump` did not expose readable device instructions.
- No new build, profile, Correctness, or Local run was started because the run filesystem reported 100% use. No process was changed.
- V021 remains the latest Candidate: source SHA-256 `3742d0979b631cc3b4b174161bc72c5bec4b9fa32331bb5c101250b295793927`; recorded Correctness remains PASS and Local remains NONE. No new Candidate or source revision was created.
- Next evidence-driven hypothesis, design only: if profiling identifies the full-tile BF16 affine `Mul`/`Add` stage as a material cost center, changing only its 4096-to-two-contiguous-2048 call granularity may alter the generated vector schedule. Preserve the existing row order and waits; confirm the compiled change before attributing any performance effect. No new device run can be recorded from this resource snapshot.
