# N03 V017 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent source revision: V016, commit `a8da9ab693747be173856a60b239c0745ef48e52`
- Candidate source commit: `87d4af6df0edba392513fc1e44dbfa54732c4825`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V017/submission.asc`
- Candidate SHA-256: `e5a18575f5736d604a571419f75a1961bd9b93db4ad2244108aeda0c8027d7eb`; remote copy matches.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent` (`R31B_V011_SOURCE_DIR`).
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V017/87d4af6d/candidate` (`R31B_V016_SOURCE_DIR`).
- Device: `cann-server3`, host `hwnput3`, physical NPU 3 (`d3`), exposed as device 0 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote result root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V017/87d4af6d`.

V017 changes only the issue order of independent rows in the BF16 affine `Add` wave, traversing rows in reverse. Per-row `Muls -> Mul -> Add` order, arithmetic count, address generation, traversal, DMA, and synchronization behavior remain unchanged.

## Run Status

- CANN `8.5.0.alpha002` Adapter `full_link`: PASS; paired Runner `paired_runner_v016` build: PASS.
- BF16 Correctness: both cases passed Parent Golden and Candidate Golden comparisons, exact match, 4/4 comparisons and zero mismatches.
- Local: `NONE`. V011 same-binary controls for d6144 and d4096 both report `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; no V017 timing was started.
- Pre/post correctness d3 snapshots and raw Runner output are stored in `results/remote-results/`; build output and source transfer record are in `results/remote-logs/`.
