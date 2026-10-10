# OFFICIAL-CASE-ANALYSIS — R31B-V011 vs improvement potential

Measurement-independent analysis of Official per-case data.
Source: `phase4/online/R31B/V011/result.json` (15/15 Pass, Official 45.16).

## Per-case breakdown

| case | timeUs | bestTimeUs | ratio | score | improvement headroom |
|---|---:|---:|---:|---:|---|
| 1 | 5.44 | 1.70 | 3.20x | 25.85 | **HIGH** (3.2x gap) |
| 2 | 3.34 | 2.16 | 1.55x | 48.19 | MEDIUM |
| 3 | 5.16 | 2.47 | 2.09x | 35.50 | **HIGH** (2.1x gap) |
| 4 | 16.55 | 6.66 | 2.48x | 30.82 | **HIGH** (2.5x gap) |
| 5 | 9.93 | 5.20 | 1.91x | 38.53 | MEDIUM-HIGH |
| 6 | 28.46 | 10.86 | 2.62x | 29.62 | **HIGH** (2.6x gap) |
| 7 | 52.34 | 13.99 | 3.74x | 23.51 | **HIGHEST** (3.7x gap) |
| 8 | 69.21 | 30.16 | 2.29x | 32.80 | **HIGH** |
| 9 | 71.85 | 68.20 | 1.05x | 88.61 | LOW (near best) |
| 10 | 74.82 | 47.05 | 1.59x | 46.64 | MEDIUM |
| 11 | 160.26 | 131.38 | 1.22x | 67.11 | LOW-MEDIUM |
| 12 | 97.69 | 76.43 | 1.28x | 62.29 | LOW-MEDIUM |
| 13 | 563.74 | 393.51 | 1.43x | 53.01 | MEDIUM |
| 14 | 16486.82 | 3750.12 | 4.40x | 21.50 | **HIGHEST** (4.4x gap) |
| 15 | 9637.47 | 8321.94 | 1.16x | 73.42 | LOW |

## Case class identification (inferred from time magnitudes)

| class | cases | characteristics | score range |
|---|---|---|---|
| Tiny (µs < 10) | 1, 2, 3, 5 | Very short kernels; launch overhead dominant | 25–48 |
| Small (10–70 µs) | 4, 6, 7, 8 | Short rows; per-row overhead visible | 23–33 |
| Medium (70–170 µs) | 9, 10, 11, 12 | Balanced compute/overhead | 46–89 |
| Large (100–600 µs) | 13 | Long rows; compute-dominated | 53 |
| Wide/Very large (>1 ms) | 14, 15 | Very wide D or many rows | 21–73 |

## Most improvable classes

1. **Class 14 (ratio 4.4x, score 21.5)**: Largest absolute gap. Likely wide-D multi-row shape. R31B-V011 wide path (cached-y / full-y) may have a structural bottleneck vs the best implementation. This is where epilogue/pipeline improvements would matter most.

2. **Class 7 (ratio 3.7x, score 23.5)**: Second largest gap. Short-to-medium kernel with high overhead. Likely a shape where per-row fixed cost (V/S handoffs, PipeBarriers) dominates. SEQ-FUSE-2 and EPI-PIPE-3 target this class.

3. **Class 1 (ratio 3.2x, score 25.8)**: Tiny kernel where launch+Init overhead is a large fraction. Ownership/scheduling changes (SCHED-CHAMPION-X) target this class.

## Implications for VECTOR-MATH-X

- Cases 14 and 7 are the highest-value targets (together worth ~2.5 points of Official score if improved to ratio 1.5x).
- Case 14 likely needs wide-path improvements (deferred in current scope) or epilogue pipelining (EPI-PIPE-3).
- Case 7 is well-suited to SEQ-FUSE-2 (denominator handoff reduction on short rows).
- Cases 9, 15 are near best — low ROI.
