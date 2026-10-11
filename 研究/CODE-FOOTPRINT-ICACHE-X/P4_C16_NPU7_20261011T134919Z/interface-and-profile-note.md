# R3 Canonical Host interface and C16 profile receipt

## Status

`ROUTE=W5-R03 / CODE-FOOTPRINT-ICACHE-X`

This is a Champion-only target-code/profile probe. It is not an R3 Kernel Revision, Candidate, Correctness result, or Local score. R3 remains `FEASIBILITY_UNPROVEN / RESEARCH_ONLY / NO_KERNEL_PATCH_TO_COMPARE`.

## Published interface and identity

The published Main2 Canonical workload is available at:

```text
/home/data4t2/lelinfeng/cann-w2-main2-control/工具/main2-canonical-benchmark.mjs
```

Its `invoke()` interface passes the target executable:

```text
[device, c.ROWS, c.WIDTH, c.DTYPE_CODE, raw_prefix, warmup, samples, '1', '0', '1']
```

The published C16 suite row is `ROWS=9`, `WIDTH=32768`, `DTYPE=BF16`, `DTYPE_CODE=2`, expected path `ProcessWideLowPrecision`. The exact published Parent executable and identities are:

```text
EXE=/home/data4t2/lelinfeng/cann-w2-main2-control/研究/主代理/MAIN-2/calibration-v4/runs/20261005-v4-candidates/stages/HOTLOOP-ADDR-HOIST-CHAMPION-X__V002/build-v4/clx_ref_parent_probe
SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
EXE_SHA256=eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71
```

The existing project profiling entry is available at:

```text
/usr/local/Ascend/ascend-toolkit/latest/bin/msprof
/home/data4t2/lelinfeng/.codex/plugins/cache/personal/ops-direct-invoke/1.2.5+codex.cannbot-1-3-2/skills/ops-profiling/scripts/msprof_profile_run.sh
```

`msprof --version` is not accepted by this installed binary (`unrecognized option '--version'`), but its usage exposes `--application`, and the existing standard wrapper completed this target workload successfully. No compiler, runner, or profiler was added.

## Device-window check

Before sampling, the published shared lease file was checked for device 7:

```text
/home/data4t2/lelinfeng/cann-w2-main2-control/调度/服务器设备使用.tsv
```

The latest device-7 lease record was `M2-CANONICAL-REDUCE-HIER-X__V005-D7-1791205602629`, with a later `RELEASED` row at `2026-10-05T13:12:29.095Z`; no current R1 or R5 Local lease was present. Live preflight recorded HBM usage 37% and external PID `439848` (`python3`, 21260 MB). The process was not touched.

## Exact run

The existing wrapper was run once, with no outer warmup because the Canonical executable itself received the fixed `45/31` protocol arguments:

```text
source /usr/local/Ascend/ascend-toolkit/set_env.sh
umask 077
bash /home/data4t2/lelinfeng/.codex/plugins/cache/personal/ops-direct-invoke/1.2.5+codex.cannbot-1-3-2/skills/ops-profiling/scripts/msprof_profile_run.sh \
  --warm-up=0 \
  --output=研究/CODE-FOOTPRINT-ICACHE-X/P4_C16_NPU7_20261011T134919Z/msprof-output-private \
  -- /home/data4t2/lelinfeng/cann-w2-main2-control/研究/主代理/MAIN-2/calibration-v4/runs/20261005-v4-candidates/stages/HOTLOOP-ADDR-HOIST-CHAMPION-X__V002/build-v4/clx_ref_parent_probe \
  7 9 32768 2 研究/CODE-FOOTPRINT-ICACHE-X/P4_C16_NPU7_20261011T134919Z/canonical-C16 45 31 1 0 1
```

Result: wrapper `RC=0`; all seven aic-metrics groups and sample-based output were produced under the R3-private capture directory:

```text
研究/CODE-FOOTPRINT-ICACHE-X/P4_C16_NPU7_20261011T134919Z/msprof-output-private/PROF_GROUP_20261011_134920/
```

The input shape/dtype/protocol matches the published C16 suite and the R3 prior capture (`9x32768`, BF16 code 2, 45 warmups, 31 samples, batch 1). A public historical V4 C16 record reports `blocks=2,gap=1`, while this direct Canonical invocation reports `blocks=1,gap=0`; that historical runner-output difference is retained and prevents reusing the public timing raw as an identical protocol. The current invocation itself is internally consistent and passed.

## Existing profiler-tool analysis

The project parser was run against the new existing `PROF_GROUP` without another workload launch:

```text
python3 /home/data4t2/lelinfeng/.codex/plugins/cache/personal/ops-direct-invoke/1.2.5+codex.cannbot-1-3-2/skills/ops-profiling/scripts/msprof_perf_summary.py \
  研究/CODE-FOOTPRINT-ICACHE-X/P4_C16_NPU7_20261011T134919Z/msprof-output-private/PROF_GROUP_20261011_134920 \
  研究/CODE-FOOTPRINT-ICACHE-X/P4_C16_NPU7_20261011T134919Z \
  --op-name '_Z24add_rms_norm_bias_customIu6__bf16EvPhS0_S0_S0_S0_mmjff' \
  --round-name profiler-tool-summary
```

Exact target-row aggregation over the 76 PipeUtilization rows (45 warmups followed by the 31 measured calls):

| Window | AIV I-cache miss | AIC I-cache miss |
|---|---:|---:|
| Warmup 45, mean / max | 0.004533 / 0.046 | 0 / 0 |
| Measured 31, min / median / mean / max | 0 / 0 / 0 / 0 | 0 / 0 / 0 / 0 |

The sample-based CSVs reported `ai_core_utilization` I-cache `0.001` for all 21 AIC rows and `ai_vector_core_utilization` I-cache `0.015–0.017` for 41 AIV rows (mean `0.015463`, about 1.55%). These are target-workload sample metrics, but they are not a before/after code-layout comparison and remain below the profiler reference's `>15%` attention guidance.

The parser's displayed first-row AIV value (`0.045`, 4.5%) is a warmup row; it is not the measured steady-state value. The standalone raw has one block, 31 samples, `bad=0`, device median `29.2 us`, CV `0.182757`; it is retained as profile context only and is not a qualified Parent/Candidate Local score.

## Interpretation and next action

The published Canonical Host workload can directly execute the target Kernel under the authorized msprof wrapper with the expected C16 input. This capture provides real target instruction-side I-cache fields, but it does not support an I-cache bottleneck or prove a compiler/layout effect: measured PipeUtilization miss rates are zero, sampled AIV rates are low, and no R3 variant exists for same-path attribution.

Keep R3 frozen at `FEASIBILITY_UNPROVEN`. Do not create a Kernel Revision or Candidate from this Champion-only probe. The next concrete step requires an authorized R3 target artifact with the same Canonical input and a controlled before/after layout/codegen comparison; no further recapture is needed for the current gate.
