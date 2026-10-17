#include "kernel_operator.h"

namespace g001 {

// The launcher writes this compact record immediately before launching the kernel.
// dtype: 0 = fp16, 1 = bf16, 2 = fp32.
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
    const uint32_t rounding = 0x7FFFu + ((bitsValue.u >> 16) & 1u);
    bitsValue.u += rounding;
    return reinterpret_cast<const bfloat16_t &>(bitsValue.u);
}

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

    // One UB row holds u=x+residual. This is 128 KiB at the largest FP32 D,
    // leaving the remaining UB for runtime bookkeeping and alignment.
    AscendC::LocalMemAllocator<> allocator;
    AscendC::LocalTensor<T> resident = allocator.Alloc<T>(cfg.dim);

    const uint64_t core = static_cast<uint64_t>(AscendC::GetBlockIdx());
    const uint64_t cores = static_cast<uint64_t>(AscendC::GetBlockNum());
    for (uint64_t row = core; row < static_cast<uint64_t>(cfg.rows); row += cores) {
        const uint64_t rowBase = row * cfg.dim;
        float sumSquares = 0.0f;

        // Fresh one-read: each source element is consumed once while the
        // native-precision u value stays resident until y is written.
        for (int32_t i = 0; i < cfg.dim; ++i) {
            const float u = ToFloat(x.GetValue(rowBase + i)) + ToFloat(residual.GetValue(rowBase + i));
            const T nativeU = FromFloat<T>(u);
            resident.SetValue(i, nativeU);
            const float storedU = ToFloat(nativeU);
            sumSquares += storedU * storedU;
        }

        const float mean = sumSquares / static_cast<float>(cfg.dim);
        const float inverseRms = 1.0f / __builtin_sqrtf(mean + cfg.epsilon);
        for (int32_t i = 0; i < cfg.dim; ++i) {
            const float u = ToFloat(resident.GetValue(i));
            const float value = u * inverseRms * ToFloat(gamma.GetValue(i)) + ToFloat(bias.GetValue(i));
            output.SetValue(rowBase + i, FromFloat<T>(value));
        }
    }
}

template <typename T>
__aicore__ inline void RunGenericRows(GM_ADDR xAddr, GM_ADDR residualAddr, GM_ADDR gammaAddr, GM_ADDR biasAddr,
    GM_ADDR outputAddr, const TilingData &cfg)
{
    // The scalar path keeps the same arithmetic and row mapping for a fallback
    // dtype code without relying on the resident UB allocation.
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
        const uint64_t rowBase = row * cfg.dim;
        float sumSquares = 0.0f;
        for (int32_t i = 0; i < cfg.dim; ++i) {
            const float u = ToFloat(x.GetValue(rowBase + i)) + ToFloat(residual.GetValue(rowBase + i));
            sumSquares += u * u;
        }
        const float inverseRms = 1.0f / __builtin_sqrtf(sumSquares / static_cast<float>(cfg.dim) + cfg.epsilon);
        for (int32_t i = 0; i < cfg.dim; ++i) {
            const float u = ToFloat(x.GetValue(rowBase + i)) + ToFloat(residual.GetValue(rowBase + i));
            const float value = u * inverseRms * ToFloat(gamma.GetValue(i)) + ToFloat(bias.GetValue(i));
            output.SetValue(rowBase + i, FromFloat<T>(value));
        }
    }
}

} // namespace g001

extern "C" __global__ __aicore__ void g001_add_rms_norm_bias(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output, GM_ADDR workspace, GM_ADDR tiling)
{
    (void)workspace;
    const __gm__ g001::TilingData *cfgGm = reinterpret_cast<const __gm__ g001::TilingData *>(tiling);
    g001::TilingData cfg;
    cfg.rows = cfgGm->rows;
    cfg.dim = cfgGm->dim;
    cfg.epsilon = cfgGm->epsilon;
    cfg.dtype = cfgGm->dtype;
    cfg.mode = cfgGm->mode;
    if (cfg.dtype == 0) {
        if (cfg.mode == 0) {
            g001::RunResidentRows<half>(x, residual, gamma, bias, output, cfg);
        } else {
            g001::RunGenericRows<half>(x, residual, gamma, bias, output, cfg);
        }
    } else if (cfg.dtype == 1) {
        if (cfg.mode == 0) {
            g001::RunResidentRows<bfloat16_t>(x, residual, gamma, bias, output, cfg);
        } else {
            g001::RunGenericRows<bfloat16_t>(x, residual, gamma, bias, output, cfg);
        }
    } else if (cfg.dtype == 2) {
        if (cfg.mode == 0) {
            g001::RunResidentRows<float>(x, residual, gamma, bias, output, cfg);
        } else {
            g001::RunGenericRows<float>(x, residual, gamma, bias, output, cfg);
        }
    } else {
        g001::RunGenericRows<float>(x, residual, gamma, bias, output, cfg);
    }
}
