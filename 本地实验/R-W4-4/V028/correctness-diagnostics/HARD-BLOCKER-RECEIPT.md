# HARD-BLOCKER-RECEIPT — V028

- ISSUED_UTC: 2026-10-06T23:10:55Z
- ROUTE / REVISION: R-W4-4 / MODE-DISPATCH-CUTOFF-X / V028
- BLOCKER: `PARENT_SHARED_FAILURE`
- TOOLING: shared `runner_main.inc` absent (`TOOLING_BLOCKER`); direct route-bound `runner_ref_parent.asc` and `runner_ref_candidate.asc` were used instead.

## Proof

- Parent binding: `runner_ref_parent.asc` -> `parent.asc` -> SHA `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`.
- Parent command: `clx_ref_parent_probe 2 1 32768 0 <prefix> 0 1 1 0 1`.
- Parent result: `rc=3`, `bad=30306`, `max_abs=1.2031`; evidence: `PARENT-direct-device2-20261006T2315Z/`.
- Candidate binding: `runner_ref_candidate.asc` -> `submission.asc` -> SHA `9fc8ded08c6a9f0dc1392dcc46bbe6572f0c66fd65df81f57a2c18700a295c37`.
- Candidate direct matrix: `C01..C16` all `rc=0`; C15 repeated twice at D=32768 with `bad=0`, `max_abs=3.57628e-06`; evidence: `CANDIDATE-FULL-device2-20261006T2305Z/` and `C15-candidate-patched-device2-20261006T2300Z/`.

## Diagnosis

The one-line V→MTE2 drain fixes the Candidate's wide-FP32 staging-buffer lifetime hazard. The exact Parent still fails on the same route-bound runner and generated input contract, so the remaining gate failure is an existing Parent defect, not a Candidate-only V028 regression and not an input/device mismatch.

## Prohibited next actions

No Local, no V029, no performance change, no shared-record edit, no other-Route modification, and no Online. Preserve all evidence. Route remains available for Planning/Review direction or separately authorized Parent repair.
