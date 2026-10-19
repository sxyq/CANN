# STORE-EPILOGUE-X V001 — timing summary (2026-09-27)

Device: server3 d6. Protocol: warmup=45, samples=41, same-binary blocks=2,
6 interleaved P/C pairs (odd P→C, even C→P). Primary metric: ALL_DEVICE
median_us. Clean pair: |delta| ≤ 50%. Negative favors candidate.

| shape | band | sb P med/MAD | sb C med/MAD | sb gate | PAIRED_DELTA median | clean n | favor C/P | reading |
|---|---|---|---|---|---|---|---|---|
| 1x32768 FP32 | large | 12.00 / 0.38 | 11.26 / 0.29 | PASS | **-7.61%** | 6 | 6/0 | clean win (wide site, 8 tiles → 1 writeback) |
| 2x8192 FP32 | large | 6.14 / 0.20 | 6.52 / 0.74 | PASS | +2.57% | 5 | 2/3 | no signal (1 outlier out) |
| 2x6144 FP32 | medium | 6.01 / 0.33 | 5.98 / 0.50 | PASS | +5.16% | 6 | 1/5 | parent-favoring direction |
| 8x8192 FP32 | medium | 6.90 / 0.32 | 6.44 / 0.20 | PASS | +1.93% | 6 | 2/4 | mild parent-favoring |
| 2x256 FP32 | small | 5.57 / 0.57 | 5.56 / 0.51 | PASS | -6.06% | 6 | 4/2 | control — mechanism not applied here, must be noise |
| 1x16384 FP16 | large (untouched path) | 7.18 / 0.24 | 8.01 / 0.79 | PASS | +0.42% | 6 | 3/3 | ≈ 0 as expected (lowp wide path unchanged) |

Per-pair deltas (%):

- 1x32768: -10.76, -7.35, -7.31, -6.71, -8.82, -7.87 (tight, all C)
- 2x6144: +11.87, +7.75, +2.57, +0.00, -7.36, +16.56
- 8x8192: +19.56, -0.58, +3.24, -3.61, +3.90, +0.61
- 2x8192: +5.00, [+777.53 out], +2.57, +20.61, -7.40, -8.38
- 2x256: +4.68, +4.49, -14.63, -4.08, -8.05, -20.72
- 1x16384 FP16: +3.50, +1.88, -3.45, -8.15, -1.03, +6.58

## Reading

- Large band, wide-FP32 site (1x32768): the merge delivers. 8 per-tile
  stores + handshakes collapse to 1 per row; 6/6 clean pairs, range
  [-10.8, -6.7], both same-binary medians already favor C (12.00 vs 11.26).
- Medium band (tileCount=2): mild but direction-consistent parent-favoring
  (2x6144 5/6 P-side pairs, 8x8192 4/6). Mechanism reading: at tileCount=2
  the descriptor saving is 1 call, while deferring the writeback past
  tile 1 gives up the parent's V006 store/compute overlap — net cost.
- Small control (2x256, ProcessNarrowMidOverlap — untouched by the diff)
  moves -6.06% with range [-20.7, +4.7]: the short-kernel noise floor on
  this device is ~±10%, which both tempers the medium-band reading and
  makes the tight 1x32768 band stand out as the real signal.

## Verdict

- LOCAL_VERDICT = NEEDS_ONE_MORE_LOCAL
  (one band wins cleanly, the critical medium band leans the other way
  at noise-floor magnitude; small control proves the noise floor)
- ONLINE_WORTHY = NO
  (criterion "wins on multiple shape classes without regression" not met)
- Follow-up candidate if Main wants V002: threshold the merge on
  tileCount (merge only when tileCount ≥ 4), one new variable, so the
  wide win is kept and the medium-band deferred-store cost is avoided.
  That is a new revision — Main approval required.
