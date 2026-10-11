# N03 V002 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- Candidate commit: `5d9a02ea070af8cc4cb76c384500b1241b0eb054`
- V001 source commit: `9be20982612bbc3a4f409731a88cbee4098e48ca`
- Support commit for independent golden reporting: `44afd954`
- Correctness support commits: `3ff6771e`, `8fb972fb`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V002/5d9a02ea`

## Status

V002 was compiled with CANN 8.5 and run on d0 through the existing paired Runner. The first run used the old `0.001` gate and preserved the common 58-error result. After switching the existing entry to the stable double-precision RMS reference and BF16 mixed tolerance, Parent and Candidate both passed with `failures=0`, `matched_ratio=1.0`, and exact output match. No timing was started.

## Evidence files

- `results/remote-results/candidate-correctness.raw.log`
- `results/remote-results/candidate-correctness.command`
- `results/remote-results/correctness-mixed-tolerance.raw.log`
- `results/remote-results/correctness-mixed-tolerance.command`
- `results/remote-results/pre-correctness-device.txt`
- `results/remote-results/pre-mixed-tolerance-device.txt`
- `results/remote-logs/compile-adapter.log`
- `results/remote-logs/compile-paired.log`
- `results/remote-logs/compile-paired-mixed-tolerance.log`

## Next action

Do not claim a V002 Local score yet. Keep the old failure and new PASS evidence, then continue with the next pure Vector ILP single-point change.
