#include "kernel_operator.h"

using namespace AscendC;

namespace c001 {

// One row-group owns one row at a time. Four vector cores make the partials;
// the first core performs the scalar merge before the second group barrier.
constexpr uint32_t kGroupSize = 4;
constexpr uint32_t kMaxGroups = 10;
constexpr uint32_t kBarrierBytesPerGroup = 2048;  // 4 cache-line slots.
constexpr uint32_t kPartialOffset = kBarrierBytesPerGroup;
constexpr uint32_t kRmsOffset = kPartialOffset + kGroupSize * 32;
constexpr uint32_t kGroupWorkspaceBytes = kRmsOffset + 32;

struct C001TilingData {
    uint64_t rows;
    uint64_t d;
    uint64_t group_count;
    uint64_t block_dim;
    float epsilon;
};

template <typename T>
__aicore__ inline float ToScalar(T value)
{
    if constexpr (IsSameType<T, bfloat16_t>::value) {
        return AscendC::ToFloat(value);
    } else {
        return static_cast<float>(value);
    }
}

template <typename T>
__aicore__ inline T FromScalar(float value)
{
    if constexpr (IsSameType<T, bfloat16_t>::value) {
        return AscendC::ToBfloat16(value);
    } else {
        return static_cast<T>(value);
    }
}

template <typename T>
class CooperativeRmsNormBias {
public:
    __aicore__ inline void Init(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias,
                                GM_ADDR output, GM_ADDR workspace, GM_ADDR tiling)
    {
        valid_ = workspace != nullptr;
        if (workspace == nullptr) {
            return;
        }
        x_ = reinterpret_cast<__gm__ const T *>(x);
        residual_ = reinterpret_cast<__gm__ const T *>(residual);
        gamma_ = reinterpret_cast<__gm__ const T *>(gamma);
        bias_ = reinterpret_cast<__gm__ const T *>(bias);
        output_ = reinterpret_cast<__gm__ T *>(output);
        SetSysWorkspaceForce(workspace);
        workspace_ = GetUserWorkspace(workspace);
        tiling_ = reinterpret_cast<__gm__ const C001TilingData *>(tiling);
    }

    __aicore__ inline void Process()
    {
        if (!valid_) {
            return;
        }
        const uint64_t rows = tiling_->rows;
        const uint64_t d = tiling_->d;
        const float epsilon = tiling_->epsilon;
        uint32_t groupCount = static_cast<uint32_t>(tiling_->group_count);
        if (groupCount == 0) {
            groupCount = rows < kMaxGroups ? static_cast<uint32_t>(rows) : kMaxGroups;
        }
        if (groupCount == 0 || groupCount > kMaxGroups) {
            GenericFallback(rows, d, epsilon);
            return;
        }

        const uint64_t block = static_cast<uint64_t>(GetBlockIdx());
        const uint32_t group = static_cast<uint32_t>(block / kGroupSize);
        const uint32_t lane = static_cast<uint32_t>(block % kGroupSize);
        if (group >= groupCount) {
            return;
        }

        // Aligned D is the steady-state path. The scalar path below remains
        // the private complete fallback for D tails and unusual launch data.
        if (d >= 64 && d <= 32768 && (d % 32) == 0) {
            HotPath(rows, d, epsilon, groupCount, group, lane);
        } else {
            GenericFallback(rows, d, epsilon);
        }
    }

private:
    __aicore__ inline void HotPath(uint64_t rows, uint64_t d, float epsilon,
                                   uint32_t groupCount, uint32_t group, uint32_t lane)
    {
        TPipe pipe;
        GM_ADDR groupWorkspace = reinterpret_cast<GM_ADDR>(
            reinterpret_cast<__gm__ uint8_t *>(workspace_) + group * kGroupWorkspaceBytes);
        GroupBarrier<PipeMode::MTE3_MODE> barrier(groupWorkspace, kGroupSize, kGroupSize);

        const uint64_t first = (d * lane) / kGroupSize;
        const uint64_t last = (d * (lane + 1)) / kGroupSize;
        const int32_t dInt = static_cast<int32_t>(d);
        const float dFloat = static_cast<float>(dInt);
        __gm__ uint8_t *groupBytes = reinterpret_cast<__gm__ uint8_t *>(groupWorkspace);
        __gm__ volatile float *partial = reinterpret_cast<__gm__ volatile float *>(
            groupBytes + kPartialOffset + lane * 32);
        __gm__ volatile float *rms = reinterpret_cast<__gm__ volatile float *>(groupBytes + kRmsOffset);

        for (uint64_t row = group; row < rows; row += groupCount) {
            const uint64_t rowOffset = row * d;
            float partialSum = 0.0f;
            for (uint64_t col = first; col < last; ++col) {
                const uint64_t index = rowOffset + col;
                const float u = ToScalar(x_[index]) + ToScalar(residual_[index]);
                partialSum += u * u;
            }
            *partial = partialSum;

            barrier.Arrive(lane);
            barrier.Wait(lane);

            if (lane == 0) {
                float sum = 0.0f;
                for (uint32_t i = 0; i < kGroupSize; ++i) {
                    const __gm__ volatile float *peer = reinterpret_cast<__gm__ volatile float *>(
                        groupBytes + kPartialOffset + i * 32);
                    sum += *peer;
                }
                *rms = 1.0f / sqrt(sum / dFloat + epsilon);
            }

            barrier.Arrive(lane);
            barrier.Wait(lane);

            const float invRms = *rms;
            for (uint64_t col = first; col < last; ++col) {
                const uint64_t index = rowOffset + col;
                const float u = ToScalar(x_[index]) + ToScalar(residual_[index]);
                const float value = u * invRms * ToScalar(gamma_[col]) + ToScalar(bias_[col]);
                output_[index] = FromScalar<T>(value);
            }
        }
    }

    __aicore__ inline void GenericFallback(uint64_t rows, uint64_t d, float epsilon)
    {
        // Only block zero enters this path. It has no barrier, so a launch
        // with extra blocks cannot strand a core when the row count is small.
        if (GetBlockIdx() != 0) {
            return;
        }
        for (uint64_t row = 0; row < rows; ++row) {
            const uint64_t rowOffset = row * d;
            float sum = 0.0f;
            for (uint64_t col = 0; col < d; ++col) {
                const uint64_t index = rowOffset + col;
                const float u = ToScalar(x_[index]) + ToScalar(residual_[index]);
                sum += u * u;
            }
            int32_t dInt = static_cast<int32_t>(d);
            float dFloat = static_cast<float>(dInt);
            float invRms = 1.0f / sqrt(sum / dFloat + epsilon);
            for (uint64_t col = 0; col < d; ++col) {
                const uint64_t index = rowOffset + col;
                const float u = ToScalar(x_[index]) + ToScalar(residual_[index]);
                const float value = u * invRms * ToScalar(gamma_[col]) + ToScalar(bias_[col]);
                output_[index] = FromScalar<T>(value);
            }
        }
    }

    __gm__ const T *x_;
    __gm__ const T *residual_;
    __gm__ const T *gamma_;
    __gm__ const T *bias_;
    __gm__ T *output_;
    GM_ADDR workspace_;
    __gm__ const C001TilingData *tiling_;
    bool valid_;
};

template <typename T>
__aicore__ inline void Run(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias,
                           GM_ADDR output, GM_ADDR workspace, GM_ADDR tiling)
{
    CooperativeRmsNormBias<T> op;
    op.Init(x, residual, gamma, bias, output, workspace, tiling);
    op.Process();
}

}  // namespace c001

extern "C" __global__ __aicore__ void c001_add_rms_norm_bias_fp16(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    GM_ADDR workspace, GM_ADDR tiling)
{
    KERNEL_TASK_TYPE_DEFAULT(KERNEL_TYPE_AIV_ONLY);
    c001::Run<half>(x, residual, gamma, bias, output, workspace, tiling);
}

extern "C" __global__ __aicore__ void c001_add_rms_norm_bias_bf16(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    GM_ADDR workspace, GM_ADDR tiling)
{
    KERNEL_TASK_TYPE_DEFAULT(KERNEL_TYPE_AIV_ONLY);
    c001::Run<bfloat16_t>(x, residual, gamma, bias, output, workspace, tiling);
}

extern "C" __global__ __aicore__ void c001_add_rms_norm_bias_fp32(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    GM_ADDR workspace, GM_ADDR tiling)
{
    KERNEL_TASK_TYPE_DEFAULT(KERNEL_TYPE_AIV_ONLY);
    c001::Run<float>(x, residual, gamma, bias, output, workspace, tiling);
}
