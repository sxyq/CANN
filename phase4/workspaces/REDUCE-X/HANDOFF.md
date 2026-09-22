# REDUCE-X V001 HANDOFF

ROUTE: REDUCE-X
REVISION: V001
HYPOTHESIS: Multi-row simultaneous ReduceSum plus vector-side Rsqrt/Brcb scale removes the per-row V/S handoff (SyncVToS + GetValue) from the hot path. Large-D uses hierarchical two-pass reduce; generic fallback keeps the shared safe idiom for full 15-case coverage.
CHANGED: Fresh reduction-centric kernel. Batched multi-row ReduceSum into packed sum[B]. Zero-scalar scale via Brcb 8-wide inv block + 8-wide tiled Mul (0 GetValue). Hierarchical chunked sum(u*u) for D>8192 with vector-side inv. Epilogue fusion u*inv*gamma+bias. Rsqrt multiplier instead of Sqrt+Div. Private generic fallback with 1 GetValue per row.
COMPILE: device compile PASS; submission compile PASS; full link PASS (cann-server3, Ascend910B3, dav-2201, CANN 8.5.0.alpha002)
SOURCE: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/REDUCE-X/submission.asc
COMPILE_LOG: /Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/REDUCE-X/compile.log
CLEANUP: none required this round; remote /tmp/reduce_x holds build cache only
ONLINE_READY: yes
EXPECTED_AFFECTED_CASES: all 15 (full domain coverage via hot + wide + fallback)
REDUCTION_DESIGN: multi-row ReduceSum -> packed sum[B] -> vector Rsqrt -> Brcb 8-wide inv -> tiled Mul scale; wide path hierarchical chunk Add + final ReduceSum; fallback ReduceSum+GetValue after SyncVToS
V_S_HANDOFF_COUNT_PER_ROW: 0 on hot and wide paths; 1 on generic fallback

## Preflight

- 首行 `#include <cmath>` PASS
- 末行 `}` PASS
- `extern "C" void run_kernel` PASS
- `__global__ __vector__` PASS
- 无 DataCopyPadExtParams 聚合初始化 PASS
- SHA-256: 9617dd041122c6d1f83862d1eba2cbb2af1ebc5759cbe773adfcc1f06ec055bb
- 行数 542 / 字节 17972

## Portable mechanisms

1. Multi-row ReduceSum into packed sum[B] before any scalar use
2. Brcb 8-wide inv block + 8-wide tiled Mul as zero-GetValue scale
3. Rsqrt-as-multiplier epilogue (no Div-by-sqrt)
4. Hierarchical chunk + final ReduceSum when D > single-tile residency
5. Batch amortize SyncVToS when a scalar fallback is required

## Next experiment

Wait for online 15-point data. If WA on large-D tails, inspect chunk ReduceSum scratch lifetime. If latency high on small-D, raise B and overlap MTE with reduce (TQue depth 2 is reserved but not yet double-buffered in V001).
