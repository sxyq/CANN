# EPI-X Architecture Metadata

- ROUTE: EPI-X
- IDEA POOL: R010 + R027 (manual tail + GPU-style epilogue fusion)
- TARGET: Ascend 910B3 / DAV_2201
- REVISION: V001

## FUSION_STAGES

1. FormU: one GM read of x and residual into resident FP32 `u`.
2. SumInv: `ReduceSum(u*u)` with software-masked tail; `invRms = Rsqrt(mean+eps)` + one `GetValue`.
3. FusedEpilogue: `out = u * invRms * gamma + bias` in one vector pass over resident `u`. No intermediate full-row norm store.

## MASK_STRATEGY

- DataCopyPad GM→UB uses 4-arg form with `rightPadding` in BYTES (32B tail fill with `T{}`).
- Explicit software mask: `Duplicate` zeros FP32 lanes `[D, align8(D))` on resident `u` before sum and scale.
- UB→GM uses 3-arg DataCopyPad and stores only valid bytes.

## Why not two-pass reload

GPU-style epilogue fusion keeps normalized `u` resident so gamma/bias apply in the same vector pass as the invRms scale. Avoids second GM traffic of x/residual and avoids any norm intermediate in GM or a second full-row UB buffer.

## ABI hard requirements covered

- first line `#include <cmath>`
- `extern "C" void run_kernel(...)` exact signature
- `__global__ __vector__` entries; launch `<<<blockCount, nullptr, stream>>>`
- no TensorInfo / TensorGroupInfo redefinition in submission
- dtype 0/1/2 and 27→BF16
- ReduceSum 8KiB tmp, dest 8B-aligned
- no `static_cast` bf16↔float; vector Cast / ToFloat / FromFloat
- no `sqrtf` in `__aicore__`; Rsqrt + GetValue
- accumulators zeroed with `Duplicate`, not `Adds(..., 0)`
- SyncVToS / SyncSToV around GetValue

## UB budget (184 KiB vec local)

- resident u (D=32768 FP32): 128 KiB
- load/store tiles (1024 or 2048 elems): ≤ 40 KiB
- scratch FP32 tile: ≤ 8 KiB
- ReduceSum tmp: 8 KiB
- scalars/acc: < 1 KiB
- peak ≈ 160–184 KiB depending on tile; tile chosen per dtype
