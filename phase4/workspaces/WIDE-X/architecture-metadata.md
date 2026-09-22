# WIDE-X V001 — Hierarchical UB Reduction

## Route

Non-D-slice wide-D specialist. Fresh workspace. Target Ascend 910B3 / DAV_2201.

## Hypothesis

Wide-D AddRmsNormBias is bound by sum-of-squares reduction latency and by
re-reading x/residual for the normalize stage. D-slice across cores does not
pay for T14-class (few-row, wide D) because cross-core combine and SyncAll
overhead dominate.

V001 explores **hierarchical UB reduction** with single-core full/near-full `u`
residency:

1. Each vector core owns whole rows (row-block split). No D-slice, no SyncAll.
2. `u = x + residual` is kept resident in UB as FP32 when the row fits the
   184 KiB budget (true for all legal D ≤ 32768 after fixed buffers).
3. `sum(u*u)` is a two-level UB tree: leaf `ReduceSum` over 256-wide groups into
   an 8B-aligned leaf slot, then running `Add` combine.
4. `inv_rms = Rsqrt(mean(u*u)+eps)` via one-element vector Rsqrt.
5. Epilogue `y = u * inv_rms * gamma + bias` streams gamma/bias tiles —
   x/residual are not re-read on the hot path.
6. Generic stream fallback recomputes `u` per leaf when residency does not apply.
7. dtype-specific compute: FP32 native; FP16/BF16 upcast to FP32, round on store.

## Difference from R31 D-slice thinking

- No D-stripe / no more-cores / no slice-count sweep.
- No cross-core partial GM combine and no global SyncAll.
- Reduction hierarchy is inside one core's UB, not across cores.

## Files

- `submission.asc` — judge source (`#include <cmath>` first, `run_kernel`,
  `extern "C" __global__ __vector__ wide_x_hier_ub`, no TensorInfo redefine).
- `local_types.h` / `device_shim.asc` / `submission_shim.asc` — local compile only.
- `main.asc` — full-link harness (rank-3 FP16 D=2048 smoke).
- `CMakeLists.txt` — device / submission / full-link targets.
- `scripts/build_server3.sh` — three-stage compile on cann-server3.
- `scripts/gen_data_pure.py` / `scripts/verify_pure.py` — smoke golden without numpy.

## Smoke verification

Rank-3 FP16 `(2,3,2048)` on 910B3: `max_abs=0.0078125`, `bad=0` vs golden.
