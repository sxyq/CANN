# N03 V004 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V003 (`b6cd00ca`)
- Candidate source commit: `07c5795f718a6791f11ad23ae9854c8a97d6c4b1`
- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V004/submission.asc`
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`); the runner's ACL index 0 is mapped with `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V004/07c5795f`

V004 groups the independent BF16 full-tile row `Muls` operations before the first row's affine chain. Per-row values and arithmetic remain unchanged. DMA calls, event calls, tile traversal, addresses, and operation counts remain unchanged. This differs from V003's independent parameter-cast order.

## Run status

- CANN 8.5 Adapter full-link compile: PASS
- CANN 8.5 paired Runner compile: PASS after using the existing kernel include filename; the first attempt's missing-include output is retained
- BF16 Correctness: PASS, both cases, zero Parent/Candidate mismatches
- Same-binary Parent qualification: completed, both shapes marked `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`
- Paired Local: completed, 21 pairs per case, both `LOCAL_SCORE=NONE` due to device and sample variability
- Raw commands, outputs, and device/process snapshots: `results/remote-logs/` and `results/remote-results/`

Continue with V005 on the same Vector ILP axis using a distinct independent-chain emission hypothesis. Keep the device on physical d3.
