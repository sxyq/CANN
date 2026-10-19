# N03 V005 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V004 (`07c5795f`)
- Candidate source commit: `26c7fad0d30410be64eb04c9c7b33dc33500baab`
- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V005/submission.asc`
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V005/26c7fad0`

V005 keeps V004's independent row `Muls` grouping and adds one independent emission-order change: group the full-tile BF16 row `Mul` operations before the per-row bias `Add` stage. Arithmetic, DMA calls, event calls, tile traversal, and addresses are unchanged.

## Run status

- CANN 8.5 Adapter full-link compile: PASS
- CANN 8.5 paired Runner compile: PASS
- BF16 Correctness: PASS, both cases, zero Parent/Candidate mismatches
- Same-binary Parent qualification: completed; both shapes marked `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`
- Paired Local: completed, 21 pairs per case; both `LOCAL_SCORE=NONE` because device samples varied substantially
- Raw commands, outputs, and device/process snapshots: `results/remote-logs/` and `results/remote-results/`

Continue with V006 on the Vector ILP axis using an independent-chain emission change distinct from V003, V004, and V005. Keep physical device d3.
