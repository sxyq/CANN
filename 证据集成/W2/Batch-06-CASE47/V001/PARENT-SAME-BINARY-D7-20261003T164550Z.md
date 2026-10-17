# CASE47 V001 Direct Parent d7 same-binary

DATE_UTC: `2026-10-03`
RUN_ID: `CASE47-V001-D7-PARENT-SAMEBIN-20261003T164550Z`
LEASE_ID: `M1-CASE47-V001-D7-PARENT-QUAL-20261003T163922Z`
RESULT: `NOT_QUALIFIED`

## Scope And Identity

- One Direct Parent same-binary invocation only; no Parent window qualification or Candidate timing followed.
- Input is synthetic `PROXY_D257_FP32_M2A`: rows `80`, width `257`, dtype FP32. Runner reported `vector_cores=40`, so rows=`2*A`.
- Runner settings: one process, 45 warmups with stream synchronization after each launch, two 21-sample device-event blocks, 30-second gap. Runner RC=0; analyzer RC=0.
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Parent module SHA256: `3de95511fd6437956a8bd48fb2ece9a1f5d0f226f568ef18d129d57db1e77c24`.
- Timing runner SHA256: `d76923bcdcd791134645e34155cfb646730f822baf73b7ec5bfb5b80d68eda0f`; AArch64 Build ID `1550bfa4235c5bc8e731bfc799976991d575b016`.
- Candidate source was not loaded or run. Its recorded source SHA256 remains `be313f80e5b088f1fe907f2f4221c61ab59ef720ce68cc9f9c941be20239c2db`.

## Result

The registered same-binary rule requires combined device-event MAD/median `<= 0.10` and absolute block-median drift / combined median `<= 0.10`.

| Statistic | Block 1 | Block 2 | Combined |
|---|---:|---:|---:|
| Samples | 21 | 21 | 42 |
| Median, us | 54.0799982846 | 19.8199991137 | 31.4400009811 |
| CV | 0.5703897313 | 0.8605590125 | 0.8036737980 |
| Max/min | 10.5859874742 | 13.3476198221 | 19.7857146421 |

Combined MAD/median is `0.4707379286`; block drift is `1.0896945961`. Both exceed the registered `0.10` limit, so the Parent same-binary shape is `NOT_QUALIFIED`. This is a measurement qualification result, not a Candidate result.

## Device And Host

- Host: `hwnput3`, AArch64; SoC: `Ascend910B3`; CANN: `8.5.0.alpha002`.
- d7 before run: HBM `38131/65536 MB` (58.2% used), AICore `38%`; after run HBM remained `38131/65536 MB`, AICore was `23%`.
- d7 process inventory showed VLLMEngineCor PID `2617616` and python3 PID `439848`; neither was changed. No CASE47 timing runner remained after the invocation.
- Project disk available before and after: `582 GB`.
- At run time, the active d7 lease in canonical `调度/服务器设备使用.tsv` was the CASE47 row above, with no other active d7 lease. The Route did not edit the shared schedule.

## Remote Evidence

Directory: `/home/data4t2/lelinfeng/cann/server_runs/CASE47-SMALL-CLUSTER-CHAMPION-X/V001/timing-build-20261003T134147Z/results/CASE47-V001-D7-PARENT-SAMEBIN-20261003T164550Z/`

- Raw events: `parent-same-binary.raw.tsv` (42 samples, one PID); per-block and combined statistics: `parent-same-binary.stats.tsv`.
- Classification: `parent-same-binary.classification.tsv`; analyzer output and RC: `analyzer.stdout.txt`, `analyzer.rc`, `analyzer.stderr.txt`.
- Invocation and identity: `run-info.tsv`, `runner-command.log`, `runner.stdout.txt`, `runner.stderr.txt`, `runner.rc`, `identities-final-pre-run.txt`, `runner-linked-libraries-pre.txt`.
- Resource/process snapshots: `npu-smi-final-pre-run.txt`, `npu-smi-post.txt`, `disk-final-pre-run.txt`, `disk-post.txt`, `project-usage-final-pre-run.txt`, `project-usage-post.txt`, `processes-pre.txt`, `runner-process-post.txt`.

No PRECHECK, Candidate P/C, correctness rerun, core query, or other timing was performed.

## Lease Release Verification

- Main/Support-B live read on `hwnput3` completed at `2026-10-03T17:16:19Z`. d7 HBM was `38131/65536 MB`; AICore was `59%`.
- d7 resident processes were VLLMEngineCor PID `2617616` (`13251 MB`) and python3 PID `439848` (`21260 MB`). Both were left untouched. The process query for `CASE47-SMALL-CLUSTER-CHAMPION-X`, `CASE47-V001-D7-PARENT-SAMEBIN`, `case47_timing_runner`, and `timing-build-20261003T134147Z` returned no matches; `npu-smi info` likewise listed no CASE47 runner on d7.
- Project disk: `/dev/nvme1n1p1`, `3.5T` size, `2.7T` used, `582G` available (`83%`). The `du -sh` child-directory snapshot was: `AGENTS.md` 4.0K; `README.md` 4.0K; `STORE-EPILOGUE-W2-X` 4.8M; `local-experiments` 2.3M; `package-lock.json` 4.0K; `package.json` 4.0K; `server_runs` 32M; `w2` 16M; `worktrees` 417M; `工具` 56K; `归档` 52M; `技术路线` 348K; `本地实验` 39M; `研究` 1.7M; `线上结果` 7.3M; `调度` 216K; `项目结构.md` 4.0K; `项目规则` 72K.
- Main's Support-B delegate appended the canonical lease's `RELEASED` row with end time `2026-10-03T17:16:19Z`. No process was changed, and no PRECHECK, Candidate P/C, correctness, or other timing was run.

## Parent Same-Binary W60 Requalification

DATE_UTC: `2026-10-03`
RUN_ID: `CASE47-V001-D7-PARENT-SAMEBIN-W60-20261003T204103Z`
LEASE_ID: `M1-CASE47-V001-D7-PARENT-W60-20261003T195527Z`
RESULT: `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`
QUALIFICATION_STATUS: `NOT_QUALIFIED`

- One Parent-only attempt on synthetic `PROXY_D257_FP32_M2A`: device 7, rows `80` (`2*A`, `A=40`), width `257`, FP32. Runner PID `2240525`, RC `0`; 60 synchronized warmups, two 21-sample device-event blocks, 30-second gap. Analyzer RC `0`; raw file has 42 samples plus header. Runner stderr is empty.
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Parent module SHA256: `3de95511fd6437956a8bd48fb2ece9a1f5d0f226f568ef18d129d57db1e77c24`.
- Timing runner SHA256: `a49d44ab4fe1adcc2f6862e18d20ebbf45ad117c00f520bcb4b75ddaf1f142cb`; AArch64 Build ID `db92ef6212551f18c4f7ea8cfaba400808c9fe3d`.

The stats file's rows 1 and 2 are the device-event blocks; rows 3 and 4 are their host-wall diagnostics; row 5 is the combined 42-sample device-event distribution. Block medians were `24.2400001734 us` and `25.9000007063 us`; combined median was `25.54000075905 us`. Combined MAD/median was `0.2611589963`, above the protocol's `0.25` measurement-block threshold; block-median drift / combined median was `0.0649961035`, within the `0.10` qualification limit. The ordinary same-binary qualification status is `NOT_QUALIFIED`; because MAD/median exceeds `0.25`, the more specific result is `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`. This is a measurement protocol/host result and does not establish a Candidate regression.

- d7 pre-run snapshot: HBM `38889/65536 MB`, AICore `38%`; VLLMEngineCor PID `2617616` (`14009 MB`) and python3 PID `439848` (`21260 MB`).
- d7 post-run snapshot: HBM `36370/65536 MB`, AICore `65%`; VLLMEngineCor PID `2213949` (`11490 MB`) and python3 PID `439848` (`21260 MB`). Processes were not changed by the Route. Post-run `ps -C case47_timing_runner` returned RC `1` with no runner process. Project disk available before and after was `582 GB`.
- Postflight snapshot time: `2026-10-03T20:52:13Z`. Main reports the shared lease is now `RELEASED` by Support-B; the Route did not edit the shared schedule.
- The pre-run grep query captured its own SSH `bash -c` line because the lease ID was present in its arguments; Main independently confirmed no CASE47 runner was active before launch. The saved process-query output is retained and not interpreted as a runner process.

Remote evidence directory: `/home/data4t2/lelinfeng/cann/server_runs/CASE47-SMALL-CLUSTER-CHAMPION-X/V001/timing-runs/M1-CASE47-V001-D7-PARENT-W60-20261003T195527Z/CASE47-V001-D7-PARENT-SAMEBIN-W60-20261003T204103Z/`. It contains `runner.rc`, runner stdout/stderr, raw and stats TSVs, analyzer RC/output/classification, before/after NPU and disk snapshots, process queries, and executable identities. The analyzer used the existing V001 `analyze_timing.py` and `timing-matrix.tsv`; its warmup metadata was set to the actual value `60` for this classification.

No PRECHECK-A/B, Candidate P/C, correctness, core query, or other timing followed. Candidate Local Score and Official result remain absent; the shared lease was not released by the Route.
