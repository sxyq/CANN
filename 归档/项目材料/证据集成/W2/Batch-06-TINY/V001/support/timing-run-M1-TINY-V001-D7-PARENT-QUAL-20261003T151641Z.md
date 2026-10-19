# TINY V001 D7 Parent-only Requalification

DATE: `2026-10-03`
ROUTE: `TINY-FIXED-OVERHEAD-CHAMPION-X`
REVISION: `V001`
LEASE_ID: `M1-TINY-V001-D7-PARENT-QUAL-20261003T151641Z`
RUN_ID: `M1-TINY-V001-D7-PARENT-QUAL-20261003T151641Z`
STATUS: `PARENT_SAME_BINARY_ONLY; CAP40_FAIL; CAP19_FAIL; PRECHECK_NOT_RUN; CANDIDATE_PC_NOT_RUN`

## Outcome

Cap 40 and cap 19 were run in separate processes and evaluated independently. Each invocation used 45 synchronized warmups followed by two 21-sample device-event blocks. Both returned `0`, read runtime `A/M/B=40/20/19`, and passed the Parent-only post-run golden comparison. Both failed same-binary qualification, so neither proceeded to PRECHECK-A/B or Candidate/Parent timing.

| Cap | Parent dispatch | Samples | Event median (us) | MAD (us) | MAD/median | Block medians (us) | Block drift | Same-binary | Shape result |
|---:|---|---:|---:|---:|---:|---|---:|---|---|
| 40 | `H2_FASTFORM`; `20xFP32_NARROW_MID_OVERLAP` | 42 | 20.08 | 6.58 | 0.327689 | 22.64 / 18.96 | 0.183267 | FAIL | `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` |
| 19 | `PARENT_FORMULA_FALLBACK`; `1xFP32_SMALL_CONTIG_BATCHED+18xFP32_NARROW_MID_OVERLAP` | 42 | 33.46 | 15.19 | 0.453975 | 37.00 / 26.64 | 0.309623 | FAIL | `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` |

Full-sample device-event statistics from each Parent stats file:

| Cap | Mean (us) | Stdev (us) | p10 (us) | p90 (us) | Min (us) | Max (us) | CV | Max/min |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 40 | 23.7548 | 13.9781 | 10.454 | 33.102 | 7.5 | 84.9 | 0.588434 | 11.32 |
| 19 | 49.9338 | 39.2441 | 21.368 | 95.928 | 7.3 | 221.26 | 0.785923 | 30.3096 |

Same-binary PASS requires full-sample MAD/median `<=0.10` and relative drift between block medians `<=0.10`. `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` applies when MAD/median exceeds `0.25` or block drift exceeds `0.25`. Cap 40 exceeds the MAD/median boundary. Cap 19 exceeds both boundaries. These are Parent timing-shape outcomes and do not measure Candidate performance. The two caps are not pooled; cap 19 is a fallback control and cannot accept or reject H2.

Parent post-run golden result for each cap: `PASS`, `max_abs=7.15255737e-07`, `bad=0`. The earlier V001 Candidate PROXY Correctness record is `build-correctness-20261003.md`; it reports both declared proxy cases PASS with `A/M/B=40/20/19`, each `max_abs=7.15255737e-07`, `bad=0`, and return code `0`. This D7 run executed only Parent. No Candidate P/C samples were collected. Overall V001 local verdict remains `NOT_COMPLETE`, pending Main review.

## Source and Executable Identity

These identities match the Route build record at commit `9105f1cfea1d1db2d35424d548f76e563dca3330` and the executable specified by Main.

| Object | SHA256 |
|---|---|
| Candidate `submission.asc` | `4e8abff76486532cab4be6bc6098ea80c048802d285837ad0afb193ff9712024` |
| Direct Parent `R31B-V011/submission.asc` | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| Timing runner source | `0a2c6f80f170ecc4c0e2e982010cc374ea2b7a0377dcf4f4abdb8202ad4b4bf6` |
| Generated Parent module | `30382d11580fd83afbf3f981fb5dffcc7cbc8d599b9ef96fa607caed143cc293` |
| Generated Candidate module | `433256f20d40c7d52b25fecc4de5b96bd77ccd57ca895faa7cc2ba09e5b03a2c` |
| ASC object | `f0daaeb65b79300f794faf3ee4aa7ce780479ecf1b812c6efe52b2f94c602c9d` |
| Timing executable | `f8e2e8a509a6a36fd854d0ff25433999bc00d3b492cc8c40fb25bca74f7f5f6c` |

The canonical lease note contains executable SHA256 `f8e8e2a509a6a36fd854d0ff25433999bc00d3b492cc8c40fb25bca74f7f5f6c`, with the middle characters transposed. The remote executable itself was freshly read and matched Main's requested SHA256 `f8e2e8a509a6a36fd854d0ff25433999bc00d3b492cc8c40fb25bca74f7f5f6c`. The shared lease row was not edited.

Full build details are in `timing-harness-build-20261003T132344Z.md`. The reset-free runner source was used, and no device reset was invoked.

## Environment and Commands

Host `hwnput3`; device 7, `Ascend910B3`, target `dav-2201`; CANN `8.5.0.alpha002`; Bisheng/Clang `15.0.5`. The live pre-run read at `2026-10-03T15:29:08Z` showed d7 HBM `38131/65536 MB` (27405 MB free), AICore `60%`, `VLLMEngineCor` PID `2617616` using `13251 MB`, `python3` PID `439848` using `21260 MB`, project disk free `582G`, and no TINY timing/correctness process. The target lease was the only active d7 lease. AICore and resident processes were recorded only; no process was changed.

The post-run resource read sequence began at `2026-10-03T15:40:08Z`: d7 HBM remained `38131/65536 MB`, AICore was `12%`, the same two resident processes remained, project disk free was `582G`, no TINY timing/correctness process was present, and the target lease's canonical last row remained `LEASED`.

Both invocations sourced the same CANN environment and used the verified executable:

```bash
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-build-20261003T132344Z/build/tiny_v001_timing_harness 7 40 same-parent /home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs/M1-TINY-V001-D7-PARENT-QUAL-20261003T151641Z/cap40/same-parent > /home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs/M1-TINY-V001-D7-PARENT-QUAL-20261003T151641Z/cap40/same-parent.stdout.log 2>&1
/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-build-20261003T132344Z/build/tiny_v001_timing_harness 7 19 same-parent /home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs/M1-TINY-V001-D7-PARENT-QUAL-20261003T151641Z/cap19/same-parent > /home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs/M1-TINY-V001-D7-PARENT-QUAL-20261003T151641Z/cap19/same-parent.stdout.log 2>&1
```

Each invocation returned `0`. The unique result root is `/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs/M1-TINY-V001-D7-PARENT-QUAL-20261003T151641Z`. Each cap retains `same-parent-raw.tsv` (42 samples plus header), `same-parent-stats.tsv`, `same-parent.stdout.log`, and `same-parent.exit-code.txt`. Each stats file has 90 lines. No PRECHECK or paired files exist.

No Candidate, shared scheduling, score, Champion, or Official record was changed. No further runner was launched.
