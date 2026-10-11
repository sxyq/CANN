# N03 V012 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V011 (`95d330e1d366b44561ed465bc8b94915141e7d58` source)
- Candidate source commit: `cb7b93f28a1e9f4ae31006d1f2822902df73af23`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V012/submission.asc`
- Candidate SHA-256: `1be243a51dd7a7f45d051051b766549d3a71f302f5de7ff596d2b91c59259c66`; remote file matches.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent` (`R31B_V011_SOURCE_DIR`).
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V012/cb7b93f2/candidate` (`R31B_V016_SOURCE_DIR`).
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V012/cb7b93f2`.

V012 changes only BF16 full-tile affine instruction issue grouping to two-row waves. Per-row arithmetic remains `Muls -> Mul -> Add`; conversion, DMA, events, traversal, and addresses are unchanged.

## Run Status

- CANN `8.5.0.alpha002` Adapter `full_link`: PASS; paired Runner `paired_runner_v016` build: PASS.
- BF16 Correctness: 4/4 Parent/Candidate golden comparisons PASS across two cases; exact matches and zero mismatches.
- Local: `NONE`. Reused V011 same-binary results: both d6144 and d4096 are `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; no V012 qualification or paired Local was started.
- d3 before/after Correctness snapshots and raw results are saved under `results/remote-results/`; compile and source SHA records are under `results/remote-logs/`.
