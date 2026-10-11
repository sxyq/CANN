# N03 V002 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- Candidate commit: `5d9a02ea070af8cc4cb76c384500b1241b0eb054`
- V001 source commit: `9be20982612bbc3a4f409731a88cbee4098e48ca`
- Support commit for independent golden reporting: `44afd954`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V002/5d9a02ea`

## Status

V002 was compiled with CANN 8.5 and run on d0 through the existing paired Runner. Both Parent and Candidate failed the current golden comparison with the same 58 BF16 errors, while the two device outputs matched exactly. The run ended with return code `3` and no timing was started.

## Evidence files

- `results/remote-results/candidate-correctness.raw.log`
- `results/remote-results/candidate-correctness.command`
- `results/remote-results/pre-correctness-device.txt`
- `results/remote-logs/compile-adapter.log`
- `results/remote-logs/compile-paired.log`

## Next action

Do not claim a V002 Local score. Keep this negative result and the Parent golden mismatch, then continue only after the accepted correctness path is reconciled.
