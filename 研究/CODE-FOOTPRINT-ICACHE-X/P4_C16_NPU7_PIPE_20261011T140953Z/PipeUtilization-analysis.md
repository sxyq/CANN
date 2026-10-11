# R3 existing PipeUtilization capture analysis

Status: read-only analysis of the already-existing `P4_C16_NPU7_PIPE_20261011T140953Z` capture. No workload was rerun. This remains a Champion-only target-kernel probe, not an R3 Kernel Revision, Candidate, or Local score.

## Capture identity and command

- Source SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Executable: `/home/data4t2/lelinfeng/cann-w2-main2-control/研究/主代理/MAIN-2/calibration-v4/runs/20261005-v4-candidates/stages/HOTLOOP-ADDR-HOIST-CHAMPION-X__V002/build-v4/clx_ref_parent_probe`.
- Executable SHA-256: `eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71`.
- Existing command arguments: device 7, `rows=9`, `width=32768`, `dtype_code=2`, `warmup=45`, `samples=31`, `blocks=1`, `gap=0`, `batch_n=1`.
- `msprof --output=<route-owned msprof-pipe> --ai-core=on --aic-metrics=PipeUtilization --task-time=on --ascendcl=on <executable> <arguments>`; stored verbatim in `command.txt`.
- `msprof.rc.txt=0`; stderr is empty; the log reports `Profiling finished` and `Process profiling data complete`.

## What the “no summary data” log means here

`msprof.stdout.log` line 17 says:

```text
There is no summary data to export for ".../device_7"
```

This message does **not** mean PipeUtilization metrics are absent. In the same existing profile directory:

- `mindstudio_profiler_output/op_summary_20261011141004.csv` exists (27,952 bytes), with all 76 target-kernel rows and the `aiv_icache_miss_rate`, `aic_icache_miss_rate`, `aiv_vec_ratio`, `aiv_mte2_ratio`, `aiv_scalar_ratio`, and `aiv_mte3_ratio` fields.
- `device_7/sqlite/metric_summary.db` exists; its `MetricSummary` table has 76 rows and those same metric columns.
- `op_statistic_20261011141004.csv`, `task_time_20261011141004.csv`, and `api_statistic_20261011141004.csv` also exist.
- The exporter log says parsing was skipped because the data had already been analyzed, then says summary output export ended and gives the `mindstudio_profiler_output` path.

Thus the warning concerns the exporter’s separate optional “summary data” payload for the device. The Pipe op-summary CSV and metric-summary SQLite were nevertheless produced. This was a direct `msprof` invocation, not a run through a post-processing step that would create a human-readable `summary.txt`. The logs do not reveal a more specific internal cause for the absent optional payload; no missing I-cache field/interface is observed in the exported files.

## Available target-task indicators

The CSV identifies the exact target kernel symbol `_Z24add_rms_norm_bias_customIu6__bf16EvPhS0_S0_S0_S0_mmjff`, `Task Type=AI_VECTOR_CORE`, device 7, block dim 9. It contains 45 warmup rows followed by 31 measured rows.

For the final 31 rows:

| Metric | Min | Median | Mean | Max | Nonzero rows |
|---|---:|---:|---:|---:|---:|
| `aiv_icache_miss_rate` | 2.6% | 6.6% | 7.061% | 10.4% | 31/31 |
| `aic_icache_miss_rate` | 0% | 0% | 0% | 0% | 0/31 |
| `aiv_vec_ratio` | 18.0% | 28.2% | 29.258% | 47.8% | 31/31 |
| `aiv_mte2_ratio` | 22.1% | 29.3% | 30.297% | 47.9% | 31/31 |

All measured AIV I-cache miss-rate rows are below the profiler reference’s `>15%` attention guidance. This is a real, target-task-attributed aggregate metric, but it is not a PC/address-level instruction trace and cannot identify which code region/layout caused misses.

## Quality and attribution limits

- Device-event timing in the same workload raw: median `26.22 us`, CV `0.324693`; do not qualify it as Local.
- Preflight: NPU7 HBM use 37%, AIV use 91%; external PID `439848` (`python3`, 21,260 MB) was present and untouched. Postflight AIV was 0%, HBM remained 37%.
- The committed full seven-metric capture of the same C16 workload reported zero AIV/AIC Pipe miss across its measured 31 rows, whereas this focused Pipe capture reports 2.6–10.4% AIV misses. Preserve both observations; the high preflight AIV activity and timing variability prevent treating the difference as a controlled code effect.
- No R3 kernel variant was measured. The result cannot establish a compiler/layout effect or a causal I-cache bottleneck.

## Conclusion / next step

The existing artifacts contain usable PipeUtilization and I-cache fields despite the “no summary data to export” line. The focused capture shows nonzero but sub-threshold target-task AIV misses; the separately committed full capture shows zero, so overall evidence is inconsistent and does not support an attributable R3 I-cache hypothesis. Keep `FEASIBILITY_UNPROVEN / RESEARCH_ONLY / NO_KERNEL_PATCH_TO_COMPARE`. Do not create a Kernel Revision or repeat this profile. A future authorized R3 variant and controlled same-workload before/after evidence would be required for code-layout attribution.
