#include <type_traits>

#include "kernel_operator.h"

#if !defined(H001_DEVICE_ONLY)
// Judge provides TensorInfo / TensorGroupInfo / aclrtStream before this file.
#endif

using namespace AscendC;

namespace {

constexpr uint32_t kBytesPerBlock = 32;
constexpr uint32_t kHotD = 1024;
constexpr uint32_t kMaxHotRows = 64;
constexpr uint32_t kHotBudgetBytes = 96 * 1024;
constexpr uint32_t kFallbackChunkD = 1024;
constexpr uint32_t kMaxBlockCount = 4095;

__aicore__ inline uint32_t AlignUp(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) / alignment * alignment;
}

// BF16 scalar path uses official AscendC::ToFloat / ToBfloat16 (bitcode/union),
// never static_cast forms that bisheng rejects as "not support bf16 type cast".
template <typename T>
__aicore__ inline float ToF32(T value)
{
    if constexpr (std::is_same_v<T, bfloat16_t>) {
        return AscendC::ToFloat(value);
    } else {
        return static_cast<float>(value);
    }
}

template <typename T>
__aicore__ inline T FromF32(float value)
{
    if constexpr (std::is_same_v<T, bfloat16_t>) {
        return AscendC::ToBfloat16(value);
    } else {
        return static_cast<T>(value);
    }
}

template <typename T>
class AddRmsNormBiasKernel {
public:
    __aicore__ inline void Init(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
        uint32_t rows, uint32_t cols, float epsilon, float colsInv)
    {
        xGm_.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(x));
        residualGm_.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(residual));
        gammaGm_.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(gamma));
        biasGm_.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(bias));
        outputGm_.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(output));
        rows_ = rows;
        cols_ = cols;
        epsilon_ = epsilon;
        colsInv_ = colsInv;
        colsPad_ = AlignUp(cols_ * static_cast<uint32_t>(sizeof(T)), kBytesPerBlock) / static_cast<uint32_t>(sizeof(T));
        rowBytes_ = cols_ * static_cast<uint32_t>(sizeof(T));
        rowPadBytes_ = colsPad_ * static_cast<uint32_t>(sizeof(T));
        hot_ = cols_ <= kHotD;
        if (hot_) {
            uint32_t maxRows = kHotBudgetBytes / (3u * rowPadBytes_);
            if (maxRows < 1) {
                maxRows = 1;
            }
            if (maxRows > kMaxHotRows) {
                maxRows = kMaxHotRows;
            }
            if (maxRows > kMaxBlockCount) {
                maxRows = kMaxBlockCount;
            }
            hotRows_ = maxRows;
            pipe_.InitBuffer(xBuf_, hotRows_ * rowPadBytes_);
            pipe_.InitBuffer(residualBuf_, hotRows_ * rowPadBytes_);
            pipe_.InitBuffer(outputBuf_, hotRows_ * rowPadBytes_);
            pipe_.InitBuffer(gammaBuf_, colsPad_ * sizeof(T));
            pipe_.InitBuffer(biasBuf_, colsPad_ * sizeof(T));
            pipe_.InitBuffer(gammaF32Buf_, colsPad_ * sizeof(float));
            pipe_.InitBuffer(biasF32Buf_, colsPad_ * sizeof(float));
            pipe_.InitBuffer(workABuf_, colsPad_ * sizeof(float));
            pipe_.InitBuffer(workBBuf_, colsPad_ * sizeof(float));
            pipe_.InitBuffer(workCBuf_, colsPad_ * sizeof(float));
        } else {
            hotRows_ = 1;
            pipe_.InitBuffer(xBuf_, kFallbackChunkD * sizeof(T));
            pipe_.InitBuffer(residualBuf_, kFallbackChunkD * sizeof(T));
            pipe_.InitBuffer(outputBuf_, kFallbackChunkD * sizeof(T));
            pipe_.InitBuffer(gammaBuf_, kFallbackChunkD * sizeof(T));
            pipe_.InitBuffer(biasBuf_, kFallbackChunkD * sizeof(T));
            pipe_.InitBuffer(gammaF32Buf_, kFallbackChunkD * sizeof(float));
            pipe_.InitBuffer(biasF32Buf_, kFallbackChunkD * sizeof(float));
            pipe_.InitBuffer(workABuf_, kFallbackChunkD * sizeof(float));
            pipe_.InitBuffer(workBBuf_, kFallbackChunkD * sizeof(float));
            pipe_.InitBuffer(workCBuf_, kFallbackChunkD * sizeof(float));
        }
    }

    __aicore__ inline void Process()
    {
        const uint32_t core = GetBlockIdx();
        const uint32_t coreCount = GetBlockNum();
        const uint32_t rowsPerCore = (rows_ + coreCount - 1) / coreCount;
        const uint32_t rowBegin = core * rowsPerCore;
        const uint32_t rowEnd = rowBegin + rowsPerCore < rows_ ? rowBegin + rowsPerCore : rows_;
        if (rowBegin >= rowEnd) {
            return;
        }
        if (hot_) {
            ProcessHot(rowBegin, rowEnd);
        } else {
            ProcessWide(rowBegin, rowEnd);
        }
    }

private:
    __aicore__ inline void CopyRows(LocalTensor<T> dst, GlobalTensor<T> src, uint32_t rowBegin, uint32_t rowCount,
        uint32_t rowStride, uint32_t rowBytes)
    {
        if (rowCount > 1 && rowBytes % kBytesPerBlock == 0 && rowPadBytes_ == rowBytes) {
            DataCopyExtParams copyParams(static_cast<uint16_t>(rowCount), rowBytes, 0, 0, 0);
            DataCopyPadExtParams<T> padParams{false, 0, 0, 0};
            DataCopyPad(dst, src[static_cast<uint64_t>(rowBegin) * rowStride], copyParams, padParams);
            return;
        }
        for (uint32_t row = 0; row < rowCount; ++row) {
            CopyRow(dst[row * colsPad_], src[(static_cast<uint64_t>(rowBegin) + row) * rowStride], rowBytes);
        }
    }

    __aicore__ inline void CopyRow(LocalTensor<T> dst, GlobalTensor<T> src, uint32_t rowBytes)
    {
        DataCopyExtParams copyParams(1, rowBytes, 0, 0, 0);
        DataCopyPadExtParams<T> padParams{true, 0, 0, 0};
        DataCopyPad(dst, src, copyParams, padParams);
    }

    __aicore__ inline void StoreRows(LocalTensor<T> src, GlobalTensor<T> dst, uint32_t rowBegin, uint32_t rowCount,
        uint32_t rowStride, uint32_t rowBytes)
    {
        if (rowCount > 1 && rowBytes % kBytesPerBlock == 0 && rowPadBytes_ == rowBytes) {
            DataCopyExtParams copyParams(static_cast<uint16_t>(rowCount), rowBytes, 0, 0, 0);
            DataCopyPad(dst[static_cast<uint64_t>(rowBegin) * rowStride], src, copyParams);
            return;
        }
        for (uint32_t row = 0; row < rowCount; ++row) {
            DataCopyPad(dst[(static_cast<uint64_t>(rowBegin) + row) * rowStride], src[row * colsPad_],
                DataCopyExtParams(1, rowBytes, 0, 0, 0));
        }
    }

    __aicore__ inline void LoadParametersResident()
    {
        LocalTensor<T> gamma = gammaBuf_.Get<T>();
        LocalTensor<T> bias = biasBuf_.Get<T>();
        CopyRow(gamma, gammaGm_, rowBytes_);
        CopyRow(bias, biasGm_, rowBytes_);
        PipeBarrier<PIPE_MTE2>();
        LocalTensor<float> gammaF32 = gammaF32Buf_.Get<float>();
        LocalTensor<float> biasF32 = biasF32Buf_.Get<float>();
        ToF32Vec(gammaF32, gamma, cols_);
        ToF32Vec(biasF32, bias, cols_);
        PipeBarrier<PIPE_V>();
    }

    __aicore__ inline void ToF32Vec(LocalTensor<float> dst, LocalTensor<T> src, uint32_t count)
    {
        if constexpr (std::is_same_v<T, float>) {
            Adds(dst, src, 0.0f, count);
        } else {
            Cast<float, T>(dst, src, RoundMode::CAST_NONE, count);
        }
    }

    __aicore__ inline void FromF32Vec(LocalTensor<T> dst, LocalTensor<float> src, uint32_t count)
    {
        if constexpr (std::is_same_v<T, float>) {
            Adds(dst, src, 0.0f, count);
        } else {
            Cast<T, float>(dst, src, RoundMode::CAST_ROUND, count);
        }
    }

    __aicore__ inline void ApplyRow(LocalTensor<T> outputRow, LocalTensor<T> xRow, LocalTensor<T> residualRow,
        LocalTensor<float> gammaF32, LocalTensor<float> biasF32, uint32_t count)
    {
        LocalTensor<float> x32 = workABuf_.Get<float>();
        LocalTensor<float> r32 = workBBuf_.Get<float>();
        LocalTensor<float> u32 = workCBuf_.Get<float>();
        ToF32Vec(x32, xRow, count);
        ToF32Vec(r32, residualRow, count);
        Add(u32, x32, r32, count);
        float sum = 0.0f;
        for (uint32_t col = 0; col < count; ++col) {
            const float u = u32.GetValue(col);
            sum += u * u;
        }
        const float invRms = 1.0f / __builtin_sqrtf(sum * colsInv_ + epsilon_);
        Muls(u32, u32, invRms, count);
        Mul(u32, u32, gammaF32, count);
        Add(u32, u32, biasF32, count);
        FromF32Vec(outputRow, u32, count);
    }

    __aicore__ inline void ProcessHot(uint32_t rowBegin, uint32_t rowEnd)
    {
        LocalTensor<T> x = xBuf_.Get<T>();
        LocalTensor<T> residual = residualBuf_.Get<T>();
        LocalTensor<T> output = outputBuf_.Get<T>();
        LoadParametersResident();
        LocalTensor<float> gammaF32 = gammaF32Buf_.Get<float>();
        LocalTensor<float> biasF32 = biasF32Buf_.Get<float>();
        for (uint32_t tileBegin = rowBegin; tileBegin < rowEnd; tileBegin += hotRows_) {
            uint32_t tileRows = rowEnd - tileBegin;
            if (tileRows > hotRows_) {
                tileRows = hotRows_;
            }
            CopyRows(x, xGm_, tileBegin, tileRows, cols_, rowBytes_);
            CopyRows(residual, residualGm_, tileBegin, tileRows, cols_, rowBytes_);
            PipeBarrier<PIPE_MTE2>();
            for (uint32_t row = 0; row < tileRows; ++row) {
                ApplyRow(output[row * colsPad_], x[row * colsPad_], residual[row * colsPad_], gammaF32, biasF32,
                    cols_);
            }
            PipeBarrier<PIPE_V>();
            StoreRows(output, outputGm_, tileBegin, tileRows, cols_, rowBytes_);
            PipeBarrier<PIPE_MTE3>();
        }
    }

    __aicore__ inline void ProcessWide(uint32_t rowBegin, uint32_t rowEnd)
    {
        LocalTensor<T> x = xBuf_.Get<T>();
        LocalTensor<T> residual = residualBuf_.Get<T>();
        LocalTensor<T> output = outputBuf_.Get<T>();
        LocalTensor<T> gamma = gammaBuf_.Get<T>();
        LocalTensor<T> bias = biasBuf_.Get<T>();
        LocalTensor<float> gammaF32 = gammaF32Buf_.Get<float>();
        LocalTensor<float> biasF32 = biasF32Buf_.Get<float>();
        for (uint32_t row = rowBegin; row < rowEnd; ++row) {
            for (uint32_t colBegin = 0; colBegin < cols_; colBegin += kFallbackChunkD) {
                const uint32_t count = (cols_ - colBegin) < kFallbackChunkD ? cols_ - colBegin : kFallbackChunkD;
                const uint32_t chunkBytes = count * static_cast<uint32_t>(sizeof(T));
                CopyRow(x, xGm_[static_cast<uint64_t>(row) * cols_ + colBegin], chunkBytes);
                CopyRow(residual, residualGm_[static_cast<uint64_t>(row) * cols_ + colBegin], chunkBytes);
                CopyRow(gamma, gammaGm_[colBegin], chunkBytes);
                CopyRow(bias, biasGm_[colBegin], chunkBytes);
                PipeBarrier<PIPE_MTE2>();
                ToF32Vec(gammaF32, gamma, count);
                ToF32Vec(biasF32, bias, count);
                PipeBarrier<PIPE_V>();
                ApplyRow(output, x, residual, gammaF32, biasF32, count);
                PipeBarrier<PIPE_V>();
                DataCopyPad(outputGm_[static_cast<uint64_t>(row) * cols_ + colBegin], output,
                    DataCopyExtParams(1, chunkBytes, 0, 0, 0));
                PipeBarrier<PIPE_MTE3>();
            }
        }
    }

    TPipe pipe_;
    TBuf<TPosition::VECCALC> xBuf_;
    TBuf<TPosition::VECCALC> residualBuf_;
    TBuf<TPosition::VECCALC> outputBuf_;
    TBuf<TPosition::VECCALC> gammaBuf_;
    TBuf<TPosition::VECCALC> biasBuf_;
    TBuf<TPosition::VECCALC> gammaF32Buf_;
    TBuf<TPosition::VECCALC> biasF32Buf_;
    TBuf<TPosition::VECCALC> workABuf_;
    TBuf<TPosition::VECCALC> workBBuf_;
    TBuf<TPosition::VECCALC> workCBuf_;
    GlobalTensor<T> xGm_;
    GlobalTensor<T> residualGm_;
    GlobalTensor<T> gammaGm_;
    GlobalTensor<T> biasGm_;
    GlobalTensor<T> outputGm_;
    uint32_t rows_ = 0;
    uint32_t cols_ = 0;
    uint32_t colsPad_ = 0;
    uint32_t rowBytes_ = 0;
    uint32_t rowPadBytes_ = 0;
    uint32_t hotRows_ = 1;
    float epsilon_ = 0.0f;
    float colsInv_ = 0.0f;
    bool hot_ = false;
};

template <typename T>
__aicore__ inline void RunAddRmsNormBias(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    uint32_t rows, uint32_t cols, float epsilon, float colsInv)
{
    AddRmsNormBiasKernel<T> op;
    op.Init(x, residual, gamma, bias, output, rows, cols, epsilon, colsInv);
    op.Process();
}

} // namespace

extern "C" __global__ __vector__ void add_rms_norm_bias(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    uint32_t rows, uint32_t cols, float epsilon, float colsInv, uint32_t dtype)
{
    if (dtype == 0) {
        RunAddRmsNormBias<float>(x, residual, gamma, bias, output, rows, cols, epsilon, colsInv);
    } else if (dtype == 1) {
        RunAddRmsNormBias<half>(x, residual, gamma, bias, output, rows, cols, epsilon, colsInv);
    } else {
        RunAddRmsNormBias<bfloat16_t>(x, residual, gamma, bias, output, rows, cols, epsilon, colsInv);
    }
}

#if !defined(H001_DEVICE_ONLY)
extern "C" void run_kernel(
    GM_ADDR x, const TensorGroupInfo &info_x,
    GM_ADDR residual, const TensorGroupInfo &info_residual,
    GM_ADDR gamma, const TensorGroupInfo &info_gamma,
    GM_ADDR bias, const TensorGroupInfo &info_bias,
    GM_ADDR output, const TensorGroupInfo &info_output,
    int64_t availableCoreNum, aclrtStream stream, float epsilon)
{
    (void)info_residual;
    (void)info_gamma;
    (void)info_bias;
    (void)info_output;
    if (info_x.numTensors < 1 || info_x.tensors == nullptr) {
        return;
    }
    const TensorInfo &info = info_x.tensors[0];
    if (info.shape == nullptr || info.numDims < 2 || info.numDims > 4) {
        return;
    }
    const int64_t dim = info.shape[info.numDims - 1];
    if (dim < 64 || dim > 32768) {
        return;
    }
    uint64_t rows = 1;
    for (int64_t axis = 0; axis + 1 < info.numDims; ++axis) {
        if (info.shape[axis] <= 0 || rows > (UINT64_MAX / static_cast<uint64_t>(info.shape[axis]))) {
            return;
        }
        rows *= static_cast<uint64_t>(info.shape[axis]);
    }
    if (rows == 0) {
        return;
    }
    uint32_t dtype;
    if (info.dtype == 0) {
        dtype = 0; // FP32
    } else if (info.dtype == 1) {
        dtype = 1; // FP16
    } else if (info.dtype == 2) {
        dtype = 2; // BF16
    } else {
        return;
    }
    uint64_t blocks = availableCoreNum > 0 ? static_cast<uint64_t>(availableCoreNum) : 1;
    if (blocks > 40) {
        blocks = 40;
    }
    if (blocks > rows) {
        blocks = rows;
    }
    const float colsInv = 1.0f / static_cast<float>(dim);
    add_rms_norm_bias<<<static_cast<uint32_t>(blocks), nullptr, stream>>>(
        x, residual, gamma, bias, output,
        static_cast<uint32_t>(rows), static_cast<uint32_t>(dim), epsilon, colsInv, dtype);
}
#endif
