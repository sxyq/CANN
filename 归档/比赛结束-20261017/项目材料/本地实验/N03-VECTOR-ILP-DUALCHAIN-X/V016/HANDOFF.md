# N03 V016 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent source revision: V015, commit `fb7f0b69b9bfe7d9dae4d4a2233329a5a95981e0`
- Candidate source commit: `a8da9ab693747be173856a60b239c0745ef48e52`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V016/submission.asc`
- Candidate SHA-256: `4ca9b98a1e209ecaa04d45bbffeb44544d20273d26ab6f76914167a1766a29b3`; remote copy matches.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent` (`R31B_V011_SOURCE_DIR`).
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V016/a8da9ab6/candidate` (`R31B_V016_SOURCE_DIR`).
- Device: `cann-server3`, host `hwnput3`, physical NPU 3 (`d3`), exposed as device 0 through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote result root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V016/a8da9ab6`.

V016 changes only the issue order of independent rows in the BF16 affine `Mul` wave, traversing rows in reverse. It retains the V015 wait placement, the `Muls -> Mul -> Add` chain, arithmetic count, address generation, traversal, DMA, and hard-event behavior.

## Run Status

- CANN `8.5.0.alpha002` Adapter `full_link`: PASS; paired Runner `paired_runner_v016` build: PASS.
- BF16 Correctness: both cases passed Parent Golden and Candidate Golden comparisons, exact match, 4/4 comparisons and zero mismatches.
- Local: `NONE`. V011 same-binary controls for d6144 and d4096 both report `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; no V016 timing was started.
- Pre/post correctness d3 snapshots and raw Runner output are stored in `results/remote-results/`; build output and source transfer record are in `results/remote-logs/`.
