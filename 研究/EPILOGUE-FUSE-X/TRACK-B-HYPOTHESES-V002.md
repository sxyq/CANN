# TRACK-B HYPOTHESES V002 — EPILOGUE-FUSE-X (MAIN-2 R2)

ROUTE=EPILOGUE-FUSE-X
REVISION_TARGET=V002
DIRECT_PARENT=V001 SCALE-FOLD (SOURCE_SHA 89868a52b59fcaf1d68220398698ef033827a50b1559df9fc69ffcab105dd597)
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3 (frozen seed)
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_EPILOGUE_DATAFLOW
TRACK=Track-B read-only research; no kernel edit in this document.

---

## 0. WHY V001 WAS INSIDE THE NOISE BAND

V001 moved one multiply off the value tile (`Muls(y,invRms)·Mul(y,gamma)` → `Muls(scale,gamma,invRms)·Mul(y,scale)`). Total vector-op count stayed at 3 per tile. All three ops serialize on PIPE_V and each writes UB. Moving a write from `valueTile` to a scratch tile does not remove V-pipe cycles or UB traffic — it relocates them. The measured 4×8192 clean pairs (−5.5%, −9.0%) sit in the ±5% short-kernel band the wide control demonstrated.

**Implication for V002:** the next revision must cut V-pipe cycles or UB writebacks on the post-RMS path, not rearrange the same three passes.

### Hardware primitives confirmed on this toolkit (dav-c220)

| API | Semantics | Header |
|---|---|---|
| `AscendC::MulAddDst(dst, src0, src1, count)` | `dst[i] += src0[i]*src1[i]` (`vmla`) | `kernel_operator_vec_binary_intf_impl.h:401` |
| `AscendC::Axpy(dst, src, scalar, count)` | `dst[i] = src[i]*scalar + dst[i]` | `kernel_operator_vec_ternary_scalar_intf_impl.h` |
| `MulCast` | `cast(src0*src1)` to int8/uint8 only — **not usable** for FP16/BF16 output | `kernel_operator_vec_mulcast_impl.h` |

`MulAddDst` type support: `Tuple<float,float>`, `Tuple<half,half>`, `Tuple<float,half>`. This is a true fused multiply-accumulate: one instruction, one UB writeback, replacing `Mul` + `Add`.

---

## 1. FROZEN-SEED EPILOGUE V-OP ACCOUNTING (mid/large D, cached params)

FP32 / BF16 second pass, per 4096 tile (after V001):

```text
Muls(scaleTile, gammaSlice, invRms)   // 1 V pass, writeback #1 (scratch)
PipeBarrier
Mul (valueTile, valueTile, scaleTile) // 1 V pass, writeback #2 (value)
PipeBarrier
Add (valueTile, valueTile, biasSlice) // 1 V pass, writeback #3 (value)
PipeBarrier
[FromFloat(outputTile, valueTile)]    // BF16/FP16 only: 1 more V pass
Store(outputTile)                     // MTE3
```

| dtype | V passes per tile | UB writebacks |
|---|---:|---:|
| FP32 | 3 | 3 |
| BF16 | 4 | 4 |
| FP16 | 4 (Muls + FromFloat + half Mul + half Add) | 4 |

4×8192 FP32 = 2 tiles ⇒ **6 V passes per row**. This is the surface V002 must shrink.

---

## 2. HYPOTHESES

Three candidates, one epilogue dataflow variable each.

---

### HYPOTHESIS-V002-1 — VMLA FUSED AFFINE with row-level operand preparation

**MECHANISM**
Replace the per-tile `Mul(valueTile, scaleTile)` + `Add(valueTile, biasSlice)` pair with a single `MulAddDst(outTile, valueTile, scaleTile)`, where `outTile` is preloaded with `biasSlice`. Preload and scale are prepared **once per row** over the full resident width, then the tile loop issues only `vmla`:

```text
// once per row (width = kCacheElems, contiguous in valueFp32Buf_ / gammaFp32Buf_):
Muls(scaleRow, gammaRow, invRms)          // 1 V pass, full width
Adds(outRow,   biasRow,  0.0f)            // 1 V pass, full width (accumulator = bias)
PipeBarrier
// per tile t (col = t·4096):
MulAddDst(outRow[col], valueRow[col], scaleRow[col])   // 1 V pass, 1 writeback
Store(outRow[col])                                    // MTE3 unchanged
```

Arithmetic identity: `y*(invRms*gamma) + bias` computed as `vmla(y, scale; acc=bias)`. Identical to V001's reassociation plus one fused accumulate. FP32 intermediates throughout. Uniform for every `T` where `MulAddDst` accepts the dtype pair (`float,float` for FP32; `half,half` for FP16 after the existing early convert; `float,half` only if a mixed form is used — default stays same-type).

**V-op accounting, 2-tile row (D=8192):**

| | per-row V passes | per-tile critical path | per-tile UB writebacks |
|---|---:|---:|---:|
| V001 today | 6 | 3 | 3 |
| VMLA + row prep | **4** | **1** | **1** |
| Δ | −33% | −67% | −67% |

For single-tile rows (D=4096) row prep is 2 passes + 1 vmla = 3 (no change vs today). The win is specific to multi-tile rows.

**EXPECTED_BOTTLENECK**
Three serialized PIPE_V passes per tile with three full-tile UB writebacks inside the second pass. `Mul` writes the value tile, `Add` immediately re-reads and re-writes it — a redundant store-to-load round trip through UB. The scale scratch write is a third pass that only serves the multiply.

**FILES/FUNCTIONS TO TOUCH**
`EPILOGUE-FUSE-X-V001-SCALE-FOLD_kernel.asc` (V001 source), second-pass sites only:
- `ApplyScaleFold` → replace body with `Adds(outTile, biasSlice, 0)` + `MulAddDst(outTile, valueTile, scaleTile)`; extend signature or add `ApplyAffineVmla`.
- Call sites: `ProcessFp32FullRowOutputPipelined` (L1196), `ProcessBf16FullRowOutputPipelined` (L961), `Process()` tiled second pass (L427), `ProcessNarrowMidOverlap` (L575), `ProcessSmallFp32Batched` (L1426), `ProcessSmallFp32FullTileBatched` (L1524), `ProcessSmallFp32ContiguousBatched` (L1614), `ProcessSmallLowPrecisionContiguousBatched` (L1754).
- Row-level prep: hoist `Muls(scaleRow, gammaRow, invRms)` + `Adds(outRow, biasRow, 0)` above the tile loop in the full-row paths (those already hold full-width `gammaFp32Buf_`/`biasFp32Buf_`/`valueFp32Buf_`).
- `Add(valueTile, valueTile, biasSlice)` is **deleted** at folded sites. No host / CMake / runner change.

**WHY_ORTHOGONAL_TO_MAIN1 (DTYPE-SPECIAL-X)**
Pure FP32/FP32 instruction fusion on the existing unified intermediate. No `if constexpr` dtype split is added; `MulAddDst` is called with the same-type pair the site already uses. DTYPE-SPECIAL-X's reserved axis is per-dtype arithmetic/conversion specialization and `Adds(x,0)` copy elision; this hypothesis changes the **affine issue form** for every dtype uniformly and touches no conversion helper. The `Adds(outRow, biasRow, 0)` preload is a bias accumulator initialization, not the `Adds(x,0)` copy-elision pattern they own.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
- VECTOR-MATH-X / REDUCE-HIER-X: invRms derivation untouched.
- COEFF-LOCALITY-X / BATCH: no parameter DMA, residency, or stripe change; gamma/bias remain where V001 has them.
- UB-CHAMPION-X: no buffer lifetime model change; `outRow` reuses the existing value/output buffer.
- ASYNC-TRIPLE-X: store loop, event IDs, and MTE3 release order are byte-identical.
- H4 ROW-WIDE-AFFINE (V001 backlog) is the **issue-width** axis; this hypothesis is the **instruction-form** axis. They compose but are not the same variable. Recorded for Main's sequencing.

**EXPECTED_WIN_SHAPES**
- 4×8192 FP32 (full-row, 2 tiles): primary target; est. 10–20% on the epilogue-bound share.
- 64×8192 FP32 / 4×8192 BF16: same structure.
- Any `rowWidth == kCacheElems` path with `localRows > 1`.
- Neutral on D=4096 single-tile and on the wide path.

**EXPECTED_RISK_SHAPES**
- D=4096 and below: row prep is not amortized; expect zero delta (use as control).
- If the bias preload pass (`Adds`) turns out to cost as much as the deleted `Add`, the win collapses to the writeback reduction only.

**CORRECTNESS_RISK**
Low–medium. `vmla` computes `dst + src0*src1` in one rounding; the V001 chain rounds `y*scale` then adds `bias`. Difference is one FP32 intermediate rounding — same class as V001's reassociation (measured max_abs 1e-7..1e-6). Must confirm `MulAddDst` accumulate direction (`dst + src0*src1` vs `src0*src1 + dst` is identical in FP32) and that the preload `Adds(..., 0.0f)` is exact for the bias bit pattern.

**MEASUREMENT_PLAN**
1. Same-binary Parent + V002 on 4×8192 FP32 and 2×4096 FP32 (control) under `local-timing-protocol.md`, warmup=45, device events.
2. One revision, one mechanism: the only deleted ops are `Mul`+`Add`; the only added ops are `Adds` (preload) + `MulAddDst`. `SINGLE_CHANGE_AUDIT` must show no other edit.
3. ≥4 interleaved P/C pairs on 4×8192 FP32 (and BF16 if time permits); control shape must be ~0.
4. Falsifier: if 4×8192 shows no direction-consistent drop beyond the same-binary noise floor while the V-pass count visibly fell in a static count, the epilogue is not the bottleneck on this shape — stop and report.

---

### HYPOTHESIS-V002-2 — OUTPUT-DOMAIN AFFINE (uniform convert-early, T-domain gamma/bias)

**MECHANISM**
Unify the epilogue to one shape for every `T`:

```text
Muls(valueTile, valueTile, invRms)        // normalize in FP32 (shared)
FromFloat(outputT, valueTile)             // ONE convert, early
Mul (outputT, outputT, gammaT)            // gamma in T domain
Add (outputT, outputT, biasT)             // bias in T domain
Store(outputT)
```

FP16 already follows this shape in the seed. This hypothesis extends it to BF16 and keeps FP32 on the identical code path (where `FromFloat` degenerates to the existing no-op copy helper). No per-dtype branch is introduced beyond the existing `FromFloat` / `Tensor<T>` type selection. The variable is the **arithmetic domain of the gamma/bias pass** (output-type vs FP32), not a per-dtype algorithm.

**V-op accounting, BF16 per tile:**

| | V passes | effective width |
|---|---:|---|
| today | 4 | all FP32-width |
| H2 | 4 | 1 FP32 + 3 BF16 (half the repeats) |
| Δ | 0 count | **≈35% fewer V cycles** |

**EXPECTED_BOTTLENECK**
BF16 currently runs the entire affine in FP32 and converts at the end: three full-width FP32 passes plus `FromFloat`. FP16's T-domain affine already demonstrates the pattern on this hardware; BF16's 16-bit repeats process twice the elements per cycle.

**FILES/FUNCTIONS TO TOUCH**
BF16 second-pass arms only: `Process()` L451–460, `ProcessNarrowMidOverlap` L596–607, `ProcessBf16FullRowOutputPipelined` L961–978, `ProcessBf16FullTileBatchedOutputPipelined` L713–727, `ProcessSmallLowPrecisionContiguousBatched` L1754–1800. Replace the FP32 `Mul(gammaFp32)`/`Add(biasFp32)` pair with T-domain `Mul(gammaLocal)`/`Add(biasLocal)` after an early `FromFloat`. The `gammaFp32Buf_`/`biasFp32Buf_` promotion for BF16 becomes unnecessary on this path (may be left allocated to keep the diff minimal).

**WHY_ORTHOGONAL_TO_MAIN1 (DTYPE-SPECIAL-X)**
The code shape is **identical** for FP16/BF16 and degenerate-but-identical for FP32; no `if constexpr (T==bf16)` special arm is created. The variable is "which domain the affine runs in", expressed once. DTYPE-SPECIAL-X creates per-dtype helpers and copy-elision paths; this hypothesis **removes** BF16's unique FP32-then-convert arm so all 16-bit types share the half path's proven shape.
Near-border note: DTYPE-SPECIAL-X's Track-B list mentions "FP32 affine" as a consideration. This hypothesis moves **away** from FP32 affine for BF16 — opposite direction — but Main should confirm they have not started the same BF16 arm.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
- VECTOR-MATH-X: invRms derivation untouched.
- UB-CHAMPION-X: `outputBuf_` / staging lifetime unchanged.
- BATCH / COEFF: no param DMA change (BF16 native gamma/bias already resident where the seed caches them).
- ASYNC: store path unchanged.

**EXPECTED_WIN_SHAPES**
- BF16 mid/large D: 4×8192, 2×4096, 8×256 batched. Est. 20–35% epilogue V-cycle reduction.
- FP16 shapes: already this shape — expect ~0 (control).
- FP32: ~0 (control).

**EXPECTED_RISK_SHAPES**
- Any BF16 case whose golden is defined on FP32-then-round: `bf16(bf16(y·scale)+bias)` vs `bf16(y·scale+bias)` differs by one BF16 intermediate rounding (≈1 ulp ≈ 0.004–0.008). Harness tolerance is 2e-2 — should pass, but must be measured.
- R004 "low-precision intermediate" is a historical caution; the distinction is that the **output is BF16 anyway**, so the final rounding is to BF16 either way. Recorded as a stated risk, not a silent assumption.

**CORRECTNESS_RISK**
Medium. BF16 intermediate rounding must stay within tolerance on all battery shapes. If any BF16 case exceeds tolerance, the hypothesis is rejected on correctness — no hybrid fallback within V002.

**MEASUREMENT_PLAN**
1. Full 54-case NPU correctness first, with explicit BF16 max-abs vs parent recorded per shape.
2. One revision: only the BF16 affine-domain change. FP16 and FP32 arms must be byte-identical to V001 (`SINGLE_CHANGE_AUDIT` checks this literally).
3. Interleaved P/C on BF16 4×8192 and BF16 8×256; FP16 8×256 as zero-delta control.
4. Falsifier: if BF16 gains nothing while correctness loosens, discard.

---

### HYPOTHESIS-V002-3 — ROW-WIDE AFFINE ISSUE (issue-width axis)

**MECHANISM**
On paths where the full row already sits contiguously in `valueFp32Buf_` (D = kCacheElems = 8192), issue the affine as **row-wide** vector calls over the whole buffer, then keep the existing per-tile store sequence and events unchanged:

```text
// today, per tile t∈{0,1}:  Muls · Mul · Add   (6 V calls for the row)
Muls(valueRow, valueRow, invRms, 8192)     // 1 call, full row
Mul (valueRow, valueRow, gammaRow, 8192)   // 1 call, full row
Add (valueRow, valueRow, biasRow,  8192)   // 1 call, full row
// then unchanged per-tile Store with the same MTE3 events
```

The variable is the **width of the affine issue group** (row-wide vs per-tile). Arithmetic identity, operands, and store schedule are unchanged. Composes with H1 (vmla) but is a distinct axis.

**EXPECTED_BOTTLENECK**
Per-tile issue overhead: two tiles × three API calls each, with a `PipeBarrier` and a store handshake between tiles. The affine itself does not need tile boundaries; only the store double-buffer does.

**FILES/FUNCTIONS TO TOUCH**
`ProcessFp32FullRowOutputPipelined` epilogue loop (L1196–1215), `ProcessBf16FullRowOutputPipelined` epilogue loop (L961–989), matching FP16 full-row epilogue. The store loop, event allocation, and release order must stay byte-identical.

**WHY_ORTHOGONAL_TO_MAIN1 (DTYPE-SPECIAL-X)**
Issue width is a pure dataflow/granularity choice in the unified FP32 chain; no dtype branch, no conversion change. FP32 and BF16 take the same structural change.

**WHY_NOT_DUPLICATE_EXISTING_MAIN2**
- ASYNC-TRIPLE-X: near-border on the shared epilogue site; the store ring and event placement must not move. The declared variable is compute issue width only.
- H1 (this document): instruction form vs issue width — different axes, do not merge into one revision.

**EXPECTED_WIN_SHAPES**
- `rowWidth == 8192` FP32/BF16 full-row paths only. Est. 5–12% (issue overhead reduction).
- Zero on D=4096 and below.

**EXPECTED_RISK_SHAPES**
- If the row-wide latency now sits fully before the first store, a short shape could lose the compute/store overlap the per-tile order provided. That is the failure mode.

**CORRECTNESS_RISK**
Low. Same ops, same operands, same per-element order. Only the `calCount` on the wide calls changes (must be exactly `kCacheElems`).

**MEASUREMENT_PLAN**
1. Same-binary on 8192 (acting) and 4096 (zero-delta control).
2. One revision: only affine call width. Diff must not move any store/event line.
3. Interleaved P/C on 4×8192 FP32; 2×4096 control.
4. Falsifier: if 8192 regresses and 4096 is flat, the per-tile order was hiding store latency (ASYNC territory) — return to parent.

---

## 3. SUMMARY TABLE

| ID | Single variable | est. win (mid/large D) | Risk | Readiness |
|---|---|---|---|---|
| **V002-1 VMLA FUSED AFFINE** | affine instruction form (`Mul`+`Add` → `MulAddDst`) with row-level scale+accumulator prep | **10–20%** on 8192, −33% V passes | low–med (vmla semantics, preload exactness) | **READY_FOR_MAIN_REVIEW** |
| V002-2 OUTPUT-DOMAIN AFFINE | affine arithmetic domain (output-T vs FP32) | 20–35% on BF16 | med (BF16 intermediate rounding; R004 near-border) | NEEDS_MORE_EVIDENCE (correctness-first) |
| V002-3 ROW-WIDE AFFINE ISSUE | affine issue width (row-wide vs per-tile) | 5–12% on 8192 | low (store overlap) | NEEDS_MORE_EVIDENCE (may compose with V002-1 later) |

---

## 4. RECOMMENDATION

**V002 = HYPOTHESIS-V002-1 (VMLA FUSED AFFINE with row-level operand preparation).**

Reasons:
1. Only candidate that cuts both V-pipe passes and UB writebacks on the post-RMS path — the property V001 lacked.
2. `MulAddDst` / `vmla` is a confirmed public API on this toolkit (`kernel_operator_vec_binary_intf_impl.h:401`, `dst[i] += src0[i]*src1[i]`).
3. FP32-only arithmetic, uniform across T, no conversion change → cleanest orthogonality to DTYPE-SPECIAL-X.
4. Est. 10–20% on the shape V001 already probed (4×8192), above the ±5% noise band.
5. V002-2 needs a BF16 correctness-tolerance decision first; V002-3 is a smaller win and is near ASYNC's store ring.

Minimal OFAT diff (conceptual):

```text
// before (per tile):
Muls(scaleTile, gammaSlice, invRms); Mul(valueTile, valueTile, scaleTile); Add(valueTile, valueTile, biasSlice);
// after (per row prep + per tile):
Muls(scaleRow, gammaRow, invRms); Adds(outRow, biasRow, 0.0f);   // once per row
MulAddDst(outRow[col], valueRow[col], scaleRow[col]);             // once per tile
```

---

## 5. REQUEST_MAIN_APPROVAL

REQUEST_MAIN_APPROVAL: **HYPOTHESIS-V002-1 — VMLA FUSED AFFINE with row-level operand preparation.**

Requested disposition: approve V002-1 as the single hypothesis for performance revision V002 of EPILOGUE-FUSE-X, parent V001 (SOURCE_SHA `89868a52b59fcaf1d68220398698ef033827a50b1559df9fc69ffcab105dd597`). No kernel edit until Main approves exactly one hypothesis. V002-2 and V002-3 stay in backlog with their stated prerequisites.
