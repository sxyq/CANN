#include <cmath>
#include <cstdint>
#include "kernel_operator.h"

namespace g001 {

// Judge TensorInfo.dtype contract: 0=FP32, 1=FP16, 2=BF16.
// Internal dtype codes follow the same encoding so run_kernel can pass through.
struct TilingData {
    int32_t rows;
    int32_t dim;
    float epsilon;
    uint32_t dtype;
    uint32_t mode;
    int32_t reserved[2];
};

template <typename T>
__aicore__ inline float ToFloat(T value)
{
    return static_cast<float>(value);
}

template <>
__aicore__ inline float ToFloat<bfloat16_t>(bfloat16_t value)
{
    const uint16_t raw = reinterpret_cast<const uint16_t &>(value);
    const uint32_t bits = static_cast<uint32_t>(raw) << 16;
    union Bits {
        uint32_t u;
        float f;
    } bitsValue;
    bitsValue.u = bits;
    return bitsValue.f;
}

template <typename T>
__aicore__ inline T FromFloat(float value)
{
    return static_cast<T>(value);
}

template <>
__aicore__ inline bfloat16_t FromFloat<bfloat16_t>(float value)
{
    union Bits {
        uint32_t u;
        float f;
    } bitsValue;
    bitsValue.f = value;
    // Round-to-nearest-even float -> bfloat16 (BF16 value lives in bits 31:16).
    const uint32_t roundingBias = 0x7FFFu + ((bitsValue.u >> 16) & 1u);
    bitsValue.u += roundingBias;
    const uint16_t result = static_cast<uint16_t>(bitsValue.u >> 16);
    return reinterpret_cast<const bfloat16_t &>(result);
}

__aicore__ inline float SqrtF(float x)
{
    return __builtin_cce_sqrtf(x);
}

// Hot path (Fresh One-Read / Resident-Y):
// each x/residual element is consumed once; FP32 u = x + residual stays
// resident in one UB row and is reused for RMS and for norm + bias.
// All arithmetic is FP32; one native cast at output.
template <typename T>
__aicore__ inline void RunResidentRows(GM_ADDR xAddr, GM_ADDR residualAddr, GM_ADDR gammaAddr, GM_ADDR biasAddr,
    GM_ADDR outputAddr, const TilingData &cfg)
{
    AscendC::GlobalTensor<T> x;
    AscendC::GlobalTensor<T> residual;
    AscendC::GlobalTensor<T> gamma;
    AscendC::GlobalTensor<T> bias;
    AscendC::GlobalTensor<T> output;
    const uint64_t total = static_cast<uint64_t>(cfg.rows) * cfg.dim;
    x.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(xAddr), total);
    residual.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(residualAddr), total);
    gamma.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(gammaAddr), cfg.dim);
    bias.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(biasAddr), cfg.dim);
    output.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(outputAddr), total);

    AscendC::LocalMemAllocator<> allocator;
    AscendC::LocalTensor<float> resident = allocator.Alloc<float>(cfg.dim);

    const uint64_t core = static_cast<uint64_t>(AscendC::GetBlockIdx());
    const uint64_t cores = static_cast<uint64_t>(AscendC::GetBlockNum());
    for (uint64_t row = core; row < static_cast<uint64_t>(cfg.rows); row += cores) {
        const uint64_t rowBase = row * static_cast<uint64_t>(cfg.dim);

        // V008 single change vs V002: pairwise (binary-counter) summation of
        // u*u instead of a single sequential accumulator.  O(log D) rounding
        // vs O(D) — matches vectorised golden reductions more closely.
        float partial[32] = {0.0f};
        for (int32_t i = 0; i < cfg.dim; ++i) {
            const float u = ToFloat(x.GetValue(rowBase + i)) + ToFloat(residual.GetValue(rowBase + i));
            resident.SetValue(i, u);
            float val = u * u;
            unsigned n = static_cast<unsigned>(i);
            int k = 0;
            while (n & 1u) {
                val = partial[k] + val;
                partial[k] = 0.0f;
                n >>= 1;
                ++k;
            }
            partial[k] = val;
        }
        float sumSquares = 0.0f;
        for (int k = 31; k >= 0; --k) {
            sumSquares += partial[k];
        }

        const float mean = sumSquares / static_cast<float>(cfg.dim);
        const float rms = SqrtF(mean + cfg.epsilon);
        for (int32_t i = 0; i < cfg.dim; ++i) {
            const float u = resident.GetValue(i);
            const float value = (u / rms) * ToFloat(gamma.GetValue(i)) + ToFloat(bias.GetValue(i));
            output.SetValue(rowBase + i, FromFloat<T>(value));
        }
    }
}

// Generic fallback: two-pass over sources, same arithmetic as the hot path.
template <typename T>
__aicore__ inline void RunGenericRows(GM_ADDR xAddr, GM_ADDR residualAddr, GM_ADDR gammaAddr, GM_ADDR biasAddr,
    GM_ADDR outputAddr, const TilingData &cfg)
{
    AscendC::GlobalTensor<T> x;
    AscendC::GlobalTensor<T> residual;
    AscendC::GlobalTensor<T> gamma;
    AscendC::GlobalTensor<T> bias;
    AscendC::GlobalTensor<T> output;
    const uint64_t total = static_cast<uint64_t>(cfg.rows) * cfg.dim;
    x.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(xAddr), total);
    residual.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(residualAddr), total);
    gamma.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(gammaAddr), cfg.dim);
    bias.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(biasAddr), cfg.dim);
    output.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(outputAddr), total);

    const uint64_t core = static_cast<uint64_t>(AscendC::GetBlockIdx());
    const uint64_t cores = static_cast<uint64_t>(AscendC::GetBlockNum());
    for (uint64_t row = core; row < static_cast<uint64_t>(cfg.rows); row += cores) {
        const uint64_t rowBase = row * static_cast<uint64_t>(cfg.dim);
        float partial[32] = {0.0f};
        for (int32_t i = 0; i < cfg.dim; ++i) {
            const float u = ToFloat(x.GetValue(rowBase + i)) + ToFloat(residual.GetValue(rowBase + i));
            float val = u * u;
            unsigned n = static_cast<unsigned>(i);
            int k = 0;
            while (n & 1u) {
                val = partial[k] + val;
                partial[k] = 0.0f;
                n >>= 1;
                ++k;
            }
            partial[k] = val;
        }
        float sumSquares = 0.0f;
        for (int k = 31; k >= 0; --k) {
            sumSquares += partial[k];
        }
        const float mean = sumSquares / static_cast<float>(cfg.dim);
        const float rms = SqrtF(mean + cfg.epsilon);
        for (int32_t i = 0; i < cfg.dim; ++i) {
            const float u = ToFloat(x.GetValue(rowBase + i)) + ToFloat(residual.GetValue(rowBase + i));
            const float value = (u / rms) * ToFloat(gamma.GetValue(i)) + ToFloat(bias.GetValue(i));
            output.SetValue(rowBase + i, FromFloat<T>(value));
        }
    }
}

} // namespace g001

extern "C" __global__ __vector__ void g001_add_rms_norm_bias(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    int32_t rows, int32_t dim, float epsilon, uint32_t dtype, uint32_t mode)
{
    g001::TilingData cfg;
    cfg.rows = rows;
    cfg.dim = dim;
    cfg.epsilon = epsilon;
    cfg.dtype = dtype;
    cfg.mode = mode;
    // dtype: 0=FP32, 1=FP16, 2=BF16 (judge contract).
    if (cfg.dtype == 0) {
        if (cfg.mode == 0) {
            g001::RunResidentRows<float>(x, residual, gamma, bias, output, cfg);
        } else {
            g001::RunGenericRows<float>(x, residual, gamma, bias, output, cfg);
        }
    } else if (cfg.dtype == 1) {
        if (cfg.mode == 0) {
            g001::RunResidentRows<half>(x, residual, gamma, bias, output, cfg);
        } else {
            g001::RunGenericRows<half>(x, residual, gamma, bias, output, cfg);
        }
    } else if (cfg.dtype == 2) {
        if (cfg.mode == 0) {
            g001::RunResidentRows<bfloat16_t>(x, residual, gamma, bias, output, cfg);
        } else {
            g001::RunGenericRows<bfloat16_t>(x, residual, gamma, bias, output, cfg);
        }
    } else {
        g001::RunGenericRows<float>(x, residual, gamma, bias, output, cfg);
    }
}

#if !defined(G001_DEVICE_ONLY)
extern "C" void run_kernel(
    GM_ADDR x, const TensorGroupInfo &info_x,
    GM_ADDR residual, const TensorGroupInfo &info_residual,
    GM_ADDR gamma, const TensorGroupInfo &info_gamma,
    GM_ADDR bias, const TensorGroupInfo &info_bias,
    GM_ADDR output, const TensorGroupInfo &info_output,
    int64_t availableCoreNum, aclrtStream stream, float epsilon)
{
    if (info_x.numTensors < 1 || info_x.tensors == nullptr) {
        return;
    }
    const auto &info = info_x.tensors[0];
    if (info.shape == nullptr || info.numDims < 2 || info.numDims > 4) {
        return;
    }
    const int64_t dim = info.shape[info.numDims - 1];
    if (dim < 64 || dim > 32768) {
        return;
    }
    uint64_t rows = 1;
    for (int64_t axis = 0; axis + 1 < info.numDims; ++axis) {
        if (info.shape[axis] <= 0 || rows > INT32_MAX / static_cast<uint64_t>(info.shape[axis])) {
            return;
        }
        rows *= static_cast<uint64_t>(info.shape[axis]);
    }
    // Judge dtype: 0=FP32, 1=FP16, 2=BF16. Pass through unchanged.
    const uint32_t dtype = static_cast<uint32_t>(info.dtype);
    if (dtype > 2) {
        return;
    }
    // mode 0 = resident one-read hot path (FP32 u row fits in UB).
    // mode 1 = generic two-pass fallback when the FP32 row would not fit.
    const uint32_t mode = (static_cast<uint64_t>(dim) * 4ull <= 160ull * 1024ull) ? 0u : 1u;
    uint64_t blocks = availableCoreNum > 0 ? static_cast<uint64_t>(availableCoreNum) : 1;
    blocks = blocks > 40 ? 40 : blocks;
    blocks = blocks > rows ? rows : blocks;
    g001_add_rms_norm_bias<<<static_cast<uint32_t>(blocks), nullptr, stream>>>(
        x, residual, gamma, bias, output, static_cast<int32_t>(rows),
        static_cast<int32_t>(dim), epsilon, dtype, mode);
}
#endif
