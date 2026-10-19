# N03 V009 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V008 (`4c95711c` source; `6973718c` evidence)
- Candidate source commit: `8dd29687a2bd6be4cf2c100fd545392b17bb7737`
- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V009/submission.asc`
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V009/8dd29687`

V009 emits each row's BF16 `Muls -> Mul -> Add` chain immediately before that row's `FromFloat` and `Store`, using the existing alternating output slots and event IDs. The row's asynchronous MTE3 store can overlap Vector work for the next row. Arithmetic, DMA calls, event calls, tile traversal, and addresses are unchanged.

## Run status

- CANN 8.5.0.alpha002 Adapter `full_link`: PASS; paired Runner `paired_runner_v016`: PASS.
- BF16 Correctness: PASS, 2/2 cases; Parent/Candidate exact match, zero mismatches.
- Parent qualification: d6144 `NEEDS_VALIDATION` after 62 samples, median `11.930 us`, MAD/median `0.22464`, block drift `0.01509`; d4096 `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`, median `14.660 us`, MAD/median `0.37449`, block drift `0.19100`.
- Paired Local: 21 pairs per case, both Parent-match checks PASS. d6144 Parent/Candidate medians `15.580/11.900 us`, paired-delta median `-0.520 us`; d4096 `16.580/13.360 us`, paired-delta median `-2.220 us`. Both delta distributions cross zero and include large outliers; `LOCAL_SCORE=NONE`.
- Device context: before paired Local d3 had HBM `63616/65536 MB`, AICore `4%`, `37 C`, `123.8 W`; after Local HBM was `63617/65536 MB`, AICore `6%`, `41 C`, `128.8 W`. Another user's Python process PID `816980` used `60246 MB` in the snapshots and was left untouched.
- Raw compile, Correctness, qualification, paired Local, and device/process evidence is in `results/remote-logs/` and `results/remote-results/`.
