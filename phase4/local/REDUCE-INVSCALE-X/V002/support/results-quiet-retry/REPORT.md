# REDUCE-INVSCALE-X 1x8192 FP32 — quiet-window exclusive same-binary re-qualification

**Decision: MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE**

- Lease `QR-REDUCE-8192` (d4): 2026-09-25T15:00:13Z -> 15:01:14Z, RELEASED.
- Method: unified reference harness (`reduce_invscale_refdirect_probe`, `runner_ref.inc`).
  `DEVICE_EVENT_US` primary, `HOST_WALL_US` secondary, warmup=10, 2 blocks x 31 samples,
  batch N=1, gap 2 s, one process. Direct Parent vs itself.
- Preflight: no concurrent route probe, d4 AICore 0%, residual VLLM HBM expected.
- Host load: 1-min average 22.32 -> 22.41 across the window.
- Kernel SHA unchanged (submission `bef271b6…`, parent `94ab0ef9…`). No source edit.

## Multi-tile path confirmation (correctness-only smoke, untimed)

| binary | shape | warmup/samples | bad | max_abs | expected |
|---|---|---|---:|---:|---|
| `reduce_invscale_refdirect_probe` (parent) | 1x8192 | 3 / 5 | **2048** | 5.4174 | bad=2048 ✅ |
| `reduce_invscale_refcand_probe` (candidate) | 1x8192 | 3 / 5 | **0** | 1.669e-06 | bad=0 ✅ |

Multi-tile path confirmed on both sides, identical to the previous cycle.

## Result (attempt qb1, the one clean exclusive attempt, Direct Parent only)

| block | n | median_us | MAD_us | p10 | p90 | min | max |
|---|---:|---:|---:|---:|---:|---:|---:|
| B1_DEVICE | 31 | 12.78 | 4.70 | 7.44 | 33.20 | 7.32 | 153.18 |
| B2_DEVICE | 31 | 6.60 | 0.18 | 6.36 | 129.94 | 6.18 | 213.96 |
| ALL_DEVICE | 62 | 7.70 | 1.21 | 6.42 | 128.20 | 6.18 | 213.96 |

- MAD/median = 1.21 / 7.70 = **0.1571** (inside the ≤0.25 block, but above the ≤0.10 PASS criterion)
- drift |B1_med − B2_med| / median = |12.78 − 6.60| / 7.70 = **0.8026**
- `drift > 0.25` → **`MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`**
- `bad=2048` is the parent's own multi-tile correctness gap, expected and unrelated to timing.

## Comparison with the prior (sibling-contaminated) cycle

| attempt | window | MAD/med | drift | direction |
|---|---|---:|---:|---|
| prior | concurrent siblings (d5 BATCH, d6 REDUCE overlap) | 0.2149 | 0.8667 | B1 slower |
| **qb1 (this run)** | **exclusive quiet window** | **0.1571** | **0.8026** | **B1 slower (12.78 > 6.60)** |

Exclusivity improved MAD/median modestly but left the block asymmetry essentially
unchanged. B2's MAD is 0.18 µs (very tight) while B1 carries the whole spread — the
slow block is block 1 in both cycles.

## Consequences

- No interleaved P/C run.
- Raw samples permanent: `d4/samebin-w10-s31-raw.tsv` (62 rows,
  sha256 `123887e2ba37063d7e36f756c580e5b4895d85b9f486905f6645c2fd6cfb0131`),
  plus smoke raw files for parent and candidate.
- Prior evidence untouched in `../results-ref-8192/`.
