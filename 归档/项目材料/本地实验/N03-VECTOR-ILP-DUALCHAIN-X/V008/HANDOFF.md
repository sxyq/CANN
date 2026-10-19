# N03 V008 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V007 (`bef53c21` source; `4d0c2744` evidence)
- Candidate source commit: `4c95711c487a576e037e601dee6430817b4505f0`
- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V008/submission.asc`
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V008/4c95711c`

V008 completes each BF16 full-tile row's `Muls -> Mul -> Add` emission before moving to the next row. Relative to V007, only the independent bias `Add` emission moved into the row loop. Arithmetic, DMA calls, event calls, tile traversal, and addresses are unchanged.

## Run status

- CANN 8.5.0.alpha002 Adapter `full_link`: PASS; paired Runner `paired_runner_v016`: PASS.
- BF16 Correctness: PASS, 2/2 cases; Parent/Candidate exact match, zero mismatches.
- Parent qualification: d6144 `NEEDS_VALIDATION` after 62 samples, median `16.790 us`, MAD/median `0.22633`, block drift `0.19893`; d4096 `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`, median `9.040 us`, MAD/median `0.35730`, block drift `0.01549`.
- Paired Local: 21 pairs per case, both Parent-match checks PASS. d6144 Parent/Candidate medians `15.000/10.660 us`, paired-delta median `-2.420 us`; d4096 `13.140/12.740 us`, paired-delta median `-3.100 us`. Both paired-delta ranges cross zero; `LOCAL_SCORE=NONE`.
- Device context: before paired Local d3 had HBM `61114/65536 MB`, AICore `3%`, `39 C`, `118.8 W`; after Local HBM was `61115/65536 MB`, AICore `7%`, `41 C`, `132.8 W`. Another user's Python process PID `816980` used `57744 MB` in the snapshots and was left untouched.
- Raw compile, Correctness, qualification, paired Local, and device/process evidence is under `results/remote-logs/` and `results/remote-results/`.
