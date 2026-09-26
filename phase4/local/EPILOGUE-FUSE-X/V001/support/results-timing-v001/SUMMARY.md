# EPILOGUE-FUSE-X V001 TIMING SUMMARY

DATE: 2026-09-26 (UTC window ~22:15–22:25)
ROUTE: EPILOGUE-FUSE-X
REVISION: V001 (SCALE-FOLD)
SOURCE_SHA: 89868a52b59fcaf1d68220398698ef033827a50b1559df9fc69ffcab105dd597
DEVICE: server3 d5 (lease R2-EPI-V001-TIMING). AICore 0%, HBM 59876/65536, VLLM resident on d0–d3; d4 used by SCHED concurrently (different device). d7 excluded.
METHOD: DEVICE_EVENT_PRIMARY + HOST_WALL_SECONDARY. warmup=45, same-binary 2×31 samples, P/C 21 samples × ≥4 interleaved pairs. Parent binary `efx_ref_parent` SHA 71c7c5c0…; Candidate binary `efx_ref_v001` SHA 7dc9f2d5….

## Phase 1 — Same-binary qualification (Parent binary)

Gate: full-sample MAD/median ≤ 0.10 AND |B1−B2|/median ≤ 0.10.

| shape | rows×D | dtype | B1_med | B2_med | MAD_us | MAD/med | drift | verdict |
|---|---|---|---:|---:|---:|---:|---:|---|
| sb-fp32-8x256 | 8×256 | FP32 | 5.46 | 5.62 | 0.62 | 0.114 | 0.029 | **FAIL** |
| sb-fp32-2x4096 | 2×4096 | FP32 | 5.42 | 7.68 | 1.78 | 0.260 | 0.330 | **FAIL** |
| sb-fp32-4x8192 | 4×8192 | FP32 | 6.58 | 6.70 | 0.31 | 0.047 | 0.018 | **PASS** |
| sb-fp16-8x256 | 8×256 | FP16 | 5.08 | 6.08 | 1.33 | 0.235 | 0.177 | **FAIL** |
| sb-bf16-8x256 | 8×256 | BF16 | 7.36 | 7.44 | 1.41 | 0.190 | 0.011 | **FAIL** |
| sb-ctrl-wide-2x16384 | 2×16384 | FP32 | 8.42 | 8.48 | 0.31 | 0.037 | 0.007 | **PASS** (control) |

Short kernels (5–7 µs device) show host-interference tails (p90 115–165 µs vs p10 5–6 µs; wall 128–177 µs). Four of six shapes fail the MAD/med gate → `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE`, P/C skipped per protocol.

## Phase 2 — Interleaved P/C (shapes with same-binary PASS only)

### 4×8192 FP32 (fold applies — full-row pipelined epilogue)

| pair | order | P_med_us | C_med_us | delta |
|---:|---|---:|---:|---:|
| 1 | PC | 13.96 | 6.70 | −52.01% |
| 2 | CP | 6.48 | 6.60 | +1.85% |
| 3 | PC | 7.30 | 6.64 | −9.04% |
| 4 | CP | 6.54 | 6.18 | −5.50% |

Parent's pair-1 median (13.96 µs) is ~2× its own same-binary median (6.58–6.70) — a parent-side outlier sample; protocol forbids dropping it. Clean pairs 2–4: +1.85%, −9.04%, −5.50% (2/3 favor Candidate). Both candidate and clean-parent medians sit inside the same-binary noise band (MAD/med 0.047 ⇒ ~0.3 µs on 6.6 µs).

### 2×16384 FP32 wide control (fold does NOT apply)

| pair | order | P_med_us | C_med_us | delta |
|---:|---|---:|---:|---:|
| 1 | PC | 8.22 | 8.12 | −1.22% |
| 2 | CP | 8.06 | 7.72 | −4.22% |
| 3 | PC | 8.30 | 8.06 | −2.89% |
| 4 | CP | 7.94 | 8.56 | +7.81% |

Mixed signs, all within ±8%. Same-binary noise floor here MAD/med 0.037. Confirms the measurement band is roughly ±5% on these short kernels; the 4×8192 clean-pair deltas sit at the edge of that band.

## Verdict

**LOCAL_VERDICT = NEEDS_ONE_MORE_LOCAL**

- 4/6 shapes `MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE` (short-kernel MAD/med gate).
- The one fold-active shape with valid same-binary (4×8192 FP32) shows mixed P/C: 1 parent-side outlier pair, 2/3 clean pairs favor Candidate but within the noise band the wide control demonstrates.
- No shape reaches a direction-consistent improvement beyond the same-binary noise floor.
- Not LOCAL_REJECTED: no stable regression. Not LOCAL_ACCEPTED: no clean win.

**ONLINE_WORTHY = NO** (KEEP_ACCUMULATING / NEEDS_ONE_MORE_LOCAL).

## Next action

Re-run same-binary + P/C for the blocked short shapes under a quieter window or with the per-shape noise-floor procedure (`local-timing-protocol.md`: same-binary noise floor before deciding fixed CV≤0.15). For 4×8192, add ≥4 more interleaved pairs to resolve the marginal signal. No new performance revision until Main dispositions V001.

## Evidence

- `results-timing-v001/samebinary-verdict.csv`
- `results-timing-v001/paired-deltas.csv`
- `results-timing-v001/*-raw.tsv` (all samples permanent)
- `results-timing-v001/*-stats.txt` (per-block stats)
- `results-timing-v001/npu-smi-start.txt`, `npu-smi-end.txt`
