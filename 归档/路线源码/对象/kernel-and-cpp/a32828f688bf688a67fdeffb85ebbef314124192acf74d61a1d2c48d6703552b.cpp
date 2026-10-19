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

// Explicit round-to-nearest-even float -> half.
// static_cast<half> rounding mode is not guaranteed; CAST_RINT (golden) is RNE.
template <>
__aicore__ inline half FromFloat<half>(float value)
{
    union Bits {
        uint32_t u;
        float f;
    } bitsValue;
    bitsValue.f = value;
    const uint32_t x = bitsValue.u;
    const uint16_t sign = static_cast<uint16_t>((x >> 16) & 0x8000u);
    const uint32_t absX = x & 0x7FFFFFFFu;

    if (absX >= 0x7F800000u) {
        if (absX == 0x7F800000u) {
            const uint16_t r = static_cast<uint16_t>(sign | 0x7C00u);
            return reinterpret_cast<const half &>(r);
        }
        const uint16_t r = static_cast<uint16_t>(sign | 0x7E00u | ((absX >> 13) & 0x3FFu));
        return reinterpret_cast<const half &>(r);
    }

    int32_t exp = static_cast<int32_t>(absX >> 23) - 127 + 15;
    const uint32_t mant = absX & 0x7FFFFFu;

    if (exp >= 31) {
        const uint16_t r = static_cast<uint16_t>(sign | 0x7C00u);
        return reinterpret_cast<const half &>(r);
    }

    if (exp <= 0) {
        if (exp < -10) {
            const uint16_t r = sign;
            return reinterpret_cast<const half &>(r);
        }
        const uint32_t fullMant = mant | ((absX >> 23) ? 0x800000u : 0u);
        const uint32_t rshift = static_cast<uint32_t>(14 - exp);
        const uint32_t halfMant = fullMant >> rshift;
        const uint32_t roundBit = (fullMant >> (rshift - 1)) & 1u;
        const uint32_t sticky = (rshift > 1) ? (fullMant & ((1u << (rshift - 1)) - 1u)) : 0u;
        uint32_t result = sign | (halfMant & 0x3FFu);
        if (roundBit && (sticky || (halfMant & 1u))) {
            result++;
        }
        const uint16_t r = static_cast<uint16_t>(result);
        return reinterpret_cast<const half &>(r);
    }

    const uint32_t halfMant = mant >> 13;
    const uint32_t roundBit = (mant >> 12) & 1u;
    const uint32_t sticky = mant & 0xFFFu;
    uint32_t result = sign | (static_cast<uint32_t>(exp) << 10) | halfMant;
    if (roundBit && (sticky || (halfMant & 1u))) {
        result++;
    }
    const uint16_t r = static_cast<uint16_t>(result);
    return reinterpret_cast<const half &>(r);
}

// Explicit round-to-nearest-even float -> bfloat16 (BF16 lives in bits 31:16).
template <>
__aicore__ inline bfloat16_t FromFloat<bfloat16_t>(float value)
{
    union Bits {
        uint32_t u;
        float f;
    } bitsValue;
    bitsValue.f = value;
    const uint32_t x = bitsValue.u;
    const uint16_t sign = static_cast<uint16_t>((x >> 16) & 0x8000u);
    const uint32_t absX = x & 0x7FFFFFFFu;

    if (absX >= 0x7F800000u) {
        if (absX == 0x7F800000u) {
            const uint16_t r = static_cast<uint16_t>(sign | 0x7F80u);
            return reinterpret_cast<const bfloat16_t &>(r);
        }
        const uint16_t r = static_cast<uint16_t>(sign | 0x7FC0u | ((absX >> 16) & 0x7Fu));
        return reinterpret_cast<const bfloat16_t &>(r);
    }

    const uint32_t halfMant = absX >> 16;
    const uint32_t truncated = absX & 0xFFFFu;
    uint32_t result = sign | halfMant;
    if (truncated > 0x8000u || (truncated == 0x8000u && (halfMant & 1u))) {
        result++;
    }
    const uint16_t r = static_cast<uint16_t>(result);
    return reinterpret_cast<const bfloat16_t &>(r);
}

__aicore__ inline float SqrtF(float x)
{
    return __builtin_cce_sqrtf(x);
}

// Hot path (Fresh One-Read / Resident-Y):
// each x/residual element is consumed once; FP32 u = x + residual stays
// resident in one UB row and is reused for RMS and for norm + bias.
// All arithmetic is FP32; one native cast at output (RNE).
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
        float sumSquares = 0.0f;

        for (int32_t i = 0; i < cfg.dim; ++i) {
            const float u = ToFloat(x.GetValue(rowBase + i)) + ToFloat(residual.GetValue(rowBase + i));
            resident.SetValue(i, u);
            sumSquares += u * u;
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
        float sumSquares = 0.0f;
        for (int32_t i = 0; i < cfg.dim; ++i) {
            const float u = ToFloat(x.GetValue(rowBase + i)) + ToFloat(residual.GetValue(rowBase + i));
            sumSquares += u * u;
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
