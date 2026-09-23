# Champion Bottleneck Review — R31B-V011 (45.16)

Reconfirmed from `phase4/online/R31B/V011/result.json` (Pass 15/15, Official 45.16).

## Gap metric

`gap = candidate_time / best_time` (lower is better; 1.0 = at board best).

## Top gaps

| rank | case | candidate_us | best_us | gap | point_score | status |
|---|---|---|---|---|---|---|
| 1 | T14 | 16486.82 | 3750.12 | **4.396** | 21.496 | Pass |
| 2 | T07 | 52.34 | 13.99 | **3.741** | 23.507 | Pass |
| 3 | T01 | 5.44 | 1.70 | **3.200** | 25.849 | Pass |
| 4 | T06 | 28.46 | 10.86 | **2.621** | 29.620 | Pass |
| 5 | T04 | 16.55 | 6.66 | **2.485** | 30.817 | Pass |

Also large: T08 gap 2.295, T03 gap 2.089.

## Mapping (label required)

### T14 (gap 4.40) — dominant score sink

- **FACT**: slowest absolute case; largest ratio; lowest point score among top gaps.
- **INFERENCE**: historically labeled wide / large-D / few-row FP32 stress in R31 notes (`results.tsv` repeatedly cites T14 as flat). Likely dispatch-mode or wide-path coverage hole rather than uniform slowdown (other mid cases gap 2–3×).
- **UNKNOWN**: exact hidden shape (R, D, dtype) of T14.

### T07 (gap 3.74)

- **FACT**: second-worst ratio; mid absolute time.
- **INFERENCE**: mid-D path less optimized than T14-class wide or small-D paths.
- **UNKNOWN**: dtype / row count.

### T01 (gap 3.20)

- **FACT**: small absolute times; still 3× board best.
- **INFERENCE**: launch / setup overhead or tiny-shape path not fully specialized.
- **UNKNOWN**: whether dominated by fixed overhead.

### T06 / T04 (gaps 2.6 / 2.5)

- **FACT**: mid gaps; scores ~30.
- **INFERENCE**: secondary mid-bucket inefficiency; lower priority than T14/T07/T01.
- **UNKNOWN**: mode assignment inside multimode dispatch.

## Priority for next single-variable work (planning only; no experiment this round)

1. T14 class (wide / large residual) — highest official leverage if fix is real.
2. T07 mid path.
3. T01 tiny-shape fixed cost.

Each future revision must still start from **Direct Parent = current Best** (V011 for R31B) with one hypothesis.
