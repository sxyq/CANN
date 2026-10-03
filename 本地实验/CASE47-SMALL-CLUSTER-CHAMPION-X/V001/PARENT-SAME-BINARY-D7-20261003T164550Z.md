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
- The active d7 lease row in canonical `调度/服务器设备使用.tsv` was the CASE47 row above, with no other active d7 lease. The Route did not edit the shared schedule; Main owns its release record.

## Remote Evidence

Directory: `/home/data4t2/lelinfeng/cann/server_runs/CASE47-SMALL-CLUSTER-CHAMPION-X/V001/timing-build-20261003T134147Z/results/CASE47-V001-D7-PARENT-SAMEBIN-20261003T164550Z/`

- Raw events: `parent-same-binary.raw.tsv` (42 samples, one PID); per-block and combined statistics: `parent-same-binary.stats.tsv`.
- Classification: `parent-same-binary.classification.tsv`; analyzer output and RC: `analyzer.stdout.txt`, `analyzer.rc`, `analyzer.stderr.txt`.
- Invocation and identity: `run-info.tsv`, `runner-command.log`, `runner.stdout.txt`, `runner.stderr.txt`, `runner.rc`, `identities-final-pre-run.txt`, `runner-linked-libraries-pre.txt`.
- Resource/process snapshots: `npu-smi-final-pre-run.txt`, `npu-smi-post.txt`, `disk-final-pre-run.txt`, `disk-post.txt`, `project-usage-final-pre-run.txt`, `project-usage-post.txt`, `processes-pre.txt`, `runner-process-post.txt`.

No PRECHECK, Candidate P/C, correctness rerun, core query, or other timing was performed.
