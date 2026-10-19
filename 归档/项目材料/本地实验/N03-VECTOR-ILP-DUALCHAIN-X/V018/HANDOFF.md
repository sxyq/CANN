# N03 V018 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent source revision: V017, commit `204c505eb1b38c7c2d60e18ce145099efa4602bf`
- Candidate source commit: `686e6bce93cf1812e835917675b595843f0ccd0d`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V018/submission.asc`
- Candidate SHA-256: `dd58d7a38aed1a801391c827906f65f0bd8c470e00d781061da3e6425b71034d`; remote copy matches.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent` (`R31B_V011_SOURCE_DIR`).
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V018/686e6bce/candidate` (`R31B_V016_SOURCE_DIR`).
- Device: `cann-server3`, host `hwnput3`, physical NPU 3 (`d3`), exposed as device 0 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote result root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V018/686e6bce`.

V018 changes only the issue order of independent BF16 normalization `Muls` rows, traversing rows in reverse. Affine `Mul/Add` order, `Muls -> Mul -> Add` dependency, arithmetic count, address generation, traversal, DMA, and event behavior remain unchanged.

## Run Status

- CANN `8.5.0.alpha002` Adapter `full_link`: PASS; existing paired Runner `paired_runner_v016` build: PASS.
- BF16 Correctness: both cases passed Parent Golden and Candidate Golden comparisons, exact match, 4/4 comparisons and zero mismatches.
- Local: `NONE`. Existing V011 same-binary controls for d6144 and d4096 report `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; no V018 timing was run.
- Pre/post correctness d3 snapshots and raw Runner output are saved in `results/remote-results/`; build output and source transfer record are in `results/remote-logs/`.
