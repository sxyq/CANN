# DTYPE-SPECIAL-X V001 Handoff

## State

- Route: `DTYPE-SPECIAL-X`
- Revision: `V001` (existing pending Candidate)
- Local disposition: `NEEDS_ONE_MORE_LOCAL`
- Branch: `exec/sixlane-20260924-dtype-special-x`
- Candidate dtype: FP32
- Direct Parent: `R31B-V011`, score `45.16`
- Candidate source: `phase4/local/DTYPE-SPECIAL-X/V001/submission.asc`
- Candidate source SHA-256: `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f`

## Retained evidence

- CPU correctness: 39/39 PASS, `logs/cpu-correctness.json`.
- CANN compile and link: PASS on CANN `8.5.0.alpha002`, SoC `Ascend910B3`, NPU arch `dav-2201`; records are in `source-meta.json` and `logs/server3-source-env-20260924T190042Z-build-link.log`.
- NPU correctness: 39/39 PASS on device 4; maximum absolute error `2.6226044e-06`; timing was not run. Log: `logs/server3-source-env-20260924T190042Z-correctness.log`.
- The local source, workspace source, sidecar, and final build source record all report SHA-256 `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f`.

## Executable identity

Read-only inspection of the existing V001 directory on `cann-server3` found:

| Path | SHA-256 | Size | Purpose |
|---|---|---:|---|
| `/home/data4t2/lelinfeng/phase4-workspaces/DTYPE-SPECIAL-X/V001/build/dtype_special_x_v001` | `14cdc6cf6f986fd49f92662b4ff180266d0265cafdc0145706e37153b984a90b` | 466832 bytes | ASC compile/link smoke target. Its `compile_adapter.asc` entry point returns immediately and does not launch the kernel. |
| `/home/data4t2/lelinfeng/phase4-workspaces/DTYPE-SPECIAL-X/V001/build/dtype_special_x_v001_npu_correctness` | `3688342faf10c080f869517a9e5a7721349dccd775e8545c60753e761aee1655` | 475112 bytes | Candidate NPU correctness runner. It runs the 39 correctness cases, copies outputs to host, and has no performance timing loop. |

Both identities were read from the remote files without executing them. Neither executable is suitable as the unified paired performance runner. No Parent executable identity is recorded here.

## Paired input definition

The existing V001 NPU correctness runner defines a deterministic FP32 workload with 12 rows per case:

- Prefixes: `[12]`, `[3, 4]`, `[1, 2, 6]`; append each width below to form the 2-D, 3-D, and 4-D shapes.
- Widths: `1, 7, 8, 9, 63, 64, 65, 127, 128, 129, 1024, 4096, 8192`.
- Total: 39 rank/width cases.
- For `i` in the maximum input allocation: `x[i] = 0.19 * sin(((i * 17) % 997) * 0.013)` and `residual[i] = 0.11 * cos(((i * 29) % 991) * 0.017)`.
- For column `c`: `gamma[c] = 0.85 + (c % 23) * 0.002` and `bias[c] = -0.025 + (c % 19) * 0.001`.
- Epsilon: `1e-5`; available core count: `8`.

These are the existing correctness inputs. Main must confirm whether this complete set is the intended local performance set. Parent and Candidate must receive identical tensors, shapes, epsilon, and core count through one Main-approved paired runner; its Parent and Candidate executable identities must be recorded before timing.

## Conditions for another local run

- No active exclusive lease for `DTYPE-SPECIAL-X` appears in the latest readable lease ledger. The entries present for devices 4-6 are released or free; this Route has no lease entry. Do not run timing until Main records and grants its exclusive lease.
- Use the unified protocol: one long-lived process, allocations and H2D before warmup, at least 10 warmup launches with synchronization, at least 21 timed samples, and at least 4 adjacent interleaved Parent/Candidate pairs.
- Record device-event duration as the primary metric and host wall duration as a diagnostic. Keep D2H and correctness comparison outside the timed loop. Record device load and lease identity with the results.
- Use the same runner revision and input buffers for both executables. Do not substitute either executable listed above for that paired runner.

## Main review point

Main to confirm the performance workload set, provide or approve the unified paired runner and Parent executable identity, then grant an exclusive device lease. Until those are available, V001 remains `NEEDS_ONE_MORE_LOCAL`; no performance claim is made.

No V002, dtype change, hypothesis change, Candidate source change, or online submission is included in this handoff.
