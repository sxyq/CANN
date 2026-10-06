# V028 Parent-shared failure closure receipt

- ISSUED_UTC: 2026-10-06T23:23:35Z
- ROUTE / REVISION: R-W4-4 / MODE-DISPATCH-CUTOFF-X / V028
- WORKTREE: `/home/data4t2/lelinfeng/cann-r-w4-4`
- BRANCH: `routes/r-w4-4-mode-dispatch-cutoff-x`
- HEAD: `61dac40ef9519d8ee621581310beec2bfc2da582`
- STAGE: correctness-only diagnosis
- BLOCKER: `PARENT_SHARED_FAILURE` plus absent shared `runner_main.inc` (`TOOLING_BLOCKER`)

## Identity verification

The usable route-bound wrappers are source-bound and share one runner implementation:

- `runner_ref_parent.asc` defines `SRX_SUBMISSION "parent.asc"`.
- `runner_ref_candidate.asc` defines `SRX_SUBMISSION "submission.asc"`.
- `runner_ref.inc` SHA256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`.
- Exact Parent `parent.asc` SHA256: `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`.
- Patched Candidate `submission.asc` SHA256: `9fc8ded08c6a9f0dc1392dcc46bbe6572f0c66fd65df81f57a2c18700a295c37`.
- Parent probe SHA256: `ddb9e0ba4f628380c9be9028f9d7c9cdc5dc89aadf6a66e14d50712054a26319`.
- Candidate probe SHA256: `c1c83462a105e33ccf179bb847f3ca1a9e48542c75f89dd3f867e28c34fce541`.
- `runner_main.inc` is absent from this V028 stage; no shared generic runner result was used as proof.

The runner uses the same deterministic harness for both probes: x is `InputValue(i,37,11)`, residual is `InputValue(i,17,3)`, gamma is `0.75 + ((i*13)%100)/200`, and bias is `((i*7)%100)/400 - 0.125`, followed by dtype conversion before H2D. These are harness inputs, not claimed official Judge bytes. No Parent/Candidate source or input identity mismatch was found.

## Correctness evidence

- Candidate route-bound matrix `C01..C16`: every return code is `0`; evidence: `CANDIDATE-FULL-device2-20261006T2305Z/return-codes.tsv`.
- Candidate FP32 boundary probes: `D=8193 bad=0`, `D=16384 bad=0`, and repeated `D=32768 bad=0` with `max_abs=3.57628e-06`; evidence: `C15-candidate-patched-device2-20261006T2300Z/`.
- Exact Parent command shape: `clx_ref_parent_probe 2 1 32768 0 <prefix> 0 1 1 0 1`.
- Exact Parent result: `rc=3`, `bad=30306`, `max_abs=1.2031`; evidence: `PARENT-direct-device2-20261006T2315Z/`.
- Prior device-2 width sweep: `D=8192` passed; `D=8193`, `D=16384`, and `D=32768` failed.
- Prior repeated Parent `D=32768` runs have varying bad counts (`C15-determinism-device2-20261006T2250Z/`), consistent with an unsynchronized wide-FP32 buffer lifetime.
- Prior device-7 evidence independently reproduces the Parent failure.

## Diagnosis and disposition

V028's performance change remains exactly `kSmallFp32BatchMaxWidth 512 -> 256`. The only direct correctness fix is the added `SyncVToMTE2()` after each wide-FP32 output tile, before `xBuf_` and `residualBuf_` are refilled for gamma/bias. That fix makes the route-bound Candidate pass the targeted matrix and wide-FP32 boundaries, while the exact V027 Parent still fails under the same runner and input contract.

Therefore the failure is an existing Parent wide-FP32 synchronization/lifetime defect, not a V028 cutoff regression and not a Parent/runner/input identity mismatch. `PARENT_SHARED_FAILURE` remains a hard blocker for the full V028 gate.

Local is prohibited while the exact Parent fails. Do not create V029, add a performance change, modify shared records or another Route, or run Online. The next permitted action is Planning/Review direction or separately authorized repair of the Parent; no further action is justified in this route under the current authorization.
