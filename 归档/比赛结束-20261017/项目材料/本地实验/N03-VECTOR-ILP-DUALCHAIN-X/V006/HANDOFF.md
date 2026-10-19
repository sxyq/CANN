# N03 V006 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V005 (`26c7fad0`)
- Candidate source commit: `af1d1aa0c41023c49b71bf032775e3707ed5d0be`
- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V006/submission.asc`
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V006/af1d1aa0`

V006 keeps V005's independent row `Muls` and affine `Mul` grouping, then groups the independent bias `Add` operations across rows before output conversion. Arithmetic, DMA calls, event calls, tile traversal, and addresses remain unchanged.

## Run status

- CANN 8.5 Adapter full-link compile: PASS
- CANN 8.5 paired Runner compile: PASS
- BF16 Correctness: PASS, both cases, zero Parent/Candidate mismatches
- Same-binary Parent qualification: d6144 `NEEDS_VALIDATION`; d4096 `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`
- Paired Local: completed, 21 pairs per case; both `LOCAL_SCORE=NONE` because Parent qualification and sample variation do not support a performance conclusion
- Raw commands, outputs, and device/process snapshots: `results/remote-logs/` and `results/remote-results/`

Continue with V007 on the Vector ILP axis using a distinct interleaved row-chain emission order. Keep physical device d3.
