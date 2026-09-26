# ASYNC-TRIPLE-X 8x8192 — quiet-window exclusive same-binary re-qualification

**Decision: MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE (8x8192, and the 8x4096 fallback too)**

- Lease `QR-ASYNC-8x8192` (d4): 2026-09-25T14:58:13Z -> 14:59:57Z, RELEASED.
- Method: unified reference harness (`async_ref_seed_probe`, `runner_ref.inc`).
  `DEVICE_EVENT_US` primary, `HOST_WALL_US` secondary, warmup=10, 2 blocks x 31 samples,
  batch N=1, gap 2 s, one process. Direct Parent (SEED) vs itself.
- Preflight: no concurrent route probe, d4 AICore 0%, residual VLLM HBM expected.
- Host load: 1-min average 25.08 -> 22.80 across the window.
- `bad=0` on both shapes. Kernel SHA unchanged. No source edit.

## PRIMARY shape 8x8192 (tileCount=8)

| block | n | median_us | MAD_us | MAD/med | min | max |
|---|---:|---:|---:|---:|---:|---:|
| B1 | 31 | 101.82 | 23.40 | 0.2298 | 55.34 | 164.62 |
| B2 | 31 | 105.98 | 46.74 | 0.4410 | 59.24 | 302.78 |
| ALL | 62 | 103.90 | 28.26 | **0.2720** | 55.34 | 302.78 |

- **MAD/median = 0.2720**, **drift = 0.0400**.
- FAIL the PASS criterion (`≤0.10` on both). `MAD/med > 0.25` → `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`.
- Note: drift is now **clean** (0.0400 vs 0.4427 last cycle) — exclusivity fixed the
  block-to-block drift. The residual failure is full-sample MAD/median, i.e. sparse
  device-event outliers (max 302.78 µs vs median 103.90 µs) under resident VLLM.

## FALLBACK shape 8x4096 (run because 8x8192 was still blocked)

| block | n | median_us | MAD_us | MAD/med | min | max |
|---|---:|---:|---:|---:|---:|---:|
| B1 | 31 | 116.96 | 16.84 | 0.1440 | 60.34 | 272.42 |
| B2 | 31 | 205.92 | 79.26 | 0.3849 | 74.60 | 586.20 |
| ALL | 62 | 118.99 | 49.54 | **0.4163** | 60.34 | 586.20 |

- **MAD/median = 0.4163**, **drift = 0.7476** → both above 0.25 → blocked as well.

## Comparison with the prior (sibling-contaminated) cycle

| shape | prior MAD/med | prior drift | this MAD/med | this drift |
|---|---:|---:|---:|---:|
| 8x8192 | 0.4565 | 0.4427 | **0.2720** | **0.0400** |
| 8x4096 | 0.3449 | 0.0893 | 0.4163 | 0.7476 |

8x8192 improved substantially under exclusivity (MAD/med 0.46 → 0.27, drift 0.44 → 0.04)
but still does not reach the ≤0.10 PASS criterion, and still crosses the >0.25 block threshold.
8x4096 went the other way this window.

## Consequences

- No interleaved P/C run.
- Raw samples permanent: `8x8192/floor/samebin-seed-raw.tsv` and
  `8x4096/floor/samebin-seed-raw.tsv` (62 rows each), plus `floor-verdict.json`.
- Prior evidence untouched in `../../results-ref-8x8192/` and `../../results-ref-8x4096/`.
