# N03 V014 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V013 (`6ec7cd1d52d95c9e3dcb5140ef3a1cf9f036d674` source)
- Candidate source commit: `5fd43e14a74e93b78e4ba20c43f186720ed06b8e`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V014/submission.asc`
- Candidate SHA-256: `1ba229fc776923089d768c89f4d7aaaca157e7f5c6abe9c20e5a35c5ac0b0e49`; remote file matches.
- Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca/parent` (`R31B_V011_SOURCE_DIR`).
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V014/5fd43e14/candidate` (`R31B_V016_SOURCE_DIR`).
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`.
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V014/5fd43e14`.

V014 changes only BF16 full-tile affine issue grouping to waves of up to four rows. Per-row arithmetic remains `Muls -> Mul -> Add`; conversion, DMA, events, traversal, and addresses are unchanged.

## Run Status

- CANN `8.5.0.alpha002` Adapter `full_link`: PASS; paired Runner `paired_runner_v016` build: PASS.
- BF16 Correctness: 4/4 Parent/Candidate golden comparisons PASS across two cases; exact matches and zero mismatches.
- Local: `NONE`. Reused V011 same-binary results: d6144 and d4096 are both `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; no V014 timing was started.
- d3 snapshots, Correctness raw output, compile logs, and source SHA records are saved under this V014 directory.
