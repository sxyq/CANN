# N03 VECTOR-ILP-DUALCHAIN-X V005 Results

## Identity

- Candidate source: `本地实验/N03-VECTOR-ILP-DUALCHAIN-X/V005/submission.asc`
- Candidate source commit: `26c7fad0d30410be64eb04c9c7b33dc33500baab`
- Parent revision: V004 (`07c5795f`)
- Measurement parent: R31B V011 (`1c51a5b2`)
- Change: group independent full-tile BF16 row affine `Mul` emissions before each row's bias `Add`; no arithmetic, DMA, event calls, tile traversal, or address changes.
- Device: `cann-server3`, `hwnput3`, physical NPU 3 (`d3`)
- Remote evidence root: `~/cann/server_runs/N03-VECTOR-ILP-DUALCHAIN-X-RESET-20261011/V005/26c7fad0`

## Compile

- Adapter CANN 8.5 configuration and `full_link`: PASS, return codes `0`.
- Paired Runner CANN 8.5 configuration and `paired_runner_v016` build: PASS, return codes `0`.

## Correctness

- Existing `paired_runner_v016 --correctness`, physical NPU 3 via `ASCEND_RT_VISIBLE_DEVICES=3`.
- `bf16-cache-params-d6144`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `0`.
- `bf16-full-tile-d4096`: Parent PASS, Candidate PASS, exact match PASS; failures `0`, max absolute error `1.4901161e-08` against reference, Parent/Candidate mismatches `0`.
- Runner return code: `0`.

## Local

- Same-binary Parent qualification: 62 samples per case. `d6144`: median `11.420 us`, MAD/median `0.22067`, block drift `0.36953`; `d4096`: median `11.300 us`, MAD/median `0.37168`, block drift `0.00354`. Both returned `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- Paired Local: 21 Parent/Candidate pairs per case; both passed output matching.
- `d6144`: Parent median `17.580 us`, Candidate median `22.300 us`, paired-delta median `+4.980 us`; Candidate maximum `223.980 us`.
- `d4096`: Parent median `11.760 us`, Candidate median `17.100 us`, paired-delta median `+6.180 us`; Candidate maximum `164.880 us`.
- Both `LOCAL_SCORE=NONE`. Parent qualification was unstable and the paired samples had large excursions. No samples were removed and no speedup is claimed.
- The paired Runner prints visible device index `0`; `ASCEND_RT_VISIBLE_DEVICES=3` maps it to assigned physical NPU 3 (`d3`). Pre-Correctness snapshots showed HBM `59870/65536 MB` with AICore `16%`; before Local HBM was `59871/65536 MB` and AICore `14%`. Another user's Python training process was present on d3 and was left unchanged.
- Raw evidence: `results/remote-results/correctness.raw.log`, `same-binary-control.raw.log`, `paired-local.raw.log`, and `*-snapshot.log` files.

## Status

- Correctness passed. Local performance is inconclusive, so score remains `NONE`.
