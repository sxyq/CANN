# N03 V011 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V010 (`204775a3d16e6f649a9147579ee3140331b63852` source)
- Candidate source commit: `95d330e1d366b44561ed465bc8b94915141e7d58`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V011/submission.asc`
- Candidate SHA-256: `03435328db74b013f0d15c169d2f29e7a49f62d7200d88e66edacc14a3c3a813`
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V011/95d330e1`
- Remote Candidate SHA-256 matches the local source.

V011 changes only BF16 full-tile Vector issue order: independent row normalization, affine multiply, and bias-add stages are each issued across the row batch. Per-row arithmetic dependency remains `Muls -> Mul -> Add`; conversion, DMA, events, row/tile traversal, and addresses are unchanged.

## Run Status

- CANN `8.5.0.alpha002` Adapter `full_link`: PASS; paired Runner `paired_runner_v016`: PASS.
- BF16 Correctness: PASS, 2/2 cases; Parent/Candidate exact match and zero mismatches.
- Same-binary qualification: d6144 62 samples, median `12.900 us`, MAD/median `0.29922`, block drift `0.13643`; d4096 62 samples, median `11.900 us`, MAD/median `0.38067`, block drift `0.25378`. Both results are `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`; `LOCAL_SCORE=NONE` / protocol unsupported.
- Paired Local raw had already been produced before the latest instruction against further Local runs. It is retained as historical evidence only and was not repeated. Both Parent-match checks passed; distributions were noisy and cross zero. See `RESULTS.md` and raw logs.
- Device snapshots record d3 only. Another user's Python PID `816980` remained untouched.
