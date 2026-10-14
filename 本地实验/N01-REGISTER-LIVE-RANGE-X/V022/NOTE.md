# N01 V022

- Hypothesis: In the wide FP32 tile loop, the `valueTile` local-tensor handle is used only by `Add` and `Mul`, but its declaration currently precedes both input loads and its lexical scope spans `ReduceSum`. Restricting the handle to the Add/Mul region may reduce live address metadata across the reduction and its following synchronization. Arithmetic and instruction order, GM offsets, tile dimensions, and buffer allocation are unchanged.
- Changed function/range: `ProcessWideFp32FullCacheRows`, Candidate lines 2121-2137. `valueTile` is declared after `SyncMTE2ToV()` and scoped through the two existing arithmetic operations only.
- Parent: `线上结果/R31B/V011/submission.asc`; SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate: `本地实验/N01-REGISTER-LIVE-RANGE-X/V022/Candidate.asc`; SHA256 `2e0d5ecb1b0b2966f2c197e2f686b7d10b2a18a184df56a4cbfed6ee85674052`.
- Device cases that exercise the change: wide FP32 rows, especially `[2, 12288]` as the route target; `[128, 12288]` is a useful same-width diagnostic when the existing runner is available. D=8192 is not the wide-path case.
- CANN 8.5 Full-link: PASS in `/tmp/cann-n01-v022-rW4pnY`; local build log is `logs/build-server3.log`. Parent and Candidate input SHA matched this note; their linked objects and libraries had distinct SHA values.
- Device correctness attempt on d5 used the existing shared runner at 128x12288 FP32. Parent itself returned 1 with `max_abs_error=4.97162247`, `mismatches=1285428`; the runner stopped before Candidate. This same-width Parent failure is also present in the prior N01 route record, so this harness result cannot assess V022.
- Raw failed Parent output, runner identity, device pre-snapshot, and return code are preserved under `logs/correctness-20261014T180815Z/`.
- `CORRECTNESS=INCONCLUSIVE_PARENT_FAIL`; `LOCAL=NONE` and not run. The available runner fixes rows at 128, while the target diagnostic is `[2, 12288]`; no matching existing runner entry was present in this Clone. No performance conclusion is made.
- Next: use the existing project route for the two-row FP32 diagnostic, if available; otherwise retain V022 as compile-only evidence until a valid Parent/Candidate comparison path is available.
