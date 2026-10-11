# N03 V013 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V012 (`cb7b93f28a1e9f4ae31006d1f2822902df73af23` source)
- Candidate source commit: `6ec7cd1d52d95c9e3dcb5140ef3a1cf9f036d674`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V013/submission.asc`
- Candidate SHA-256: `3eb0b8ac3084cf488e4ccfb125da16525d55c83ec48f0849f547c59614d4d2a7`; remote file matches.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent` (`R31B_V011_SOURCE_DIR`).
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V013/6ec7cd1d/candidate` (`R31B_V016_SOURCE_DIR`).
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V013/6ec7cd1d`.

V013 changes only BF16 full-tile affine issue grouping to waves of up to three rows. Per-row arithmetic remains `Muls -> Mul -> Add`; conversion, DMA, events, traversal, and addresses are unchanged.

## Run Status

- CANN `8.5.0.alpha002` Adapter `full_link`: PASS; paired Runner `paired_runner_v016` build: PASS.
- BF16 Correctness: 4/4 Parent/Candidate golden comparisons PASS across two cases; exact matches and zero mismatches.
- Local: `NONE`. Reused V011 same-binary results: both d6144 and d4096 are `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; no V013 timing was started.
- d3 snapshots, Correctness raw output, compile logs, and source SHA records are saved under this V013 directory.
