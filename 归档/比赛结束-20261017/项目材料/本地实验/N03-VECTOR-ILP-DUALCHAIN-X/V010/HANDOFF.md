# N03 V010 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent revision: V009 (`8dd2968794b32917b90053fa834c27d64e188c02` source)
- Candidate source commit: `204775a3d16e6f649a9147579ee3140331b63852`
- Candidate source: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain/本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V010/submission.asc`
- Candidate SHA-256: `1d17d9abdbdce2216e5dce1ca3ff8be5cff49829ee588227d292a9597e3d6faa`
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`), mapped through `ASCEND_RT_VISIBLE_DEVICES=3`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V010/204775a3`

V010 changes only the BF16 full-tile batch Vector issue order: independent row normalization `Muls` instructions are emitted across rows before the per-row affine `Mul -> Add` chains. Arithmetic, conversion, DMA, synchronization, tile traversal, and addresses remain unchanged.

## Run Status

- CANN `8.5.0.alpha002` Adapter `full_link`: PASS; paired Runner `paired_runner_v016`: PASS.
- BF16 Correctness: PASS, 2/2 cases, Parent/Candidate exact match, zero mismatches.
- Parent qualification: d6144 `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` (62 samples, median `14.730 us`, MAD/median `0.28174`, block drift `0.24576`); d4096 `NEEDS_VALIDATION` (62 samples, median `15.660 us`, MAD/median `0.13091`, block drift `0.08940`).
- Paired Local: 21 pairs per case, both Parent-match checks PASS. d6144 Parent/Candidate medians `12.040/10.320 us`, delta median `-1.900 us`, range `-41.580–13.500 us`; d4096 `18.560/11.480 us`, delta median `-3.560 us`, range `-48.320–52.280 us`. Both distributions cross zero and contain large outliers; `LOCAL_SCORE=NONE`.
- Device context before/after paired Local: HBM usage `97%` both times; AICore/AIVector `6%/3%` before and `20%/26%` after. Another user's Python PID `816980` used `60246 MB` and was left untouched.
- Raw Compile, Correctness, Parent qualification, paired Local, and device/process evidence is stored in this V010 directory under `results/remote-logs/` and `results/remote-results/`.
- Normal push created `origin/route/n03-vector-ilp-dualchain-x-20261011`; the remote ref was `d15bc465e56f3c79a94a43656c482b80829f6e3b` after the evidence commit push.
