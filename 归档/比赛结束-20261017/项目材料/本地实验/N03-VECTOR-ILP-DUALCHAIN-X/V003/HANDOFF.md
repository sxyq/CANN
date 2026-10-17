# N03 V003 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- Candidate commit: `b6cd00ca`
- Candidate source SHA256: `faac3b1c85969849782400528a333d9235fa1ec5813a2ccaeb772b81743e9d26`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V003/b6cd00ca`

V003 changes one full-tile BF16 parameter-cast order and has passed CANN 8.5 compile plus Parent/Candidate correctness for `[16,6144]` and `[16,4096]`. Both outputs match exactly. Existing same-binary qualification and 21-pair Local completed for both cases; device noise makes both Local scores `NONE`.

Evidence files:

- `results/remote-results/correctness.raw.log`
- `results/remote-results/pre-correctness-device.txt`
- `results/remote-results/same-binary-control.raw.log`
- `results/remote-results/paired-local.raw.log`
- `results/remote-results/pre-paired-device.txt`
- `results/remote-results/post-paired-device.txt`
- `results/remote-logs/compile-adapter.log`
- `results/remote-logs/compile-paired.log`
