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

The named Direct Parent source artifact, `phase4/online/R31B/V011/submission.asc`, is present in this checkout and has SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, matching the declared parent source SHA. The local Candidate source SHA-256 is `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f`. These source identities do not prove a Parent executable. The route evidence has no Parent binary path, binary SHA-256, or build record connecting an executable to the declared Parent source.

## Paired input decision

Use the 24 correctness shapes inside the task's legal width range for both same-binary noise-floor runs and P/C paired measurements. The shared task ABI allows FP32 rank-2/3/4 inputs with `D=64..32768`; the existing NPU correctness suite has eight such widths, each tested with all three rank prefixes:

- Prefixes: `[12]`, `[3, 4]`, `[1, 2, 6]`; append `D` to form `[12, D]`, `[3, 4, D]`, and `[1, 2, 6, D]`.
- Widths: `64, 65, 127, 128, 129, 1024, 4096, 8192`.
- Total: 24 rank/width cases; each has 12 rows after flattening the leading dimensions.
- For `i = 0..98303`, initialize `x[i] = 0.19f * sin(float((i * 17) % 997) * 0.013f)` and `residual[i] = 0.11f * cos(float((i * 29) % 991) * 0.017f)`. For a case of width `D`, use the first `12 * D` values from each input array.
- For column `c = 0..8191`, initialize `gamma[c] = 0.85f + float(c % 23) * 0.002f` and `bias[c] = -0.025f + float(c % 19) * 0.001f`. For a case of width `D`, use the first `D` parameter values.
- Epsilon: `1e-5f`; `availableCoreNum`: `8`; one ACL stream. With 12 rows, the entry point launches `min(8, 12) = 8` blocks for every case.
- ABI: FP32 `x`, `residual`, and `output` share the case shape; FP32 `gamma` and `bias` each have shape `[D]`. The output shape matches the input. The operation is `u=x+residual`, `rms=sqrt(mean(u*u, last_dimension)+epsilon)`, `output=(u/rms)*gamma+bias`.

The selected widths include the legal 64/128 alignment points, adjacent misaligned widths, and medium/wide cases; all three ranks retain the task's host metadata and flattening paths while keeping the device row count fixed. The patterns are deterministic, so both binaries receive byte-identical tensors and attributes. The five widths below the task minimum (`1, 7, 8, 9, 63`) remain in the 39-case correctness suite but are excluded from task-domain performance results; including them would give out-of-domain cases weight in the performance comparison.

The existing V001 NPU runner is correctness-only: it copies inputs before each launch, synchronizes, copies output back, and compares the golden result after each call. It has no warmup/sample timing loop or event measurement. The compile smoke entry point returns immediately. Neither is compliant with the local timing protocol, and no Route-local timing runner is present. The recorded Candidate NPU correctness executable identity is not a timing-runner identity.

## Conditions for another local run

- The Route's tracked lease-ledger snapshot was last updated on 2026-09-24 and has no `DTYPE-SPECIAL-X` grant. It cannot establish a current lease for this resumption. Main must grant an exclusive device lease before timing.
- Main must provide or approve one unified runner for the same-binary and P/C workloads. Do not use the correctness executable, the compile smoke executable, or another Route's runner.
- Before P/C timing, record executable identities for the Candidate and for a Parent binary proved to come from the Direct Parent source SHA above. No Parent executable identity is currently available in Route evidence.
- Use the unified protocol: one long-lived process, allocations and H2D before warmup, at least 10 warmup launches with synchronization, at least 21 timed samples, and at least 4 adjacent interleaved Parent/Candidate pairs.
- Record device-event duration as the primary metric and host wall duration as a diagnostic. Keep D2H and correctness comparison outside the timed loop. Record device load and lease identity with the results.
- Use the same runner revision and input buffers for both executables. Do not substitute either executable listed above for that paired runner.

## Main review point

Main to review the selected task-domain workload, provide or approve the unified paired runner and Parent executable identity, then grant an exclusive device lease. Until those are available, V001 remains `NEEDS_ONE_MORE_LOCAL`; no performance claim is made.

No V002, dtype change, hypothesis change, Candidate source change, or online submission is included in this handoff.
