# N03 V015 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent source revision: V014, commit `5fd43e14a74e93b78e4ba20c43f186720ed06b8e`
- Candidate source commit: `fb7f0b69b9bfe7d9dae4d4a2233329a5a95981e0`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V015/submission.asc`
- Candidate SHA-256: `a67102cd4d9c2d0a4d0e1f973c37b48635c3fe363a2af6933c7c6db4d95fbc6e`; remote copy matches.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent` (`R31B_V011_SOURCE_DIR`).
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V015/fb7f0b69/candidate` (`R31B_V016_SOURCE_DIR`).
- Device: `cann-server3`, host `hwnput3`, physical NPU 3 (`d3`), exposed as device 0 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote result root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V015/fb7f0b69`.

V015 changes only Vector wait placement in the BF16 full-tile batch path. Independent row `Muls` operations now share a stage-end `PIPE_V` wait; each affine wave shares one wait after its `Mul` stage and one after its `Add` stage. Arithmetic order and count, address generation, row/tile traversal, DMA, and hard-event calls remain unchanged.

## Run Status

- CANN `8.5.0.alpha002` Adapter `full_link`: PASS; paired Runner `paired_runner_v016` build: PASS.
- BF16 Correctness: both cases passed Parent Golden and Candidate Golden comparisons, exact match, 4/4 comparisons and zero mismatches.
- Local: `NONE`. V011 same-binary controls for d6144 and d4096 both report `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; no V015 timing was started.
- Pre/post correctness d3 snapshots and raw Runner output are stored in `results/remote-results/`; build output and source transfer record are in `results/remote-logs/`.
