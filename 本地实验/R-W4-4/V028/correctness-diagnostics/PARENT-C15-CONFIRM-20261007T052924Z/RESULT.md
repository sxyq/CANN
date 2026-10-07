# Exact V027 Parent C15 confirmation

- ROUTE / REVISION: `R-W4-4 / MODE-DISPATCH-CUTOFF-X / V028`
- DIRECT PARENT: V027
- HOST: `hwnput3`
- DEVICE: 7, 910B3
- SHAPE / DTYPE: C15, `1x32768`, FP32
- COMMAND: existing `clx_ref_parent_probe 7 1 32768 0 <prefix> 0 1 1 0 1`
- RESULT: return code `3`; `bad=25332`; `max_abs=1.2031`
- START / END: `2026-10-07T05:29:24.614007135Z` / `2026-10-07T05:29:36.779681941Z`

## Identities

- Exact Parent `parent.asc`: `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`
- Route runner `runner_ref.inc`: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`
- Parent wrapper `runner_ref_parent.asc`: `ce5e6b47de1ff74741b57b24e817aac10a4968e52a41c168c4f1d2627d619910`
- Existing executable `clx_ref_parent_probe`: `ddb9e0ba4f628380c9be9028f9d7c9cdc5dc89aadf6a66e14d50712054a26319`
- Repaired diagnostic Parent (not used): `c3007e431d75f3123b8f9d45cbbabaf6440b23596bd26b665a79d12f6ce5dde8`

## Device context

- Pre: HBM `24790/65536 MB` used (`40746 MB` free), AICore `0%`, AIVector `88%`.
- Post: HBM `24792/65536 MB` used (`40744 MB` free), AICore `0%`, AIVector `88%`.
- Device 7 retained the same Python process, PID `439848`, using `21260 MB`; it was not modified.
- Full pre/post `npu-smi` and process-memory snapshots are retained beside this report.

## Disposition

BLOCKER: `TOOLING/CORRECTNESS BLOCKER` — the Route is blocked on a valid exact-Parent correctness baseline. The exact Parent failure reproduced, while high AIVector utilization remains an observed load confounder. `LOCAL_SCORE=NONE` (not zero); no performance comparison is admitted. Stop before Candidate correctness or Local; Candidate remains frozen. No repaired Parent, source edit, V029 work, Online action, or shared-record write occurred.

Next minimal diagnostic: repeat only this exact-source C15 probe with the same route-bound executable on a device showing low/zero AIVector use and no active route lease; retain the same identity and pre/post load snapshots. Do not attempt Local unless the exact Parent passes.
