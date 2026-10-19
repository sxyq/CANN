# EPILOGUE-FUSE-X BOTTLENECK NOTE

DATE: 2026-09-27
SCOPE: Why three post-RMS epilogue revisions (V001 SCALE-FOLD, V002 VMLA-cached, V003 VMLA-uncached) produced no measurable win.

The frozen seed's second-pass epilogue runs **three dependent full-tile PIPE_V passes** per tile — `Muls(valueTile, invRms)`, `Mul(valueTile, gammaSlice)`, `Add(valueTile, biasSlice)` — each writing the same value tile. Every revision either relocated those passes without cutting them (V001), added a per-row parameter-reload + `SyncVToMTE2` tax that exceeded the fused-instruction gain (V002, +3.4% regression), or fused two passes into a `vmla` but then needed a full-tile copy-back to reach the store, restoring the original pass count (V003). The core arithmetic `y·invRms·gamma + bias` needs at least 2 vector ops (multiply + add) after the normalize; without a hardware FMA that writes the value tile directly, the chain does not drop below 3 passes without destroying a reusable buffer (reload tax) or adding a copy (pass-count tax).

| rev | mechanism | V passes/tile | valueTile writebacks | result | why it failed |
|---|---|---:|---:|---|---|
| V001 SCALE-FOLD | normalize → gamma scratch | 3 | 3 | ±5% noise | relocated, not reduced |
| V002 VMLA-cached | `vmla` into cached bias, reload per row | 2 | 1 | **+3.4% regression** | per-row bias reload + `SyncVToMTE2` tax |
| V003 VMLA-uncached | `vmla` into per-tile bias, copy back | 3 | 2 | in-noise | copy-back restores pass count; store-from-bias races deferred `SyncMTE3ToV` |

**Conclusion:** the post-RMS epilogue's UB writeback count is not the binding constraint on these short kernels (5–12 µs). The V-pipe critical path and host-interference noise floor (±5%) dominate. Further epilogue work needs either a hardware FMA with a direct value-tile destination or a reduction in the number of elements processed (mask/padding), not instruction reshuffling.
