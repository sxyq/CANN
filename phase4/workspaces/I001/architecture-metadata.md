# I001 V003 Architecture Metadata

candidate: I001
revision: V003
main_change: ABI-matched minimal kernel (GM_ADDR, const TensorGroupInfo&, int64_t cores, aclrtStream)

architecture hypothesis: Online CE was ABI/template shape, not wide-D math. V003 copies only B001's entry/ABI shape and keeps a simple two-pass row-parallel body.

core mapping: row-parallel, contiguous row ranges.
blockDim: min(availableCoreNum, 40, rows).

UB: 3 TQue tiles + 2 float tiles + 128B reduce tmp. No retained full-row path in V003 (minimize).

reduction: intra-core ReduceSum(sum(u*u)). No cross-core sync.

x/residual rereads: 2 (two-pass).
gamma/bias reloads: 1 per D-tile per row.

hot path: all legal inputs use the same two-pass body (fp32/fp16/bf16 via I003Ops).
fallback: same body.

template shape (from online-pass B001, architecture not copied):
- #include <cmath> then kernel_operator.h
- extern "C" __global__ __vector__ with GM_ADDR args
- extern "C" void run_kernel(GM_ADDR, const TensorGroupInfo&, ..., int64_t availableCoreNum, aclrtStream, float)
- DataCopyExtParams / DataCopyPadExtParams default-ctor + field assign
- Cast<float,T> / Cast<T,float> with CAST_NONE / CAST_ROUND
- no if constexpr, no auto, no nested templates, no acl.h, no aggregate = {

online identity:
lines=277 bytes≈9480
see preflight SHA-256 in build handoff
