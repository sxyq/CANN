# ALIGN-TAIL-X 2x100 FP32 — quiet-window exclusive same-binary re-qualification

**Decision: MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE**

- Lease `QR-ALIGN-2x100` (d4): 2026-09-25T14:57:19Z -> 14:57:57Z, RELEASED.
- Method: unified reference harness (`atx_ref_parent_probe`, verbatim `runner_ref.inc` port).
  `DEVICE_EVENT_US` primary, `HOST_WALL_US` secondary, warmup=10, 2 blocks x 31 samples,
  batch N=1, gap 2 s, one process. Direct Parent vs itself.
- Preflight: no active lease, no concurrent route probe (`ps` = NONE), d4 AICore 0%,
  residual VLLM HBM resident on d4 (expected).
- Host load: 1-min average 27.20 (start) -> 26.36 (end); 5/15-min ~24-25. Driven by
  persistent VLLM workers + a user `Lingma` process. Documented, not shut off.
- Correctness: `bad=0`, `max_abs=3.58e-07`.
- Kernel SHA unchanged (candidate `f573d16d…`, parent `c8d0f010…`). No source edit.

## Result (attempt qb1, the one clean exclusive attempt)

| block | n | median_us | MAD_us | p10 | p90 | min | max |
|---|---:|---:|---:|---:|---:|---:|---:|
| B1_DEVICE | 31 | 9.44 | 2.28 | 7.00 | 22.54 | 5.90 | 246.64 |
| B2_DEVICE | 31 | 6.30 | 0.84 | 5.42 | 135.46 | 5.22 | 191.64 |
| ALL_DEVICE | 62 | 8.25 | 2.36 | 5.56 | 130.15 | 5.22 | 246.64 |

- MAD/median = 2.36 / 8.25 = **0.2861**
- drift |B1_med − B2_med| / median = |9.44 − 6.30| / 8.25 = **0.3806**
- PASS criterion is `MAD/med ≤ 0.10 AND drift ≤ 0.10`. Both fail.
- `MAD/med > 0.25` **and** `drift > 0.25` → `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`
  (harness/host issue, not a Candidate issue).

## Comparison with the prior cycle

| attempt | window | MAD/med | drift | direction |
|---|---|---:|---:|---|
| sb1 (prior) | sibling-contaminated | 0.2201 | 0.3724 | B1 slower (10.18 > 7.00) |
| sb2 (prior) | sibling-contaminated | 0.1166 | 0.4257 | B1 slower (9.24 > 6.32) |
| **qb1 (this run)** | **exclusive quiet window** | **0.2861** | **0.3806** | **B1 slower (9.44 > 6.30)** |

Exclusivity removed the concurrent-sibling lease contention, but the block-1/block-2
asymmetry did **not** go away — the same systematic direction (B1 slower than B2,
~30-50% on the median) appears again with zero concurrent probes. So the failure is
reproduced under an exclusive window and is not attributable to sibling leases.

## Consequences

- No interleaved P/C run (runbook §5 only opens after (a) PASS).
- Do not treat this as a Route/Candidate failure; the shape's device-event noise floor
  is host/harness limited at ~7-10 µs kernel scale under resident VLLM.
- Raw samples permanent: `d4/sb1-parent-w10-s31-raw.tsv` (62 rows,
  sha256 `21438576f9684a3e0e86d2fe9923bcb6112adc8f5d91dacfa5908f3a7a6f3f5a`).
- Prior evidence kept untouched in `../results-qual-2x100/d4/`.
