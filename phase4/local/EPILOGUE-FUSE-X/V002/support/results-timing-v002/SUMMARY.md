# EPILOGUE-FUSE-X V002 TIMING SUMMARY

DATE: 2026-09-27 (UTC ~00:05–00:15)
ROUTE: EPILOGUE-FUSE-X / V002 (VMLA fused affine)
SOURCE_SHA: 3d10417497b387d42969a2980d2bf4b4ffbb7ad3a86767c081fdb1d08324030d
DEVICE: server3 d5 (lease R2-EPI-V002-TIMING). AICore 0%, VLLM resident.
METHOD: DEVICE_EVENT_PRIMARY. warmup=45, same-binary 2×41, P/C 41 samples × 6 pairs.
Parent `efx_ref_parent` SHA 71c7c5c0…; Candidate `efx_ref_v002` SHA 645f4277….

## Shape dispatch note

`ProcessFp32FullRowOutputPipelined` requires `rowWidth == 8192 && localRows > 1`.
With 40 cores, `localRows > 1` requires `rowCount > 40`. Of the requested shapes,
only **64×8192** exercises the VMLA path (24/40 cores have localRows=2).
The other shapes (2×8192, 4×8192, 8×8192) have localRows=1 and go through the
generic tiled path (V002 change does not apply there). 2×4096 and 2×256 are controls.

## Phase 1 — Same-binary (parent, 2×41)

Gate: MAD/median ≤ 0.10 AND |B1−B2|/median ≤ 0.10.

| shape | B1_med | B2_med | all_med | MAD/med | drift | verdict |
|---|---:|---:|---:|---:|---:|---|
| 2×8192 | 6.04 | 6.68 | 6.16 | 0.054 | 0.104 | FAIL (drift) |
| 4×8192 | 6.38 | 7.52 | 6.72 | 0.058 | 0.170 | FAIL (drift) |
| 8×8192 | 6.84 | 51.62 | 6.98 | 0.077 | 6.42 | FAIL (B2 outlier) |
| 2×4096 | 5.60 | 6.08 | 5.78 | 0.121 | 0.083 | FAIL (MAD) |
| 2×256 ctrl | 58.8 | 5.76 | 8.44 | 0.581 | 6.28 | FAIL (B1 outlier) |
| 64×8192 VMLA | 10.66 | 12.24 | 10.68 | 0.060 | 0.148 | FAIL (drift) |

All shapes fail the strict drift gate (short-kernel host-interference outliers).
MAD/med is acceptable (0.05–0.08) on the 8192-width shapes. P/C was run as
diagnostic evidence despite the gate failure.

## Phase 2 — Interleaved P/C (6 pairs, 41 samples)

### 64×8192 FP32 (VMLA path — primary)

| pair | order | P_med µs | C_med µs | Δ% |
|---:|---|---:|---:|---:|
| 1 | PC | 11.14 | 11.38 | +2.15 |
| 2 | CP | 10.58 | 11.12 | +5.10 |
| 3 | PC | 10.48 | 10.78 | +2.86 |
| 4 | CP | 10.88 | 11.28 | +3.68 |
| 5 | PC | 10.48 | 10.82 | +3.24 |
| 6 | CP | 10.58 | 11.32 | +6.99 |

**6/6 pairs: Candidate slower.** Median Δ = **+3.4%**. Direction fully consistent.
This is a **stable regression** on the VMLA path. The in-place accumulator
pattern (bias reload + SyncVToMTE2 per row) costs more than the fused `vmla`
saves on this shape.

### Controls (non-VMLA paths)

| shape | pair Δ% range | median Δ | interpretation |
|---|---|---|---|
| 2×8192 (generic) | −88.6 … +1.3 | −1.6% | P outlier in pair5; no systematic signal |
| 4×8192 (generic) | −4.4 … +164 | +10.8% | C outliers in pairs 1,4; noise-dominated |
| 8×8192 (generic) | −5.1 … +2.1 | −1.0% | mixed, small — confirms no change on generic path |
| 2×4096 (control) | −18.4 … +18.4 | −2.8% | mixed, noise band |
| 2×256 (control) | −24.5 … +58.5 | +5.0% | very noisy, no signal |

## Verdict

**LOCAL_VERDICT = LOCAL_REJECTED**

The one shape that exercises the VMLA path (64×8192) shows a consistent,
direction-stable **+3.4% median regression** across 6 interleaved pairs.
The fused `MulAddDst` in-place accumulator pattern (with required per-row bias
reload and `SyncVToMTE2`) is slower than the original `Mul` + `Add` on this
hardware. Controls confirm no systematic change on non-VMLA paths.

**ONLINE_WORTHY = NO.** Roll back to the frozen parent for the next revision.

## Evidence

- `results-timing-v002/samebinary-verdict.csv`
- `results-timing-v002/paired-deltas.csv`
- `results-timing-v002/*-raw.tsv`, `*-stats.txt`
- `results-timing-v002/npu-smi-start.txt`, `npu-smi-end.txt`
