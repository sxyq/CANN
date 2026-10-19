#include "ascendc/basic_api/kernel_operator.h"

namespace d001 {

using AscendC::GetBlockIdx;

struct D001TilingData {
    uint32_t rows;
    uint32_t d;
    uint32_t usedCores;
    uint32_t dtype;
    uint32_t rowBatch;
    uint32_t mode;
    float epsilon;
    float invD;
};

constexpr uint32_t kStripeTile = 128;
constexpr uint32_t kMaxRowsPerBatch = 32;

template <typename T>
__aicore__ inline float ToFloat(T value)
{
    return static_cast<float>(value);
}

template <>
__aicore__ inline float ToFloat<uint16_t>(uint16_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value) << 16;
    return *reinterpret_cast<const float*>(&bits);
}

template <typename T>
__aicore__ inline T FromFloat(float value)
{
    return static_cast<T>(value);
}

template <>
__aicore__ inline uint16_t FromFloat<uint16_t>(float value)
{
    const uint32_t bits = *reinterpret_cast<const uint32_t*>(&value);
    return static_cast<uint16_t>(bits >> 16);
}

template <typename T>
__aicore__ inline void RunPersistentStripe(__gm__ const T* x,
                                           __gm__ const T* residual,
                                           __gm__ const T* gamma,
                                           __gm__ const T* bias,
                                           __gm__ T* output,
                                           __gm__ float* workspacePtr,
                                           const D001TilingData& tiling)
{
    const uint32_t block = static_cast<uint32_t>(GetBlockIdx());
    const uint32_t used = tiling.usedCores == 0 ? 1 : tiling.usedCores;
    if (block >= used) {
        return;
    }

    // A block owns one fixed D stripe and keeps reusing it over all row batches.
    const uint32_t stripeWidth = (tiling.d + used - 1) / used;
    const uint32_t stripeBegin = block * stripeWidth;
    const uint32_t stripeEnd = stripeBegin + stripeWidth < tiling.d ? stripeBegin + stripeWidth : tiling.d;
    const uint32_t rowBatch = tiling.rowBatch == 0 || tiling.rowBatch > kMaxRowsPerBatch
        ? kMaxRowsPerBatch : tiling.rowBatch;
    const uint32_t workspaceStride = rowBatch * used + rowBatch;
    for (uint32_t batchBegin = 0, batchIndex = 0; batchBegin < tiling.rows;
         batchBegin += rowBatch, ++batchIndex) {
        const uint32_t batchRows = batchBegin + rowBatch < tiling.rows ? rowBatch : tiling.rows - batchBegin;
        const uint32_t slot = batchIndex & 1;
        __gm__ float* partial = workspacePtr + slot * workspaceStride;
        __gm__ float* merged = partial + rowBatch * used;
        float localSums[kMaxRowsPerBatch] = {};

        for (uint32_t tileBegin = stripeBegin; tileBegin < stripeEnd; tileBegin += kStripeTile) {
            const uint32_t tileEnd = tileBegin + kStripeTile < stripeEnd ? tileBegin + kStripeTile : stripeEnd;
            for (uint32_t localRow = 0; localRow < batchRows; ++localRow) {
                const uint64_t rowBase = static_cast<uint64_t>(batchBegin + localRow) * tiling.d;
                for (uint32_t col = tileBegin; col < tileEnd; ++col) {
                    const float u = ToFloat(x[rowBase + col]) + ToFloat(residual[rowBase + col]);
                    localSums[localRow] += u * u;
                }
            }
        }

        for (uint32_t localRow = 0; localRow < batchRows; ++localRow) {
            partial[localRow * used + block] = localSums[localRow];
        }
        AscendC::SyncAll<true>();

        // Core zero is the deterministic cross-stripe row-scalar merge owner.
        if (block == 0) {
            for (uint32_t localRow = 0; localRow < batchRows; ++localRow) {
                float total = 0.0f;
                for (uint32_t stripe = 0; stripe < used; ++stripe) {
                    total += partial[localRow * used + stripe];
                }
                merged[localRow] = total;
            }
        }
        AscendC::SyncAll<true>();

        for (uint32_t tileBegin = stripeBegin; tileBegin < stripeEnd; tileBegin += kStripeTile) {
            const uint32_t tileEnd = tileBegin + kStripeTile < stripeEnd ? tileBegin + kStripeTile : stripeEnd;
            float gammaTile[kStripeTile] = {};
            float biasTile[kStripeTile] = {};
            for (uint32_t col = tileBegin; col < tileEnd; ++col) {
                gammaTile[col - tileBegin] = ToFloat(gamma[col]);
                biasTile[col - tileBegin] = ToFloat(bias[col]);
            }
            for (uint32_t localRow = 0; localRow < batchRows; ++localRow) {
                const uint64_t rowBase = static_cast<uint64_t>(batchBegin + localRow) * tiling.d;
                const float invRms = 1.0f / __builtin_sqrtf(merged[localRow] * tiling.invD + tiling.epsilon);
                for (uint32_t col = tileBegin; col < tileEnd; ++col) {
                    const float u = ToFloat(x[rowBase + col]) + ToFloat(residual[rowBase + col]);
                    output[rowBase + col] = FromFloat<T>(u * invRms * gammaTile[col - tileBegin] +
                                                         biasTile[col - tileBegin]);
                }
            }
        }
    }
}

template <typename T>
__aicore__ inline void RunGenericFallback(__gm__ const T* x,
                                          __gm__ const T* residual,
                                          __gm__ const T* gamma,
                                          __gm__ const T* bias,
                                          __gm__ T* output,
                                          const D001TilingData& tiling)
{
    // The fallback retains complete scalar semantics and uses one block, with no synchronization.
    if (GetBlockIdx() != 0) {
        return;
    }
    for (uint32_t row = 0; row < tiling.rows; ++row) {
        const uint64_t rowBase = static_cast<uint64_t>(row) * tiling.d;
        float sumSquares = 0.0f;
        for (uint32_t col = 0; col < tiling.d; ++col) {
            const float u = ToFloat(x[rowBase + col]) + ToFloat(residual[rowBase + col]);
            sumSquares += u * u;
        }
        const float invRms = 1.0f / __builtin_sqrtf(sumSquares * tiling.invD + tiling.epsilon);
        for (uint32_t col = 0; col < tiling.d; ++col) {
            const float u = ToFloat(x[rowBase + col]) + ToFloat(residual[rowBase + col]);
            output[rowBase + col] = FromFloat<T>(u * invRms * ToFloat(gamma[col]) + ToFloat(bias[col]));
        }
    }
}

}  // namespace d001

extern "C" __global__ __aicore__ void d001_add_rms_norm_bias(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    GM_ADDR workspace, GM_ADDR tiling)
{
    const auto* configGm = reinterpret_cast<__gm__ const d001::D001TilingData*>(tiling);
    d001::D001TilingData config;
    config.rows = configGm->rows;
    config.d = configGm->d;
    config.usedCores = configGm->usedCores;
    config.dtype = configGm->dtype;
    config.rowBatch = configGm->rowBatch;
    config.mode = configGm->mode;
    config.epsilon = configGm->epsilon;
    config.invD = configGm->invD;
    if (config.dtype == 0) {
        if (config.mode == 0) {
            d001::RunPersistentStripe(reinterpret_cast<__gm__ const half*>(x),
                                      reinterpret_cast<__gm__ const half*>(residual),
                                      reinterpret_cast<__gm__ const half*>(gamma),
                                      reinterpret_cast<__gm__ const half*>(bias),
                                      reinterpret_cast<__gm__ half*>(output),
                                      reinterpret_cast<__gm__ float*>(workspace), config);
        } else {
            d001::RunGenericFallback(reinterpret_cast<__gm__ const half*>(x),
                                     reinterpret_cast<__gm__ const half*>(residual),
                                     reinterpret_cast<__gm__ const half*>(gamma),
                                     reinterpret_cast<__gm__ const half*>(bias),
                                     reinterpret_cast<__gm__ half*>(output), config);
        }
    } else if (config.dtype == 1) {
        if (config.mode == 0) {
            d001::RunPersistentStripe(reinterpret_cast<__gm__ const uint16_t*>(x),
                                      reinterpret_cast<__gm__ const uint16_t*>(residual),
                                      reinterpret_cast<__gm__ const uint16_t*>(gamma),
                                      reinterpret_cast<__gm__ const uint16_t*>(bias),
                                      reinterpret_cast<__gm__ uint16_t*>(output),
                                      reinterpret_cast<__gm__ float*>(workspace), config);
        } else {
            d001::RunGenericFallback(reinterpret_cast<__gm__ const uint16_t*>(x),
                                     reinterpret_cast<__gm__ const uint16_t*>(residual),
                                     reinterpret_cast<__gm__ const uint16_t*>(gamma),
                                     reinterpret_cast<__gm__ const uint16_t*>(bias),
                                     reinterpret_cast<__gm__ uint16_t*>(output), config);
        }
    } else {
        if (config.mode == 0) {
            d001::RunPersistentStripe(reinterpret_cast<__gm__ const float*>(x),
                                      reinterpret_cast<__gm__ const float*>(residual),
                                      reinterpret_cast<__gm__ const float*>(gamma),
                                      reinterpret_cast<__gm__ const float*>(bias),
                                      reinterpret_cast<__gm__ float*>(output),
                                      reinterpret_cast<__gm__ float*>(workspace), config);
        } else {
            d001::RunGenericFallback(reinterpret_cast<__gm__ const float*>(x),
                                     reinterpret_cast<__gm__ const float*>(residual),
                                     reinterpret_cast<__gm__ const float*>(gamma),
                                     reinterpret_cast<__gm__ const float*>(bias),
                                     reinterpret_cast<__gm__ float*>(output), config);
        }
    }
}
