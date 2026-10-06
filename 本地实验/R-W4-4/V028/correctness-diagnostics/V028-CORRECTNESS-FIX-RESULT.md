# V028 correctness-fix follow-up

- ROUTE: R-W4-4 / MODE-DISPATCH-CUTOFF-X
- REVISION: V028
- DIRECT_PARENT: V027
- OBSERVED_UTC: 2026-10-06T23:10:55Z
- SCOPE: correctness-only; no Local, performance edit, shared-record edit, or Online

## Source and runner identity

| object | SHA256 | binding |
|---|---|---|
| exact Parent `parent.asc` | `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033` | `runner_ref_parent.asc` defines `SRX_SUBMISSION "parent.asc"` |
| patched Candidate `submission.asc` | `9fc8ded08c6a9f0dc1392dcc46bbe6572f0c66fd65df81f57a2c18700a295c37` | `runner_ref_candidate.asc` defines `SRX_SUBMISSION "submission.asc"` |
| route-bound Parent probe | `ddb9e0ba4f628380c9be9028f9d7c9cdc5dc89aadf6a66e14d50712054a26319` | built from the same V028 stage |
| route-bound Candidate probe | `c1c83462a105e33ccf179bb847f3ca1a9e48542c75f89dd3f867e28c34fce541` | rebuilt after the fix with HCC include paths |
| direct runner layer | `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7` | V028 `runner_ref.inc` |

The shared `runner_main.inc` is absent from this V028 stage. That is a genuine `TOOLING_BLOCKER` for the shared generic runner. The executed probes are the available route-bound `runner_ref_*` wrappers and are therefore source-bound to this worktree. The runner generates deterministic harness inputs: x uses `InputValue(i,37,11)`, residual uses `InputValue(i,17,3)`, gamma uses `0.75 + ((i*13)%100)/200`, and bias uses `((i*7)%100)/400 - 0.125`, with dtype conversion before H2D. These are not official Judge input bytes.

## Minimal fix

The only source edit after the original V028 result is one `SyncVToMTE2()` at the end of each wide-FP32 output tile in `ProcessWideFp32FullCacheRows`, before `xBuf_` and `residualBuf_` are refilled by the next gamma/bias MTE2 loads. The preceding vector operations still read those staging buffers as gamma/bias, so the event drains the V use before the MTE2 overwrite.

This is a correctness/lifetime repair only. The V028 performance change remains exactly `kSmallFp32BatchMaxWidth` 512 -> 256.

## Retest evidence

Compile verification: rebuilding `clx_ref_candidate_probe` from the V028 route-bound stage with the installed CANN 8.5.0 toolchain and the HCC C++ include paths exited 0 (`C15-candidate-patched-device2-20261006T2300Z/compile-retry.log`). A first invocation in a fresh shell omitted `CPLUS_INCLUDE_PATH` and failed in generated host plugin compilation with `fatal error: 'vector' file not found`; the corrected build succeeded and produced the same probe SHA `c1c83462…`. This is an environment-only failed attempt, not a kernel compile failure.

The local CANN 8.5.0 API-doc directory was absent, so no target-version documentation claim is made. The fix uses the source's existing `SyncVToMTE2()` helper (`SetFlag/WaitFlag` on `V_MTE2`) and the corrected source compiled successfully for the target.

The route-bound patched Candidate on device 2, `dtype=FP32`, passed all 16 existing cases:

- `C01..C16`: every process return code `0`, recorded in `CANDIDATE-FULL-device2-20261006T2305Z/return-codes.tsv`.
- Boundary `D=8193`: `bad=0`, `max_abs=1.66893e-06`.
- Boundary `D=16384`: `bad=0`, `max_abs=8.58307e-06`.
- `D=32768`, repeat 1: `bad=0`, `max_abs=3.57628e-06`.
- `D=32768`, repeat 2: `bad=0`, `max_abs=3.57628e-06`.

The probe also emits a single timing sample per case; those values are incidental to correctness execution and are not Local performance evidence.

The exact Parent, through the same route-bound runner and same generated input construction on device 2, still fails:

- `D=32768`: `rc=3`, `bad=30306`, `max_abs=1.2031`.
- Source binding and command evidence: `PARENT-direct-device2-20261006T2315Z/`.

Earlier preserved evidence independently shows the same Parent failure on device 7, device-2 width sweep failure at `D=8193/16384/32768`, and variable Parent bad counts across repeated `D=32768` runs. The patched Candidate's transition from failure to repeated zero-bad results localizes the defect to the wide-FP32 buffer lifetime/synchronization path and justifies this minimal direct fix.

## Exact disposition

- `CANDIDATE_CORRECTNESS`: `PASS` for the route-bound 16-case matrix and targeted wide-FP32 boundaries after the fix.
- `PARENT_CORRECTNESS`: `FAIL` on the exact V027 source, same route-bound runner, device 2 and prior device 7 evidence.
- `V028_FULL_GATE`: `HARD_BLOCKED / PARENT_SHARED_FAILURE`.
- `LOCAL`: prohibited while the exact Parent fails.
- `V029`: prohibited; no new performance change.
- `ONLINE`: prohibited.
- Shared TSVs, route lifecycle, and other Routes: untouched.

This follow-up does not overwrite `RESULT-CLOSURE.md`; that file remains the original pre-fix result. The current conclusion is precise: the V028 Candidate correctness defect is repaired, but V028 cannot be promoted through the full gate because its exact Direct Parent has a pre-existing wide-FP32 correctness failure. The next permitted action requires a Planning/Review decision or a separately authorized Parent repair, not Local measurement.
