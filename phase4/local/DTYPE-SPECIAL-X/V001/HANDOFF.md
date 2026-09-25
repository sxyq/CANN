# DTYPE-SPECIAL-X V001 Handoff

## State

- Route: `DTYPE-SPECIAL-X`
- Revision: `V001` (existing pending Candidate)
- Local disposition: `NEEDS_ONE_MORE_LOCAL`
- Branch: `exec/sixlane-20260924-dtype-special-x`
- Candidate dtype: FP32
- Direct Parent: `R31B-V011`, score `45.16`
- Candidate source: `phase4/workspaces/DTYPE-SPECIAL-X/V001/submission.asc`
- Candidate source SHA-256: `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f`

## Retained evidence

- CPU correctness: 39/39 PASS, `logs/cpu-correctness.json`.
- CANN compile and link: PASS on CANN `8.5.0.alpha002`, SoC `Ascend910B3`, NPU arch `dav-2201`; records are in `source-meta.json` and `logs/server3-source-env-20260924T190042Z-build-link.log`.
- NPU correctness: 39/39 PASS on device 4; maximum absolute error `2.6226044e-06`; timing was not run. Log: `logs/server3-source-env-20260924T190042Z-correctness.log`.
- The local source, workspace source, sidecar, and final build source record all report SHA-256 `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f`.

## Executable identity

On 2026-09-25, the V001 Parent, Candidate, and unified runner targets were configured, compiled, and linked on `cann-server3` using CANN `8.5.0.alpha002`, SoC `Ascend910B3`, and NPU arch `dav-2201`. The build script uploaded and verified all six source inputs before configuration: Parent source, Candidate source, both compile adapters, unified runner source, and `CMakeLists.txt`.

| Path | SHA-256 | Size | Purpose |
|---|---|---:|---|
| `/home/data4t2/lelinfeng/phase4-workspaces/DTYPE-SPECIAL-X/V001/build/dtype_special_x_v001_parent` | `11e3c07c4e35181153b20f5acd90cad700261af9d309d52724cb0713a1646434` | 466832 bytes | Parent build from the declared Parent source. |
| `/home/data4t2/lelinfeng/phase4-workspaces/DTYPE-SPECIAL-X/V001/build/dtype_special_x_v001` | `14cdc6cf6f986fd49f92662b4ff180266d0265cafdc0145706e37153b984a90b` | 466832 bytes | Candidate build. |
| `/home/data4t2/lelinfeng/phase4-workspaces/DTYPE-SPECIAL-X/V001/build/dtype_special_x_v001_unified_runner` | `545bf0fdaf4b5d6d064551ce28cf465419fe04a809645acbceaf75f1af4e6bcb` | 605808 bytes | Unified runner supporting correctness, same-binary noise-floor, and paired modes. |

The Parent and Candidate source SHAs are respectively `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` and `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f`. The paired runner source SHA is `cc16e8149314232d4e4e7ea3bf361479c96e66ee79ad54dabc8b977141ef99a1`. Source and executable identities are retained in `logs/unified-unified-build-20260925T131059Z-30843-build-identity.txt`, with source inputs in `logs/unified-unified-build-20260925T131059Z-30843-source.sha256`.

The runner was built but not executed. The build log contains CCEC host parsing warnings for `GM_ADDR` attributes; all three targets linked successfully. Earlier failed unified-runner link evidence remains retained alongside the successful build logs.

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

The existing 39/39 NPU correctness result remains the correctness evidence for the unchanged Candidate source. Correctness was not rerun during this build-identity closure. The unified runner has distinct correctness, noise-floor, and paired modes; its measurement modes require a current preflight timestamp, explicit lease identity, and a same-shape Parent noise-floor record before paired mode.

## Conditions for another local run

- No current `MAIN-1` device lease is active. Main must issue a fresh exclusive lease and the required preflight must pass before any timing.
- The unified runner and both Parent/Candidate executable identities are now present in Route evidence. Use this Route's runner only; do not substitute another Route's runner.
- Use the unified protocol: one long-lived process, allocations and H2D before warmup, at least 10 warmup launches with synchronization, at least 21 timed samples, and at least 4 adjacent interleaved Parent/Candidate pairs.
- Record device-event duration as the primary metric and host wall duration as a diagnostic. Keep D2H and correctness comparison outside the timed loop. Record device load and lease identity with the results.
- Use the same runner revision and input buffers for both executables. Do not substitute either executable listed above for that paired runner.

## Main review point

Main to review the exact 24-shape workload, source/executable identities, and build records. No timing or new correctness run was performed in this continuation. V001 remains `NEEDS_ONE_MORE_LOCAL`; no performance claim is made.

No V002, dtype change, hypothesis change, Candidate source change, or online submission is included in this handoff.
