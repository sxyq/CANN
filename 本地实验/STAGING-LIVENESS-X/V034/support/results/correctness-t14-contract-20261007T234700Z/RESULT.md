# STAGING-LIVENESS-X V034 T14 Contract Recheck

DATE: 2026-10-07
ROUTE: STAGING-LIVENESS-X
REVISION: V034 (frozen)
CASE: route-local synthetic T14; not an official case mapping

## Contract

- Launch: device 1, dtype 0 (FP32), shape `[2,1,2,32768]`, epsilon `1e-5`, one warmup, one checked invocation.
- The wrapper loaded 524288 bytes each for `x`, `residual`, and output, and 131072 bytes each for `gamma` and `bias`. Input shape metadata is rank 4 with dimensions `[2,1,2,32768]`; parameter metadata is rank 1 with dimension `[32768]`.
- `run_kernel` flattens the leading dimensions to 4 rows, uses width 32768, and clamps the reported 40 vector cores to 4 blocks. The Parent and Candidate runner logs identify the expected source SHA and device.
- T14 seed is 341047. Input, residual, gamma, bias, and golden SHA-256 values match between Parent and Candidate and between the original full-suite run and this targeted rerun. The saved golden was independently recomputed from the binary inputs using `y=x+residual`, row-wise `mean(y*y)`, epsilon `1e-5`, and `y/sqrt(meanSquare+epsilon)*gamma+bias`; the recomputed FP32 golden matched the saved golden exactly.
- A route-local harness correction now preserves each case's original suite index when `--case T14` is selected. The all-cases seed mapping is unchanged. An initial targeted attempt without the CANN runtime environment exited 127 before kernel execution and is retained under `correctness-t14-contract-20261007T234229Z/`.

## Dispatch And Results

Width 32768 exceeds `kCacheElems` (8192), so both sources dispatch through `ProcessWideFp32` to `ProcessWideFp32FullCacheRows`. V034's only source change moves a wait in `ProcessFp32FullRowOutputPipelined`, which this T14 dispatch does not enter.

The corrected targeted rerun completed successfully at the runner level but failed the numerical check for both exact sources:

| Source | Original full-suite matched ratio / max error | Targeted rerun matched ratio / max error |
|---|---:|---:|
| V033 Parent `0df982248c80c4f2e5541dc730b0a25ba97544efc3b3e3e3409c24deed2d2eff` | 0.172195 / 3.505801 | 0.166946 / 3.430017 |
| V034 Candidate `75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33` | 0.155510 / 3.418998 | 0.166084 / 3.462913 |

Although each repeated run used identical fixture and golden hashes, its actual-output SHA changed for both Parent and Candidate. The Parent is therefore not a valid T14 correctness oracle, and the common wide-FP32 path exhibits unstable incorrect output under this fixture. The available evidence localizes the failure to that shared kernel path; it does not prove the precise device-side synchronization defect.

## Disposition

- No T14 shape, buffer-size, dtype, epsilon, golden, or `run_kernel` ABI mismatch was found. The isolated-case seed selection bug is fixed, and the corrected T14 rerun reproduces the failure.
- V034 is `CORRECTNESS_FAILED` for this route-local synthetic suite; this is not an isolated Parent-versus-Candidate regression and is not a claim about the unpublished official case mapping.
- Local remains `NOT_RUN`; no timing result is claimed. V034 source copies remain frozen at the recorded SHA.
- No valid STAGING-LIVENESS-X Local Best is evidenced in this worktree: V001-V004 are `NOT_COMPLETE` / `SUSPENDED_FOR_W4`, and no later route `local-result.json` exists. Do not invent a base or start another revision until Planning identifies a valid Local Best and resolves the required V034 disposition.
- The original Parent/Candidate T01-T15 outputs and the failed environment-only attempt remain preserved. No shared record or route lifecycle state was changed.

Evidence: `parent/`, `candidate/`, and the original `correctness-pair-20261007T231845Z/` run.
