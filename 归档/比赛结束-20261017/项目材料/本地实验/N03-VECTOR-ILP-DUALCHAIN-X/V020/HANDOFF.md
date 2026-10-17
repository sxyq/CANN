# N03 V020 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent source revision: V019, commit `c3787b92db9fd889a3195b78ecc7dfc9eb3aeb2c`
- Candidate source commit: `082c91a893dd8b3f92e6ca70a62665c07be8d9af`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V020/submission.asc`
- Candidate SHA-256: `f7c5cade4ee68dedb456ee1f05a27c315456c86d8e067a4f1516853503637722`; remote copy matches.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent` (`R31B_V011_SOURCE_DIR`).
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V020/082c91a8/candidate` (`R31B_V016_SOURCE_DIR`).
- Device: `cann-server3`, host `hwnput3`, physical NPU 3 (`d3`), exposed as device 0 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote result root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V020/082c91a8`.

V020 changes only the BF16 affine issue-wave width to one row. Arithmetic order and count, addresses, traversal, DMA, and synchronization remain unchanged.

## Run Status

- CANN `8.5.0.alpha002` Adapter `full_link`: PASS; existing paired Runner `paired_runner_v016` build: PASS.
- BF16 Correctness: both cases passed Parent Golden and Candidate Golden comparisons, exact match, 4/4 comparisons and zero mismatches.
- Local: `NONE`. Existing V011 controls label d6144 and d4096 `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; V020 timing was not started.
- Build output, d3 snapshots, raw Correctness output, and source transfer record are under `results/`.
