# STORE-EPILOGUE-X V002 — timing summary (2026-09-27)

Device: server3 d6 (resident VLLM workers idle, AICore 0%). Protocol:
warmup=45, samples=41, same-binary blocks=2, 6 interleaved P/C pairs
(odd P→C, even C→P). Primary metric: ALL_DEVICE median_us. Negative
favors candidate.

## Gate reachability (which shapes the candidate actually differs on)

V002 differs from the parent ONLY in `ProcessWideFp32FullCacheRows`
output pass, and only when `tileCount >= 4`. That path is reached only
for wide FP32 (rowWidth > kCacheElems = 8192). Consequently:

| shape | path | gate | code vs parent |
|---|---|---|---|
| 1x32768 FP32 | wide FP32, tileCount=8 | ON | merged writeback |
| 2x8192 / 8x8192 FP32 | generic (8192 is not > 8192) | — | **byte-identical** |
| 2x6144 FP32 | generic cacheRow | — | **byte-identical** |
| 2x256 FP32 | ProcessNarrowMidOverlap | — | **byte-identical** |
| 1x16384 FP16 | ProcessWideLowPrecision | — | **byte-identical** |

So on five of six measured shapes the two binaries execute the same
code; their P/C deltas measure the noise floor, not the mechanism, and
cannot be regressions *caused by* the candidate.

## Results

| shape | band | sb P med/MAD | sb C med/MAD | sb gate | PAIRED_DELTA median | clean n | favor C/P | reading |
|---|---|---|---|---|---|---|---|---|
| 1x32768 FP32 | large (gate ON) | 12.28 / 0.28 | 11.41 / 0.44 | PASS | **-5.62%** | 6 | 6/0 | clear win kept (V001: -7.61%) |
| 2x8192 FP32 | identical code | 5.98 / 0.35 | 6.37 / 0.63 | PASS | -4.20% | 6 | 4/2 | noise (no mechanism) |
| 2x6144 FP32 | identical code | 6.29 / 1.16 | 5.77 / 0.54 | FAIL | -2.49% | 6 | 5/1 | noisy window; no regression possible |
| 8x8192 FP32 | identical code | 7.31 / 1.36 | 7.09 / 0.58 | FAIL | +1.19% | 5 | 2/3 | noisy window; no regression possible |
| 2x256 FP32 | identical code | 5.76 / 1.18 | 6.34 / 0.72 | FAIL | +4.47% | 6 | 2/4 | control, noisy window |
| 1x16384 FP16 | identical code | 7.45 / 0.46 | 7.32 / 0.28 | PASS | +0.84% | 6 | 2/4 | ≈ 0 as expected |

Per-pair deltas (%):

- 1x32768: -4.83, -14.33, -5.45, -5.79, -8.77, -2.02 (6/6 C)
- 2x8192: +2.03, -3.28, -9.01, -5.13, +2.61, -7.07
- 2x6144: -15.84, -2.48, +11.68, -9.67, -0.70, -2.50
- 8x8192: -5.31, +2.24, -2.61, +1.19, +1.52, [-73.44 out]
- 2x256: +6.37, -0.65, -3.32, +2.57, +31.64, +30.17
- 1x16384 FP16: +0.82, +1.06, -2.49, -5.15, +0.85, +7.52

## Acceptance check (Main's LOCAL_ACCEPTED criteria)

1. **1x32768 keeps clear win — MET.** -5.62%, 6/6 clean pairs favor C,
   sb PASS both sides. (V001 measured -7.61% on the same shape/protocol;
   both runs agree the win is real and tight.)
2. **2x6144 and 8x8192 no longer regress — MET.** The candidate is
   byte-identical to the parent on these paths (gate closed / path
   untouched), so no regression is possible by construction. Measured
   deltas sit in this round's noise (3 of 6 shapes had sb FAIL — a busier
   window than V001's all-PASS round). V001's medium-band regression
   (+5.16% / +1.93%) is gone because the merge no longer runs there.
3. **No new small/large regression — MET.** 2x256 and 1x16384 FP16 are
   identical-code controls; their spread (±30% outliers on 2x256) is
   window noise, not candidate effect.

## Verdict

- LOCAL_VERDICT = **LOCAL_ACCEPTED**
  (gate keeps the wide win, removes the medium-band cost; identical-code
  shapes prove no regression by construction)
- ONLINE_WORTHY = the mechanism class is safe to carry into a stack;
  no Official submission this cycle per Main instruction.
