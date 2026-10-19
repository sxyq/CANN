# W4-R11 V003 RESULT

## Status

- Direct Parent: V002 (`本地实验/W4-R11/V002/submission.asc`)
- Single performance change: the narrow-row remainder owner interval is centered across the block range. `blockCount`, `baseRows`, row arithmetic, tile selection, and pipeline bodies are unchanged.
- Candidate SHA256: `ba813753e1a36cf94aae8b81649e95bdaf30770e9992398fa78f1bd7aeeeeebc`
- Official score: `NONE` in this Route task; no Online submission was performed.
- Local status: `NONE` as a qualified score. Numeric observations and every raw sample remain below.

## Compile

- Command: `bash 本地实验/W4-R11/V003/build_server3.sh`
- Device: `0`, Ascend 910B3, host `hwnput3`
- Compile admission: `FREE_HBM_MB=57671`
- Result: `CONFIGURE_RC=0`, `BUILD_RC=0`
- Remote log: `/home/data4t2/lelinfeng/cann/w4/R11-row-remainder-balance-x/V003/logs/server3-compile-20261009T162846Z.log`
- Local transfer log: `本地实验/W4-R11/V003/logs/server3-transfer-compile-20261009T162823Z.log` (the script output records the remote log path above)

## Correctness

- Command: `bash 本地实验/W4-R11/V003/run_server3.sh correctness 0`
- Runtime reported `available_vector_cores=40`.
- `target-fp32-64x8192`: Parent failures `0`, Candidate failures `0`, maximum absolute error `4.76837e-06` for both.
- `control-fp32-12x8192`: Parent failures `0`, Candidate failures `0`, maximum absolute error `3.09944e-06` for both.
- Evidence: `本地实验/W4-R11/V003/logs/correctness-device0-preflight.txt`, `本地实验/W4-R11/V003/logs/correctness-device0-postflight.txt`.

## Parent same-binary qualification

- Command: `bash 本地实验/W4-R11/V003/run_server3.sh same 0 本地实验/W4-R11/V003/results/parent-same-20261010T-local.raw.tsv`
- The runner captured 82 samples per shape. The relative output path was normalized into the V003 results directory after the command completed; the raw data was not changed.
- `target-fp32-64x8192`: median `33.62 us`, MAD/median `0.328971`, block drift `0.499108`, verdict `MEASUREMENT_BLOCKED`.
- `control-fp32-12x8192`: median `16.77 us`, MAD/median `0.586166`, block drift `0.806202`, verdict `MEASUREMENT_BLOCKED`.
- Raw evidence: `本地实验/W4-R11/V003/results/parent-same-20261010T-local.raw.tsv`.

## Local observations

- Command: `bash 本地实验/W4-R11/V003/run_server3.sh local 0 本地实验/W4-R11/V003/results/local-20261010T-local.raw.tsv`
- 44 interleaved Parent/Candidate pairs per shape; warmup `45`; device-event timing; device `0`.

| Shape | Parent median (us) | Candidate median (us) | Paired delta median | Median-ratio delta | Parent MAD/median | Candidate MAD/median |
|---|---:|---:|---:|---:|---:|---:|
| 64x8192 FP32 target | 9.85 | 10.12 | +2.637930% | +2.741117% | 0.044670 | 0.042490 |
| 12x8192 FP32 control | 8.58 | 6.49 | -2.645927% | -24.358974% | 0.304196 | 0.106317 |

- Target raw ranges: Parent `8.86..154.76 us`, Candidate `9.14..58.56 us`.
- Control raw ranges: Parent `5.60..41.70 us`, Candidate `5.72..27.80 us`.
- The two same-binary qualifications were blocked by high dispersion, so these deltas are observations only. `LOCAL_SCORE=NONE`, `LOCAL_DELTA=NONE`, `CURRENT_LOCAL_BEST=NONE`.
- Raw evidence: `本地实验/W4-R11/V003/results/local-20261010T-local.raw.tsv`.

## Device context

- Local preflight: HBM capacity `65536 MB`, usage rate `73%`, calculated free HBM about `17694 MB`; AICore `0%`; load average `40.29 39.54 35.30`.
- Local postflight: HBM usage rate `5%`, calculated free HBM about `62259 MB`; AICore `0%`; load average `35.31 38.48 35.02`.
- Device 0 had no running NPU process in the snapshots. Other device processes remained untouched and are retained as load context.
- Device evidence: `本地实验/W4-R11/V003/logs/local-device0-preflight.txt`, `本地实验/W4-R11/V003/logs/local-device0-postflight.txt`, `本地实验/W4-R11/V003/logs/same-device0-preflight.txt`, `本地实验/W4-R11/V003/logs/same-device0-postflight.txt`.

## Result and handoff

- Build: `PASS`
- Correctness: `PASS` for both Parent and Candidate on both local proxy shapes.
- Local: numeric observations retained; no qualified numeric Local score because Parent same-binary qualification was `MEASUREMENT_BLOCKED`.
- Performance hypothesis: a centered contiguous owner interval may avoid concentrating extra-row blocks at the launch-index suffix while preserving contiguous per-block rows and the existing compute pipeline.
- Candidate is a real, source-distinct R11 artifact and can be handed to Online Owner under the user-authorized W4 multi-Candidate scope. This Route Agent did not submit it.
- `PUSH=NO`.
