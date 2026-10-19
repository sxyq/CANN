#include "kernel_operator.h"

using namespace AscendC;

namespace {

constexpr uint32_t kBytesPerBlock = 32;
constexpr uint32_t kHotD = 1024;
constexpr uint32_t kHotRows = 8;
constexpr uint32_t kFallbackChunkD = 1024;

__aicore__ inline uint32_t AlignUp(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) / alignment * alignment;
}

template <typename T>
__aicore__ inline float ToFloat(T value)
{
    return static_cast<float>(value);
}

template <typename T>
__aicore__ inline T FromFloat(float value)
{
    return static_cast<T>(value);
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
        colsPad_ = AlignUp(cols_ * sizeof(T), kBytesPerBlock) / sizeof(T);
        hot_ = cols_ <= kHotD;
        if (hot_) {
            pipe_.InitBuffer(xBuf_, kHotRows * colsPad_ * sizeof(T));
            pipe_.InitBuffer(residualBuf_, kHotRows * colsPad_ * sizeof(T));
            pipe_.InitBuffer(outputBuf_, kHotRows * colsPad_ * sizeof(T));
            pipe_.InitBuffer(gammaBuf_, colsPad_ * sizeof(T));
            pipe_.InitBuffer(biasBuf_, colsPad_ * sizeof(T));
        } else {
            pipe_.InitBuffer(xBuf_, kFallbackChunkD * sizeof(T));
            pipe_.InitBuffer(residualBuf_, kFallbackChunkD * sizeof(T));
            pipe_.InitBuffer(outputBuf_, kFallbackChunkD * sizeof(T));
            pipe_.InitBuffer(gammaBuf_, kFallbackChunkD * sizeof(T));
            pipe_.InitBuffer(biasBuf_, kFallbackChunkD * sizeof(T));
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
        if (rowCount > 1 && rowBytes % kBytesPerBlock == 0) {
            DataCopyExtParams copyParams(static_cast<uint16_t>(rowCount), rowBytes, 0, 0, 0);
            DataCopyPadExtParams<T> padParams{false, 0, 0, 0};
            DataCopyPad(dst, src[rowBegin * rowStride], copyParams, padParams);
            return;
        }
        for (uint32_t row = 0; row < rowCount; ++row) {
            CopyRow(dst[row * colsPad_], src[(rowBegin + row) * rowStride], rowBytes);
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
        if (rowCount > 1 && rowBytes % kBytesPerBlock == 0) {
            DataCopyExtParams copyParams(static_cast<uint16_t>(rowCount), rowBytes, 0, 0, 0);
            DataCopyPad(dst[rowBegin * rowStride], src, copyParams);
            return;
        }
        for (uint32_t row = 0; row < rowCount; ++row) {
            DataCopyPad(dst[(rowBegin + row) * rowStride], src[row * colsPad_],
                DataCopyExtParams(1, rowBytes, 0, 0, 0));
        }
    }

    __aicore__ inline void LoadParameters(LocalTensor<T> gamma, LocalTensor<T> bias, uint32_t count,
        uint32_t countPad)
    {
        const uint32_t bytes = count * sizeof(T);
        DataCopyExtParams copyParams(1, bytes, 0, 0, 0);
        DataCopyPadExtParams<T> padParams{true, 0, 0, 0};
        DataCopyPad(gamma, gammaGm_, copyParams, padParams);
        DataCopyPad(bias, biasGm_, copyParams, padParams);
        (void)countPad;
    }

    __aicore__ inline void ProcessHot(uint32_t rowBegin, uint32_t rowEnd)
    {
        LocalTensor<T> x = xBuf_.Get<T>();
        LocalTensor<T> residual = residualBuf_.Get<T>();
        LocalTensor<T> output = outputBuf_.Get<T>();
        LocalTensor<T> gamma = gammaBuf_.Get<T>();
        LocalTensor<T> bias = biasBuf_.Get<T>();
        LoadParameters(gamma, bias, cols_, colsPad_);
        const uint32_t rowBytes = cols_ * sizeof(T);
        for (uint32_t tileBegin = rowBegin; tileBegin < rowEnd; tileBegin += kHotRows) {
            const uint32_t tileRows = (rowEnd - tileBegin) < kHotRows ? rowEnd - tileBegin : kHotRows;
            CopyRows(x, xGm_, tileBegin, tileRows, cols_, rowBytes);
            CopyRows(residual, residualGm_, tileBegin, tileRows, cols_, rowBytes);
            for (uint32_t row = 0; row < tileRows; ++row) {
                LocalTensor<T> xRow = x[row * colsPad_];
                LocalTensor<T> residualRow = residual[row * colsPad_];
                LocalTensor<T> outputRow = output[row * colsPad_];
                float sum = 0.0f;
                for (uint32_t col = 0; col < cols_; ++col) {
                    const float u = ToFloat(xRow.GetValue(col)) + ToFloat(residualRow.GetValue(col));
                    sum += u * u;
                }
                float invRms = 1.0f / __builtin_sqrtf(sum * colsInv_ + epsilon_);
                for (uint32_t col = 0; col < cols_; ++col) {
                    const float u = ToFloat(xRow.GetValue(col)) + ToFloat(residualRow.GetValue(col));
                    const float y = u * invRms * ToFloat(gamma.GetValue(col)) + ToFloat(bias.GetValue(col));
                    outputRow.SetValue(col, FromFloat<T>(y));
                }
            }
            StoreRows(output, outputGm_, tileBegin, tileRows, cols_, rowBytes);
        }
    }

    __aicore__ inline void ProcessWide(uint32_t rowBegin, uint32_t rowEnd)
    {
        LocalTensor<T> x = xBuf_.Get<T>();
        LocalTensor<T> residual = residualBuf_.Get<T>();
        LocalTensor<T> output = outputBuf_.Get<T>();
        LocalTensor<T> gamma = gammaBuf_.Get<T>();
        LocalTensor<T> bias = biasBuf_.Get<T>();
        for (uint32_t row = rowBegin; row < rowEnd; ++row) {
            float sum = 0.0f;
            for (uint32_t colBegin = 0; colBegin < cols_; colBegin += kFallbackChunkD) {
                const uint32_t count = (cols_ - colBegin) < kFallbackChunkD ? cols_ - colBegin : kFallbackChunkD;
                const uint32_t rowBytes = count * sizeof(T);
                CopyRow(x, xGm_[row * cols_ + colBegin], rowBytes);
                CopyRow(residual, residualGm_[row * cols_ + colBegin], rowBytes);
                for (uint32_t col = 0; col < count; ++col) {
                    const float u = ToFloat(x.GetValue(col)) + ToFloat(residual.GetValue(col));
                    sum += u * u;
                }
            }
            float invRms = 1.0f / __builtin_sqrtf(sum * colsInv_ + epsilon_);
            for (uint32_t colBegin = 0; colBegin < cols_; colBegin += kFallbackChunkD) {
                const uint32_t count = (cols_ - colBegin) < kFallbackChunkD ? cols_ - colBegin : kFallbackChunkD;
                const uint32_t rowBytes = count * sizeof(T);
                CopyRow(x, xGm_[row * cols_ + colBegin], rowBytes);
                CopyRow(residual, residualGm_[row * cols_ + colBegin], rowBytes);
                CopyRow(gamma, gammaGm_[colBegin], rowBytes);
                CopyRow(bias, biasGm_[colBegin], rowBytes);
                for (uint32_t col = 0; col < count; ++col) {
                    const float u = ToFloat(x.GetValue(col)) + ToFloat(residual.GetValue(col));
                    const float y = u * invRms * ToFloat(gamma.GetValue(col)) + ToFloat(bias.GetValue(col));
                    output.SetValue(col, FromFloat<T>(y));
                }
                DataCopyPad(outputGm_[row * cols_ + colBegin], output,
                    DataCopyExtParams(1, rowBytes, 0, 0, 0));
            }
        }
    }

    TPipe pipe_;
    TBuf<TPosition::VECCALC> xBuf_;
    TBuf<TPosition::VECCALC> residualBuf_;
    TBuf<TPosition::VECCALC> outputBuf_;
    TBuf<TPosition::VECCALC> gammaBuf_;
    TBuf<TPosition::VECCALC> biasBuf_;
    GlobalTensor<T> xGm_;
    GlobalTensor<T> residualGm_;
    GlobalTensor<T> gammaGm_;
    GlobalTensor<T> biasGm_;
    GlobalTensor<T> outputGm_;
    uint32_t rows_ = 0;
    uint32_t cols_ = 0;
    uint32_t colsPad_ = 0;
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

extern "C" __global__ __aicore__ void add_rms_norm_bias_fp16(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma,
    GM_ADDR bias, GM_ADDR output, uint32_t rows, uint32_t cols, float epsilon, float colsInv)
{
    RunAddRmsNormBias<half>(x, residual, gamma, bias, output, rows, cols, epsilon, colsInv);
}

extern "C" __global__ __aicore__ void add_rms_norm_bias_bf16(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma,
    GM_ADDR bias, GM_ADDR output, uint32_t rows, uint32_t cols, float epsilon, float colsInv)
{
    RunAddRmsNormBias<bfloat16_t>(x, residual, gamma, bias, output, rows, cols, epsilon, colsInv);
}

extern "C" __global__ __aicore__ void add_rms_norm_bias_fp32(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma,
    GM_ADDR bias, GM_ADDR output, uint32_t rows, uint32_t cols, float epsilon, float colsInv)
{
    RunAddRmsNormBias<float>(x, residual, gamma, bias, output, rows, cols, epsilon, colsInv);
}
