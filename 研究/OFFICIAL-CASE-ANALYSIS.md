# OFFICIAL-CASE-ANALYSIS — R31B-V011 vs improvement potential

2026-10-12 update: the current six-version per-case matrix and ten latest valid receipts are appended below. Earlier Tiny/Small/Wide labels based on elapsed time are unverified hypotheses; they are not workload shape, dtype, or mechanism evidence.

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

## Historical timing groups (unverified workload mapping)

The following groups were inferred only from elapsed-time magnitude. Judge and repository receipts do not map these cases to dtype, Rows, D, dispatch, or tile configuration.

| class | cases | characteristics | score range |
|---|---|---|---|
| Tiny (µs < 10) | 1, 2, 3, 5 | Very short kernels; launch overhead dominant | 25–48 |
| Small (10–70 µs) | 4, 6, 7, 8 | Short rows; per-row overhead visible | 23–33 |
| Medium (70–170 µs) | 9, 10, 11, 12 | Balanced compute/overhead | 46–89 |
| Large (100–600 µs) | 13 | Long rows; compute-dominated | 53 |
| Wide/Very large (>1 ms) | 14, 15 | Very wide D or many rows | 21–73 |

## Historical time-scale hypotheses (not current route priorities)

1. **Class 14 (ratio 4.4x, score 21.5)**: Largest absolute gap. Likely wide-D multi-row shape. R31B-V011 wide path (cached-y / full-y) may have a structural bottleneck vs the best implementation. This is where epilogue/pipeline improvements would matter most.

2. **Class 7 (ratio 3.7x, score 23.5)**: Second largest gap. Short-to-medium kernel with high overhead. Likely a shape where per-row fixed cost (V/S handoffs, PipeBarriers) dominates. SEQ-FUSE-2 and EPI-PIPE-3 target this class.

3. **Class 1 (ratio 3.2x, score 25.8)**: Tiny kernel where launch+Init overhead is a large fraction. Ownership/scheduling changes (SCHED-CHAMPION-X) target this class.

## Historical VECTOR-MATH-X discussion (not a current route instruction)

- Cases 14 and 7 are the highest-value targets (together worth ~2.5 points of Official score if improved to ratio 1.5x).
- Case 14 likely needs wide-path improvements (deferred in current scope) or epilogue pipelining (EPI-PIPE-3).
- Case 7 is well-suited to SEQ-FUSE-2 (denominator handoff reduction on short rows).
- Cases 9, 15 are near best — low ROI.

## 2026-10-12 Six-Version Receipt Matrix

Sources are result.json and source-meta.json in each version's online-results directory. All six receipts are Pass 15/15. Each cell is timeUs / bestTimeUs / case score. The official formula is s_i = 100 / (1 + log_1.5(timeUs / bestTimeUs)); the total is the arithmetic mean of the 15 case scores. The implementation is in 工具/cannjudge-submit.mjs, and the public problem description states the same rule.

| Case | testcase_id | dtype / Rows / D | R31B V011 | N01 V013 | N02 V010 | N03 V012 | N04 V017 | N05 V018 |
|---:|---|---|---|---|---|---|---|---|
| 1 | 6a9a9a99bf41025d6013eb8a | UNKNOWN | 5.44/1.70/25.849 | 5.25/1.12/20.789 | 4.88/1.12/21.599 | 5.36/1.12/20.570 | 4.91/1.12/21.528 | 4.87/1.12/21.622 |
| 2 | 6a9a9a99bf41025d6013eb8e | UNKNOWN | 3.34/2.16/48.193 | 3.21/1.63/37.434 | 3.24/1.63/37.115 | 3.24/1.63/37.115 | 3.28/1.63/36.703 | 3.39/1.63/35.639 |
| 3 | 6a9a9a99bf41025d6013eb92 | UNKNOWN | 5.16/2.47/35.499 | 5.33/2.10/30.329 | 5.46/2.10/29.792 | 5.45/2.10/29.832 | 5.11/2.10/31.317 | 4.97/2.10/32.003 |
| 4 | 6a9a9a99bf41025d6013eb96 | UNKNOWN | 16.55/6.66/30.817 | 16.76/4.42/23.325 | 17.14/4.42/23.028 | 17.82/4.42/22.530 | 16.60/4.42/23.455 | 16.41/4.42/23.612 |
| 5 | 6a9a9a99bf41025d6013eb9a | UNKNOWN | 9.93/5.20/38.529 | 9.90/4.91/36.636 | 9.82/4.91/36.907 | 10.01/5.03/37.075 | 10.02/4.91/36.242 | 9.86/4.91/36.771 |
| 6 | 6a9a9a99bf41025d6013eb9e | UNKNOWN | 28.46/10.86/29.620 | 28.60/7.95/24.053 | 28.55/7.95/24.078 | 29.78/7.95/23.490 | 28.36/7.95/24.174 | 28.41/7.95/24.149 |
| 7 | 6a9a9a99bf41025d6013eba2 | UNKNOWN | 52.34/13.99/23.507 | 52.92/10.50/20.044 | 51.77/10.50/20.264 | 51.90/10.61/20.345 | 51.80/10.50/20.258 | 51.75/10.61/20.374 |
| 8 | 6a9a9a99bf41025d6013eba6 | UNKNOWN | 69.21/30.16/32.802 | 69.86/29.72/32.176 | 69.82/29.72/32.191 | 69.72/29.72/32.228 | 69.80/29.72/32.198 | 69.62/29.72/32.264 |
| 9 | 6a9a9a99bf41025d6013ebaa | UNKNOWN | 71.85/68.20/88.607 | 72.10/67.81/86.859 | 71.22/67.81/89.206 | 70.92/67.81/90.042 | 71.10/67.81/89.538 | 72.92/67.81/84.804 |
| 10 | 6a9a9a99bf41025d6013ebae | UNKNOWN | 74.82/47.05/46.641 | 74.92/45.87/45.249 | 75.15/45.87/45.095 | 74.92/45.87/45.249 | 75.00/45.87/45.195 | 74.49/45.87/45.542 |
| 11 | 6a9a9a99bf41025d6013ebb2 | UNKNOWN | 160.26/131.38/67.111 | 163.27/129.65/63.749 | 161.40/129.65/64.925 | 159.90/129.65/65.911 | 162.12/129.65/64.466 | 161.59/129.65/64.803 |
| 12 | 6a9a9a99bf41025d6013ebb6 | UNKNOWN | 97.69/76.43/62.294 | 97.88/75.96/61.527 | 98.77/75.96/60.694 | 99.15/75.96/60.347 | 97.86/75.96/61.546 | 97.64/75.96/61.757 |
| 13 | 6a9a9a99bf41025d6013ebba | UNKNOWN | 563.74/393.51/53.005 | 563.51/394.15/53.147 | 567.25/394.15/52.690 | 563.63/394.15/53.132 | 566.93/394.15/52.728 | 561.92/394.15/53.344 |
| 14 | 6a9a9a99bf41025d6013ebbe | UNKNOWN | 16486.82/3750.12/21.496 | 16473.23/3532.25/20.844 | 16532.60/3532.25/20.805 | 16592.98/3532.25/20.766 | 16698.10/3532.25/20.699 | 16498.84/3532.25/20.827 |
| 15 | 6a9a9a99bf41025d6013ebc2 | UNKNOWN | 9637.47/8321.94/73.423 | 9643.96/7961.18/67.892 | 9662.81/7961.18/67.671 | 9646.12/7961.18/67.867 | 9663.82/7961.18/67.659 | 9677.15/7961.18/67.504 |

### Version Identity

| Version | Official | SID | Source SHA |
|---|---:|---|---|
| R31B V011 | 45.16 | 6ab2c10c0304f72a56a0c5cb | a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3 |
| N01 V013 | 41.60 | 6acc0cd4694b590c3c875250 | 3499280f91cd2a3069d72b08fde4cc865a99bf84238c73168e34d6847076ca32 |
| N02 V010 | 41.74 | 6acc167c694b590c3c89c48d | 22cb1a23baa96a4864913f8c8189cd3ab8ae2df0fd361d456c3e65d17185845a |
| N03 V012 | 41.77 | 6acbfef2694b590c3c83b7a6 | 1be243a51dd7a7f45d051051b766549d3a71f302f5de7ff596d2b91c59259c66 |
| N04 V017 | 41.85 | 6acc2d65694b590c3c919bc8 | 786b0100c88adb232219fd221c4ffeea68ee393dc692405a41cbaf3cb97e646f |
| N05 V018 | 41.67 | 6acc07c4694b590c3c85ffcf | 9cfa3d8367be47e467a3afde1f8aebb9dda6c28341d452e6f63717b451246703 |

### Score Changes and Confidence

Comparing the case scores returned in each receipt with Champion, the largest negative differences are Case 2 (-10.76 to -12.55), Case 4 (-7.21 to -8.29), Case 15 (-5.53 to -5.92), Case 6 (-5.45 to -6.13), Case 1 (-4.23 to -5.28), and Case 3 (-3.50 to -5.71). Positive differences appear at Case 9 for N02/N03/N04 (+0.60/+1.43/+0.93), and Case 13 for N01/N03/N05 (+0.14/+0.13/+0.34).

These are actual case scores returned by Judge, but they are not pure Candidate latency changes. All 15 bestTimeUs values differ from the Champion receipt. Raw timeUs values across Route receipts are mostly close to Champion. All five routes are lower on Case 1 (4.87–5.36 us versus 5.44 us); on Case 4, N03/N02/N01/N04 are slower by about 7.7%/3.6%/1.3%/0.3%, while N05 is about 0.8% faster. N02/N03/N04 are about 0.9%–1.3% faster on Case 9. Each Official receipt contains one Judge timeUs value, with no repeated-sample distribution for significance analysis.

The following table isolates the returned `timeUs` ratio from each route's one-shot result against Champion: `(route timeUs / Champion timeUs - 1) * 100`. Negative means the returned latency is lower. It does not identify input shape or establish statistical significance.

| Case | N01 V013 | N02 V010 | N03 V012 | N04 V017 | N05 V018 |
|---:|---:|---:|---:|---:|---:|
| 1 | -3.49% | -10.29% | -1.47% | -9.74% | -10.48% |
| 2 | -3.89% | -2.99% | -2.99% | -1.80% | +1.50% |
| 3 | +3.29% | +5.81% | +5.62% | -0.97% | -3.68% |
| 4 | +1.27% | +3.56% | +7.67% | +0.30% | -0.85% |
| 5 | -0.30% | -1.11% | +0.81% | +0.91% | -0.70% |
| 6 | +0.49% | +0.32% | +4.64% | -0.35% | -0.18% |
| 7 | +1.11% | -1.09% | -0.84% | -1.03% | -1.13% |
| 8 | +0.94% | +0.88% | +0.74% | +0.85% | +0.59% |
| 9 | +0.35% | -0.88% | -1.29% | -1.04% | +1.49% |
| 10 | +0.13% | +0.44% | +0.13% | +0.24% | -0.44% |
| 11 | +1.88% | +0.71% | -0.22% | +1.16% | +0.83% |
| 12 | +0.19% | +1.11% | +1.49% | +0.17% | -0.05% |
| 13 | -0.04% | +0.62% | -0.02% | +0.57% | -0.32% |
| 14 | -0.08% | +0.28% | +0.64% | +1.28% | +0.07% |
| 15 | +0.07% | +0.26% | +0.09% | +0.27% | +0.41% |

On returned latency, the clearest shared regression signal is Case 3 for N01/N02/N03, followed by Case 4 for N02/N03 and Case 6 for N03. These are route-specific one-shot deltas, not a proven cross-route workload class. Case 1 is faster in all five route receipts; Case 7 is within roughly -1.13% to +1.11%; Case 14 and 15 returned times are within about 1.3% and 0.5% of Champion respectively. The largest negative *case-score* changes (especially Case 2) should not be used as a raw-latency ranking because the `bestTimeUs` denominator changed between receipts.

The public Judge problem data exposes the 15 testcase IDs and type=default, but no per-case dtype, Rows, D, dispatch, or tile mapping. Keep all of these input fields UNKNOWN; do not infer shape from elapsed time. The Route Official results establish full-score behavior for each submitted source, but do not directly bind a case score to a particular mechanism or Local shape.

### Ten Latest Valid Receipts

Ordered by source-meta terminal_at descending. Times are UTC. Each is Pass 15/15 with 15 unique testcase IDs.

| Time | Version | SID | Official |
|---|---|---|---:|
| 2026-10-12 00:45:37.527 | N04 V017 | 6acc2d65694b590c3c919bc8 | 41.85 |
| 2026-10-12 00:39:39.877 | N03 V018 | 6acc2bfc694b590c3c90fdf2 | 41.00 |
| 2026-10-12 00:33:59.749 | N01 V018 | 6acc2aab694b590c3c906872 | 41.21 |
| 2026-10-12 00:28:18.186 | N05 V025 | 6acc2953694b590c3c8fd733 | 41.18 |
| 2026-10-12 00:24:28.356 | N05 V024 | 6acc286f694b590c3c8f867e | 41.26 |
| 2026-10-12 00:05:30.563 | N05 V020 | 6acc23fe694b590c3c8dd44f | 40.85 |
| 2026-10-11 23:59:43.621 | N05 V023 | 6acc229f694b590c3c8d5ac5 | 41.30 |
| 2026-10-11 23:54:32.436 | N03 V017 | 6acc216c694b590c3c8cf9b7 | 40.98 |
| 2026-10-11 23:50:26.012 | N02 V011 | 6acc2075694b590c3c8caa6a | 41.39 |
| 2026-10-11 23:45:52.649 | N01 V017 | 6acc1f64694b590c3c8c5b47 | 41.03 |

N04 V018 HTTP 429 returned no SID and is not a valid receipt.
