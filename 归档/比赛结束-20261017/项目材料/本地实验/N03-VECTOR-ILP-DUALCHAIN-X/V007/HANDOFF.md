# N03 V007 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V006 (`af1d1aa0`)
- Candidate source commit: `bef53c218131c1deeb5cef39129f88d0cf690989`
- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V007/submission.asc`
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V007/bef53c21`

V007 emits each independent row's BF16 scaling `Muls` immediately followed by its affine `Mul`, then proceeds to the next row. Bias `Add` emissions remain grouped as in V006. Arithmetic, DMA calls, event calls, tile traversal, and addresses are unchanged.

## Run status

- CANN 8.5.0.alpha002 Adapter `full_link`: PASS; paired Runner `paired_runner_v016`: PASS.
- BF16 Correctness: PASS, 2/2 cases; Parent/Candidate exact match, zero mismatches.
- Same-binary Parent qualification: both shapes returned `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` after 62 samples. d6144 median `29.660 us`, MAD/median `0.67532`, block drift `0.45314`; d4096 median `13.940 us`, MAD/median `0.55022`, block drift `1.01004`.
- Paired Local: 21 pairs per case, both Parent-match checks PASS. d6144 Parent/Candidate medians `15.000/15.520 us`, paired-delta median `+0.220 us`; d4096 `12.760/12.440 us`, paired-delta median `+0.200 us`. Delta ranges cross zero and sample variation is high; `LOCAL_SCORE=NONE`.
- Device: physical NPU 3 (`d3`) via `ASCEND_RT_VISIBLE_DEVICES=3`. Before Local: HBM `61115/65536 MB`, AICore `12%`, `32 C`, `137.6 W`; after Local AICore was `9%`, HBM unchanged. Another user's Python process PID `816980` used `57744 MB` throughout the captured snapshots and was left untouched.
- Raw build, correctness, qualification, paired Local, and device/process evidence is in `results/remote-logs/` and `results/remote-results/`.
