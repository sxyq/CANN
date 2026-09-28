# COEFF-LOCALITY-X V003 — Timing Summary

Date: 2026-09-29
Device: d4 (AICore 0% pre/post, HBM ~90%)
Protocol: runner_ref.inc DEVICE_EVENT_PRIMARY, warmup=45, samples=41, same-binary blocks=2, 6 interleaved P/C pairs
Hypothesis: NH-2 split-phase param MTE2 emission (gamma after last Mul, bias after last Add)

## Same-binary

| shape | P MAD/med | P drift | C MAD/med | C drift | qual |
|---|---|---|---|---|---|
| 1x32768_fp32 | 0.020 | 0.000 | 0.024 | 0.012 | PASS both |
| 1x16384_fp32 | 0.083 | 0.055 | 0.050 | **1.192** | P only (C drift fail) |
| 8x32768_fp32 | 0.030 | 0.026 | 0.041 | -0.006 | PASS both |
| 1x4096_fp32 | **0.137** | 0.078 | 0.080 | 0.018 | C only |
| 1x8192_fp32 | **0.149** | **0.159** | 0.064 | -0.055 | C only |

## Paired clean deltas (delta% = (C−P)/P; positive = candidate slower = fav parent)

| shape | clean n | median Δ% | favP / favC | note |
|---|---|---|---|---|
| **1x32768_fp32** | 6 | **+2.37** | 4 / 2 | primary, both same-binary PASS |
| **8x32768_fp32** | 5 | **+2.06** | 3 / 1 | both same-binary PASS |
| 1x16384_fp32 | 5 | +2.78 | 5 / 0 | directional only (C same-binary FAIL) |
| **1x4096_fp32 (control)** | 4 | **+1.47** | 3 / 1 | tileCount=1, split-phase never fires → **bias floor** |
| 1x8192_fp32 | 3 | +5.17 | 3 / 0 | noisy (P same-binary FAIL) |

Outliers removed (raw retained): 1x16384 p5, 8x32768 p3, 1x4096 p1/p4, 1x8192 p3/p4/p6.

## Verdict

**LOCAL_REJECTED.** All three large-D probes favor parent. The control shape
(+1.47% with the mechanism inert) sets the measurement-bias floor; the large-D
excess is only ~0.6–1.3%.

**Falsification (from DECLARATION):** with tileElems constant at 4096,
1x32768 clean |Δ| = +2.37% ≈ noise → **param MTE2 timing is NOT the large-D
bottleneck.** The V001 confound is now resolved: V001's +19.2% was the
tile-shrink side effect, and the timing mechanism itself is at best neutral.

Likely cost source: the extra V_MTE2 SetFlag/WaitFlag pairs (2 per tile;
tileCount=8 → 14 extra flag ops) cost at least what the prefetch hides.
