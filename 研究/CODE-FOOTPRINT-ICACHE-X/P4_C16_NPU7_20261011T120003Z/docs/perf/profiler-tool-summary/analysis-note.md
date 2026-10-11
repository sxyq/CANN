# R3 P4 C16 existing-profile analysis

Status: Champion-only bottleneck probe; not an R3 Kernel Revision, Candidate, or Local score. R3 remains `FEASIBILITY_UNPROVEN` / `NO_KERNEL_PATCH_TO_COMPARE`.

## Identity and inputs

- Workload: Champion V011 C16, NPU7, BF16 (`dtype=2`), shape `9x32768`, `Block Dim=9`.
- Source SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Executable SHA-256: `eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71`.
- Existing capture: `../../msprof-output-private/PROF_GROUP_20261011_120616/`.
- The seven metric logs identify `device=7 shape=9x32768 dtype=2 warmup=45 samples=31 blocks=1 bad=0`.
- Canonical standalone raw/stats are in `../../../canonical-C16-raw.tsv` and `../../../canonical-C16-stats.txt`; 31 device-event samples, one block, CV `1.21926`. This is noisy and is not a qualified Local score.

## Existing report-tool run and caveat

Ran the project's existing `ops-profiling/scripts/msprof_perf_summary.py` in standard mode against the existing `PROF_GROUP`, with the exact kernel `Op Name`; no workload was rerun:

```text
python3 /home/data4t2/lelinfeng/.codex/plugins/cache/personal/ops-direct-invoke/1.2.5+codex.cannbot-1-3-2/skills/ops-profiling/scripts/msprof_perf_summary.py \
  研究/CODE-FOOTPRINT-ICACHE-X/P4_C16_NPU7_20261011T120003Z/msprof-output-private/PROF_GROUP_20261011_120616 \
  研究/CODE-FOOTPRINT-ICACHE-X/P4_C16_NPU7_20261011T120003Z \
  --op-name '_Z24add_rms_norm_bias_customIu6__bf16EvPhS0_S0_S0_S0_mmjff' \
  --round-name profiler-tool-summary
```

The tool archived the available metric CSVs and wrote `summary.txt` here.

The tool's standard summary selects the *first* exact-name row from each CSV. Because each metric group contains 76 rows (45 warmups followed by 31 measured calls), the summary's displayed AIV I-cache value `0.101` (10.1%) is a warmup row, not the steady measured window. The tool also looks only under `device_0` for sample-based `aicore.db`, so it does not emit a per-core balance report for this NPU7 capture. Do not treat either limitation as a capture failure or use the first-row value as the C16 measured result.

## I-cache evidence

Parsed the exact target row across all 76 `PipeUtilization` entries and kept the final 31 measured calls separate from the 45 warmups:

- Measured 31: `aiv_icache_miss_rate` min/median/mean/max = `0 / 0 / 0 / 0`; all 31 are zero. `aic_icache_miss_rate` is zero in all 76 entries.
- Warmup 45: AIV rate mean `0.01187` (1.187%), max `0.103` (10.3%); 18 nonzero entries. Even the warmup maximum is below the profiler reference's `>15%` attention threshold.
- Sample-based `ai_vector_core_utilization_20261011120744.csv`: 41 vector-core rows, `icache_miss_rate` min/median/mean/max = `0.002 / 0.003 / 0.002829 / 0.003` (0.2–0.3%; mean 0.283%). Sample-based `ai_core_utilization_20261011120744.csv`: 21 rows, all I-cache rates zero.
- The other metric groups contain no I-cache-named fields; `PipeUtilization` and the sample-based core-utilization CSVs are the sources of the reported rate.

Interpretation: this Champion C16 capture does **not** support a steady-state I-cache bottleneck; measured PipeUtilization rates are zero and sampled AIV rates are approximately 0.28%. This is not a same-path R3 before/after comparison, so it cannot establish or exclude a compiler/layout effect for an R3 variant.

## Device context

Successful capture preflight (2026-10-11 12:05:39Z): NPU7 HBM usage 37%, AICore 10%, AIVector 43%; external PID `439848` (`python3`) used about `21260 MB`. Postflight (12:08:49Z): HBM usage 37%, AICore 0%, AIVector 4%; only that external PID remained. No process was touched. Treat the standalone timing as noisy; preserve the raw samples.

## Next action

No recapture or Candidate is justified by this result. Keep R3 frozen at `FEASIBILITY_UNPROVEN`; a future authorized R3 kernel artifact is required for an attributable same-workload code/layout and instruction-side comparison.
