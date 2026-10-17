# TINY V001 D5 Parent-only Timing Record

DATE: `2026-10-03`
ROUTE: `TINY-FIXED-OVERHEAD-CHAMPION-X`
REVISION: `V001`
RUN_ID: `M1-TINY-V001-D5-LOCAL-PROXY-20261003T134147Z`
STATUS: `PARENT_SAME_BINARY_ONLY; BOTH_CAPS_SHAPE_PROTOCOL_BLOCKED; PRECHECK_NOT_RUN; CANDIDATE_PC_NOT_RUN`

## Scope and Outcome

The cap 40 equal-ownership probe and cap 19 Parent-fallback control were run and evaluated separately. Their samples are not pooled. Each same-binary invocation returned `0`, and its Parent-only post-run golden comparison passed. Both failed the registered same-binary qualification; no PRECHECK-A, PRECHECK-B, or Candidate/Parent timing was run for either cap.

| Cap | Runtime A/M/B | Reached branch and per-block dispatch | Samples | Event median (us) | MAD (us) | MAD/median | Block medians (us) | Block drift | Same-binary | Shape result |
|---:|---|---|---:|---:|---:|---:|---|---:|---|---|
| 40 | 40/20/19 | `H2_FASTFORM`; `20xFP32_NARROW_MID_OVERLAP` | 42 | 11.89 | 2.99 | 0.251472 | 13.78 / 10.96 | 0.237174 | FAIL | `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` |
| 19 | 40/20/19 | `PARENT_FORMULA_FALLBACK`; `1xFP32_SMALL_CONTIG_BATCHED+18xFP32_NARROW_MID_OVERLAP` | 42 | 11.18 | 3.54 | 0.316637 | 12.00 / 11.02 | 0.0876565 | FAIL | `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` |

Full-sample device-event summaries from each Parent stats file:

| Cap | Mean (us) | Stdev (us) | p10 (us) | p90 (us) | Min (us) | Max (us) | CV | Max/min |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 40 | 35.5814 | 54.1706 | 9.502 | 134.496 | 7.18 | 205.72 | 1.52244 | 28.6518 |
| 19 | 25.2062 | 42.4033 | 7.12 | 53.484 | 5.68 | 205.98 | 1.68226 | 36.2641 |

The registered same-binary PASS limits are full-sample MAD/median `<=0.10` and relative drift between the two block medians `<=0.10`. The shape classification is `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` when MAD/median exceeds `0.25` or block drift exceeds `0.25`; both caps exceed the MAD/median limit. This is a timing-shape outcome, not a Candidate performance result. Cap 19 remains a fallback control and cannot accept or reject H2.

Both invocations reported Parent golden `PASS`, `max_abs=7.15255737e-07`, `bad=0`. Candidate P/C was not run. The overall V001 local verdict remains `NOT_COMPLETE`, pending Main review.

## Source and Executable Identity

The identities below match the Route build record at source-control commit `9105f1cfea1d1db2d35424d548f76e563dca3330` and the executable specified for this lease.

| Object | SHA256 |
|---|---|
| Candidate `submission.asc` | `4e8abff76486532cab4be6bc6098ea80c048802d285837ad0afb193ff9712024` |
| Direct Parent `R31B-V011/submission.asc` | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| Timing runner source | `0a2c6f80f170ecc4c0e2e982010cc374ea2b7a0377dcf4f4abdb8202ad4b4bf6` |
| Generated Parent module | `30382d11580fd83afbf3f981fb5dffcc7cbc8d599b9ef96fa607caed143cc293` |
| Generated Candidate module | `433256f20d40c7d52b25fecc4de5b96bd77ccd57ca895faa7cc2ba09e5b03a2c` |
| ASC object | `f0daaeb65b79300f794faf3ee4aa7ce780479ecf1b812c6efe52b2f94c602c9d` |
| Timing executable | `f8e2e8a509a6a36fd854d0ff25433999bc00d3b492cc8c40fb25bca74f7f5f6c` |

Full build details are in `timing-harness-build-20261003T132344Z.md`. The reset-free timing runner source was used; no device reset was invoked.

## Correctness Context

The earlier V001 PROXY Correctness record `build-correctness-20261003.md` reports Candidate correctness PASS for both declared cases on the same `[20,256]` FP32 proxy input family: runtime `A=40`, `M=20`, `B=19`; cap 40 reached `rowCount==blockCount` / `H2_FASTFORM`, and cap 19 reached `rowCount!=blockCount` / `PARENT_FORMULA_FALLBACK`. Each returned `0`, with `max_abs=7.15255737e-07` and `bad=0`. Its Candidate correctness executable SHA256 is `08fc6c673b56ea677ed23bcceb73fa8ed1f7b8e39475f49bcb768f447a06e49e`.

Those PROXY cases are not Official workload claims. The timing run compared only Parent against itself for stability; it contains no Candidate P/C samples.

## Environment and Commands

Host `hwnput3`; device 5, `Ascend910B3`, target `dav-2201`; CANN `8.5.0.alpha002`; Bisheng/Clang `15.0.5`. The runner used shape `[20,256]`, dtype FP32, 45 synchronized warmups and two blocks of 21 single-launch device-event samples per invocation.

Main's pre-run snapshot at `2026-10-03T13:41:47Z`: d5 HBM `60201/65536 MB`, AICore `44%`, `VLLMWorker_TP` PID `89602`, project disk free `591G`, no TINY runner/build process, and no other active d5 lease. The Route Agent's additional pre-run reads showed d5 HBM `60200/65536 MB`, AICore `44%`, the same VLLM PID, `591G` disk free, no matching TINY process, and this lease active; the individual read commands did not save a separate timestamp.

The post-run resource read sequence began at `2026-10-03T14:11:31Z`: d5 HBM `60200/65536 MB`, AICore `36%`, VLLM PID `89602` using `56678 MB`, project disk free `582G`, and no matching TINY process. A subsequent read at `14:16:59Z` showed d5 HBM unchanged, AICore `38%`, the same VLLM PID and disk free `582G`; the lease's last row remained `LEASED`. AICore and VLLM were recorded only; no process was changed.

The environment and exact invocations were:

```bash
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-build-20261003T132344Z/build/tiny_v001_timing_harness 5 40 same-parent /home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs/M1-TINY-V001-D5-LOCAL-PROXY-20261003T134147Z/cap40/same-parent > /home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs/M1-TINY-V001-D5-LOCAL-PROXY-20261003T134147Z/cap40/same-parent.stdout.log 2>&1
/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-build-20261003T132344Z/build/tiny_v001_timing_harness 5 19 same-parent /home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs/M1-TINY-V001-D5-LOCAL-PROXY-20261003T134147Z/cap19/same-parent > /home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs/M1-TINY-V001-D5-LOCAL-PROXY-20261003T134147Z/cap19/same-parent.stdout.log 2>&1
```

Each invocation returned `0`. The unique result root is `/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs/M1-TINY-V001-D5-LOCAL-PROXY-20261003T134147Z`. Each cap retains its own `same-parent-raw.tsv` (42 samples plus header), `same-parent-stats.tsv`, `same-parent.stdout.log`, and `same-parent.exit-code.txt`. Each stats file has 90 lines. No PRECHECK or paired files exist.

Cap 40 raw: `<result-root>/cap40/same-parent-raw.tsv`; stats: `<result-root>/cap40/same-parent-stats.tsv`; stdout: `<result-root>/cap40/same-parent.stdout.log`; exit code: `<result-root>/cap40/same-parent.exit-code.txt`.

Cap 19 raw: `<result-root>/cap19/same-parent-raw.tsv`; stats: `<result-root>/cap19/same-parent-stats.tsv`; stdout: `<result-root>/cap19/same-parent.stdout.log`; exit code: `<result-root>/cap19/same-parent.exit-code.txt`.

No Candidate, shared scheduling, score, Champion, or Official record was changed. No further runner was launched.
