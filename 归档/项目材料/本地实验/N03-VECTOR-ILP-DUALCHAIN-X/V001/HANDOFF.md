# N03 V001 Handoff

- Route: `N03-VECTOR-ILP-DUALCHAIN-X`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n03-vector-ilp-dualchain`
- Branch: `route/n03-vector-ilp-dualchain-x-20261011`
- Parent: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- Candidate commit: `9be20982612bbc3a4f409731a88cbee4098e48ca`
- Support commit for independent golden reporting: `44afd954`
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V001/9be20982612bbc3a4f409731a88cbee4098e48ca`

## Status

The V001 Kernel source is committed and compiled with CANN 8.5. The independent device run completed on d0. Parent and Candidate both failed the current golden comparison with the same 58 BF16 errors, while their device outputs matched exactly. The earlier exact-compare output alone contained only `PARENT_MATCH PASS`; the later golden-check output is the authoritative Candidate result for this decision.

Paired Local was intentionally not started after the independent golden failure. The existing same-binary Parent control remains raw evidence only and must not be reported as a V001 Candidate score.

## Evidence files

- `results/remote-results/candidate-correctness-systemlib.raw.log`
- `results/remote-results/candidate-correctness-systemlib.command`
- `results/remote-results/correctness-exactcompare.raw.log`
- `results/remote-results/same-binary-control.raw.log`
- `results/remote-results/pre-candidate-correctness-device.txt`
- `results/remote-logs/compile-adapter.log`
- `results/remote-logs/compile-paired-candidate-golden.log`

## Next action

Do not claim V001 Local. Keep the 58-error Parent/Candidate golden failure as a negative result, reconcile the oracle or Parent behavior with the accepted correctness path, then proceed with the next single Vector instruction-emission hypothesis under the same Route boundary.
