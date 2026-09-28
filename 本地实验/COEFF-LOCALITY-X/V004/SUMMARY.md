# COEFF-LOCALITY-X V004 — Timing Summary (3 rounds)

Date: 2026-09-29
Hypothesis: NH-1 generic full-row param preload for single-row multi-tile cores
Diff: Process() cacheParams condition only (one line)
Protocol: runner_ref.inc DEVICE_EVENT_PRIMARY, warmup≥45, samples=41 (81 in R3),
same-binary blocks + interleaved P/C pairs, controls included per V003 practice

## Rounds

| round | device | protocol | shapes |
|---|---|---|---|
| R1 | d4 | warmup=45 samples=41, sb×2, pairs×6 | 1x8192, 1x6144, 2x8192, 3x6144, ctrl 1x4096/1x32768 |
| R2 | d5 | warmup=45 samples=41, sb×2, pairs×8 | 1x8192, 1x6144, 2x8192, ctrl 1x4096/1x32768 |
| R3 | d4 | warmup=60 samples=81, sb×3, pairs×8 | 1x6144, 1x8192, ctrl 1x32768 |

## Same-binary (best round R3)

| shape | P MAD/med | P drift | C MAD/med | C drift | qual |
|---|---|---|---|---|---|
| 1x6144_fp32 | 0.098 | 0.062 | 0.083 | -0.004 | **PASS both** |
| 1x8192_fp32 | 0.067 | -0.013 | 0.103 | 0.078 | P only |
| 1x32768_fp32 (ctrl) | 0.024 | -0.013 | 0.029 | 0.024 | PASS both |

R1/R2 had more sb failures (short-kernel jitter); R3's longer sampling qualified 1x6144 both sides.

## Paired clean deltas (negative = candidate faster)

| shape | R1 | R2 | R3 | pooled favC/total |
|---|---|---|---|---|
| **1x6144_fp32** | -4.83% (4/4) | -10.22% (5/6) | **-4.25% (6/8)** | **15/18** |
| 1x8192_fp32 | -4.35% (5/5) | -0.52% (3/3) | -0.68% (4/7) | 12/15 |
| 2x8192_fp32 | +1.23% (2/1) | -4.25% (5/7) | — | 7/9 |
| 3x6144_fp32 | -1.54% (3/3) | — | — | 3/6 |
| **ctrl 1x32768** (wide, untouched) | -1.14% (3/2) | +1.03% (4/3) | **-2.77% (5/6)** | 11/18 |
| ctrl 1x4096 (NarrowMid, untouched) | -5.99% (4/1) | +4.40% (5/2) | — | 9/12 |

(favC/total = clean pairs favoring candidate / clean pairs; medians are clean-pair medians)

## The confounder

The 1x32768 control (code path literally identical in both binaries) shows
**-2.77% in R3** with 5/6 clean pairs favoring candidate. That is a
binary-wide candidate-favoring bias of the measurement apparatus (layout /
launch overhead), not the mechanism. The 1x6144 excess over that control in
the same run is only **≈ -1.5%**.

Pooled direction: mechanism-on shapes favor candidate 37/45 clean pairs;
controls 20/30. Target skew is stronger but the control skew is the same sign.

## Verdict

**NEEDS_ONE_MORE_LOCAL.** Direction is candidate-favoring on the claimed band
(1x6144 stable across 3 rounds, 15/18 pairs), but:
- co-primary 1x8192 is null in R2/R3 (-0.5%, -0.7% — at or inside control bias);
- the only fully-qualified round shows target excess of just ~1.5% over control;
- the control itself shows a statistically visible candidate-favoring delta.

Per DECLARATION falsification ("if 1x8192 clean |Δ| ≤ control bias, descriptor
hoist is not material"): **1x8192 is falsified**. 1x6144 keeps a small residual
signal that cannot be cleanly separated from binary-wide bias with the current
apparatus.

Lane-level reading: large-D closed by V003; NH-1 marginal (1x6144-only, ~1-2%
excess). Recommend LANE fact package → LANE_NEEDS_PLANNING_REVIEW unless
Main-2 wants a null-binary control round (parent rebuilt vs parent) to settle
the bias question.

## What would settle it

A parent-vs-parent same-binary rebuild pair measured under the same protocol.
If that "null binary" also shows -2~-3%, the 1x6144 residual is apparatus bias
and the mechanism is fully closed. If it shows ~0, then 1x6144's -4% is real
and worth keeping as a small-win accumulator.
