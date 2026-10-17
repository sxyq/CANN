# N03 VECTOR-ILP-DUALCHAIN-X V004 Results

## Identity

- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V004/submission.asc`
- Candidate source commit: `07c5795f718a6791f11ad23ae9854c8a97d6c4b1`
- Parent revision: V003 (`b6cd00ca`)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: group independent full-tile BF16 row `Muls` emissions before each row's affine continuation; no arithmetic, DMA, event calls, tile traversal, or address changes.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`)
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V004/07c5795f`

## Compile

- Adapter CANN 8.5 configuration: PASS, return code `0`.
- Adapter CANN 8.5 `full_link`: PASS, return code `0`.
- Paired Runner CANN 8.5 configuration: PASS, return code `0`.
- Paired Runner CANN 8.5 build: PASS, return code `0`, target `paired_runner_v016`.
- The first paired build stopped on the missing include `R31B-V016-WIDE-TILE-SEED_kernel.asc`; the candidate was placed at the existing expected filename and the build then passed. Both outputs are retained.
- CMake reported its default install prefix under the V003 paired source tree. Only the V004 build target was built; no install command was run.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 via `ASCEND_RT_VISIBLE_DEVICES=3`.
- `bf16-cache-params-d6144`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08` against the reference, Parent/Candidate mismatches `0`.
- Runner return code: `0`.

## Local

- Same-binary Parent qualification: 62 samples per case. `d6144`: median `12.400 us`, MAD/median `0.34032`, block drift `0.02742`; `d4096`: median `9.250 us`, MAD/median `0.37189`, block drift `0.57081`. Both returned `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- Paired Local: 21 Parent/Candidate pairs per case; both cases passed output matching.
- `d6144`: Parent median `10.220 us`, Candidate median `10.540 us`, paired-delta median `-0.640 us`; raw Parent maximum `40.960 us`.
- `d4096`: Parent median `11.600 us`, Candidate median `12.680 us`, paired-delta median `-1.900 us`; raw Candidate maximum `35.040 us`.
- Both `LOCAL_SCORE=NONE`. The same-binary qualification was unstable and paired samples had substantial excursions. No samples were removed and no speedup is claimed.
- Paired Runner reports visible device index `0`; `ASCEND_RT_VISIBLE_DEVICES=3` maps it to the assigned physical NPU 3 (`d3`). Pre-run snapshots recorded HBM `59871/65536 MB` and AICore `6–7%`; another Python process was present on d3. No other process was changed.
- Raw evidence: `results/remote-results/correctness.raw.log`, `same-binary-control.raw.log`, `paired-local.raw.log`, and the `*-snapshot.log` files.

## Status

- Correctness passed. Local performance is inconclusive, so the score remains `NONE`.
