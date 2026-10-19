#include <cmath>
#include "kernel_operator.h"

// H001 Fresh Small-D / High-R. Judge template: npu_kernel_dev (kernel.asc).
// Do not redefine TensorInfo / TensorGroupInfo. No host-only std headers.

namespace {

constexpr uint32_t kBytesPerBlock = 32;
constexpr uint32_t kHotD = 1024;
constexpr uint32_t kMaxHotRows = 64;
constexpr uint32_t kHotBudgetBytes = 96 * 1024;
constexpr uint32_t kFallbackChunkD = 1024;
constexpr uint32_t kMaxBlockCount = 4095;
constexpr uint32_t kDtypeF32 = 0;
constexpr uint32_t kDtypeF16 = 1;
constexpr uint32_t kDtypeBf16 = 2;

__aicore__ inline uint32_t AlignUp(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) / alignment * alignment;
}

template <typename T>
struct H001Convert {
    __aicore__ inline static void ToF32(AscendC::LocalTensor<float> dst, AscendC::LocalTensor<T> src, uint32_t count)
    {
        AscendC::Cast<float, T>(dst, src, AscendC::RoundMode::CAST_NONE, static_cast<int32_t>(count));
    }
    __aicore__ inline static void FromF32(AscendC::LocalTensor<T> dst, AscendC::LocalTensor<float> src, uint32_t count)
    {
        AscendC::Cast<T, float>(dst, src, AscendC::RoundMode::CAST_ROUND, static_cast<int32_t>(count));
    }
};

template <>
struct H001Convert<float> {
    __aicore__ inline static void ToF32(AscendC::LocalTensor<float> dst, AscendC::LocalTensor<float> src, uint32_t count)
    {
        AscendC::Adds(dst, src, 0.0f, static_cast<int32_t>(count));
    }
    __aicore__ inline static void FromF32(AscendC::LocalTensor<float> dst, AscendC::LocalTensor<float> src, uint32_t count)
    {
        AscendC::Adds(dst, src, 0.0f, static_cast<int32_t>(count));
    }
};

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
        const uint32_t core = static_cast<uint32_t>(AscendC::GetBlockIdx());
        const uint32_t coreCount = static_cast<uint32_t>(AscendC::GetBlockNum());
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
    __aicore__ inline void MakeCopyParams(AscendC::DataCopyExtParams &copyParams, uint32_t blockCount,
        uint32_t blockLen, uint32_t srcStride, uint32_t dstStride)
    {
        copyParams.blockCount = static_cast<uint16_t>(blockCount);
        copyParams.blockLen = blockLen;
        copyParams.srcStride = srcStride;
        copyParams.dstStride = dstStride;
        copyParams.rsv = 0;
    }

    __aicore__ inline void MakePadParams(AscendC::DataCopyPadExtParams<T> &padParams, bool isPad, uint32_t rightPad)
    {
        padParams.isPad = isPad;
        padParams.leftPadding = 0;
        padParams.rightPadding = rightPad;
        padParams.paddingValue = static_cast<T>(0);
    }

    __aicore__ inline void CopyRows(AscendC::LocalTensor<T> dst, AscendC::GlobalTensor<T> src, uint32_t rowBegin,
        uint32_t rowCount, uint32_t rowStride, uint32_t rowBytes)
    {
        if (rowCount > 1 && rowBytes % kBytesPerBlock == 0 && rowPadBytes_ == rowBytes) {
            AscendC::DataCopyExtParams copyParams;
            MakeCopyParams(copyParams, rowCount, rowBytes, 0, 0);
            AscendC::DataCopyPadExtParams<T> padParams;
            MakePadParams(padParams, false, 0);
            AscendC::DataCopyPad(dst, src[static_cast<uint64_t>(rowBegin) * rowStride], copyParams, padParams);
            return;
        }
        for (uint32_t row = 0; row < rowCount; ++row) {
            CopyRow(dst[row * colsPad_], src[(static_cast<uint64_t>(rowBegin) + row) * rowStride], rowBytes);
        }
    }

    __aicore__ inline void CopyRow(AscendC::LocalTensor<T> dst, AscendC::GlobalTensor<T> src, uint32_t rowBytes)
    {
        AscendC::DataCopyExtParams copyParams;
        MakeCopyParams(copyParams, 1, rowBytes, 0, 0);
        AscendC::DataCopyPadExtParams<T> padParams;
        MakePadParams(padParams, true, 0);
        AscendC::DataCopyPad(dst, src, copyParams, padParams);
    }

    __aicore__ inline void StoreRows(AscendC::LocalTensor<T> src, AscendC::GlobalTensor<T> dst, uint32_t rowBegin,
        uint32_t rowCount, uint32_t rowStride, uint32_t rowBytes)
    {
        if (rowCount > 1 && rowBytes % kBytesPerBlock == 0 && rowPadBytes_ == rowBytes) {
            AscendC::DataCopyExtParams copyParams;
            MakeCopyParams(copyParams, rowCount, rowBytes, 0, 0);
            AscendC::DataCopyPad(dst[static_cast<uint64_t>(rowBegin) * rowStride], src, copyParams);
            return;
        }
        for (uint32_t row = 0; row < rowCount; ++row) {
            AscendC::DataCopyExtParams copyParams;
            MakeCopyParams(copyParams, 1, rowBytes, 0, 0);
            AscendC::DataCopyPad(dst[(static_cast<uint64_t>(rowBegin) + row) * rowStride], src[row * colsPad_],
                copyParams);
        }
    }

    __aicore__ inline void LoadParametersResident()
    {
        AscendC::LocalTensor<T> gamma = gammaBuf_.template Get<T>();
        AscendC::LocalTensor<T> bias = biasBuf_.template Get<T>();
        CopyRow(gamma, gammaGm_, rowBytes_);
        CopyRow(bias, biasGm_, rowBytes_);
        AscendC::PipeBarrier<PIPE_MTE2>();
        AscendC::LocalTensor<float> gammaF32 = gammaF32Buf_.template Get<float>();
        AscendC::LocalTensor<float> biasF32 = biasF32Buf_.template Get<float>();
        H001Convert<T>::ToF32(gammaF32, gamma, cols_);
        H001Convert<T>::ToF32(biasF32, bias, cols_);
        AscendC::PipeBarrier<PIPE_V>();
    }

    __aicore__ inline void ApplyRow(AscendC::LocalTensor<T> outputRow, AscendC::LocalTensor<T> xRow,
        AscendC::LocalTensor<T> residualRow, AscendC::LocalTensor<float> gammaF32,
        AscendC::LocalTensor<float> biasF32, uint32_t count)
    {
        AscendC::LocalTensor<float> x32 = workABuf_.template Get<float>();
        AscendC::LocalTensor<float> r32 = workBBuf_.template Get<float>();
        AscendC::LocalTensor<float> u32 = workCBuf_.template Get<float>();
        H001Convert<T>::ToF32(x32, xRow, count);
        H001Convert<T>::ToF32(r32, residualRow, count);
        AscendC::Add(u32, x32, r32, static_cast<int32_t>(count));
        float sum = 0.0f;
        for (uint32_t col = 0; col < count; ++col) {
            const float u = u32.GetValue(col);
            sum += u * u;
        }
        const float invRms = 1.0f / __builtin_sqrtf(sum * colsInv_ + epsilon_);
        AscendC::Muls(u32, u32, invRms, static_cast<int32_t>(count));
        AscendC::Mul(u32, u32, gammaF32, static_cast<int32_t>(count));
        AscendC::Add(u32, u32, biasF32, static_cast<int32_t>(count));
        H001Convert<T>::FromF32(outputRow, u32, count);
    }

    __aicore__ inline void ProcessHot(uint32_t rowBegin, uint32_t rowEnd)
    {
        AscendC::LocalTensor<T> x = xBuf_.template Get<T>();
        AscendC::LocalTensor<T> residual = residualBuf_.template Get<T>();
        AscendC::LocalTensor<T> output = outputBuf_.template Get<T>();
        LoadParametersResident();
        AscendC::LocalTensor<float> gammaF32 = gammaF32Buf_.template Get<float>();
        AscendC::LocalTensor<float> biasF32 = biasF32Buf_.template Get<float>();
        for (uint32_t tileBegin = rowBegin; tileBegin < rowEnd; tileBegin += hotRows_) {
            uint32_t tileRows = rowEnd - tileBegin;
            if (tileRows > hotRows_) {
                tileRows = hotRows_;
            }
            CopyRows(x, xGm_, tileBegin, tileRows, cols_, rowBytes_);
            CopyRows(residual, residualGm_, tileBegin, tileRows, cols_, rowBytes_);
            AscendC::PipeBarrier<PIPE_MTE2>();
            for (uint32_t row = 0; row < tileRows; ++row) {
                ApplyRow(output[row * colsPad_], x[row * colsPad_], residual[row * colsPad_], gammaF32, biasF32,
                    cols_);
            }
            AscendC::PipeBarrier<PIPE_V>();
            StoreRows(output, outputGm_, tileBegin, tileRows, cols_, rowBytes_);
            AscendC::PipeBarrier<PIPE_MTE3>();
        }
    }

    __aicore__ inline void ProcessWide(uint32_t rowBegin, uint32_t rowEnd)
    {
        AscendC::LocalTensor<T> x = xBuf_.template Get<T>();
        AscendC::LocalTensor<T> residual = residualBuf_.template Get<T>();
        AscendC::LocalTensor<T> output = outputBuf_.template Get<T>();
        AscendC::LocalTensor<T> gamma = gammaBuf_.template Get<T>();
        AscendC::LocalTensor<T> bias = biasBuf_.template Get<T>();
        AscendC::LocalTensor<float> gammaF32 = gammaF32Buf_.template Get<float>();
        AscendC::LocalTensor<float> biasF32 = biasF32Buf_.template Get<float>();
        for (uint32_t row = rowBegin; row < rowEnd; ++row) {
            for (uint32_t colBegin = 0; colBegin < cols_; colBegin += kFallbackChunkD) {
                const uint32_t count = (cols_ - colBegin) < kFallbackChunkD ? cols_ - colBegin : kFallbackChunkD;
                const uint32_t chunkBytes = count * static_cast<uint32_t>(sizeof(T));
                CopyRow(x, xGm_[static_cast<uint64_t>(row) * cols_ + colBegin], chunkBytes);
                CopyRow(residual, residualGm_[static_cast<uint64_t>(row) * cols_ + colBegin], chunkBytes);
                CopyRow(gamma, gammaGm_[colBegin], chunkBytes);
                CopyRow(bias, biasGm_[colBegin], chunkBytes);
                AscendC::PipeBarrier<PIPE_MTE2>();
                H001Convert<T>::ToF32(gammaF32, gamma, count);
                H001Convert<T>::ToF32(biasF32, bias, count);
                AscendC::PipeBarrier<PIPE_V>();
                ApplyRow(output, x, residual, gammaF32, biasF32, count);
                AscendC::PipeBarrier<PIPE_V>();
                AscendC::DataCopyExtParams copyParams;
                MakeCopyParams(copyParams, 1, chunkBytes, 0, 0);
                AscendC::DataCopyPad(outputGm_[static_cast<uint64_t>(row) * cols_ + colBegin], output, copyParams);
                AscendC::PipeBarrier<PIPE_MTE3>();
            }
        }
    }

    AscendC::TPipe pipe_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> xBuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> residualBuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> outputBuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> gammaBuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> biasBuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> gammaF32Buf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> biasF32Buf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> workABuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> workBBuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> workCBuf_;
    AscendC::GlobalTensor<T> xGm_;
    AscendC::GlobalTensor<T> residualGm_;
    AscendC::GlobalTensor<T> gammaGm_;
    AscendC::GlobalTensor<T> biasGm_;
    AscendC::GlobalTensor<T> outputGm_;
    uint32_t rows_;
    uint32_t cols_;
    uint32_t colsPad_;
    uint32_t rowBytes_;
    uint32_t rowPadBytes_;
    uint32_t hotRows_;
    float epsilon_;
    float colsInv_;
    bool hot_;
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
    if (dtype == kDtypeF32) {
        RunAddRmsNormBias<float>(x, residual, gamma, bias, output, rows, cols, epsilon, colsInv);
    } else if (dtype == kDtypeF16) {
        RunAddRmsNormBias<half>(x, residual, gamma, bias, output, rows, cols, epsilon, colsInv);
    } else {
        RunAddRmsNormBias<bfloat16_t>(x, residual, gamma, bias, output, rows, cols, epsilon, colsInv);
    }
}

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
    if (info_x.numTensors < 1 || info_x.tensors == 0 || info_x.tensors[0].shape == 0) {
        return;
    }
    const TensorInfo &info = info_x.tensors[0];
    if (info.numDims < 2 || info.numDims > 4) {
        return;
    }
    const int64_t dim = info.shape[info.numDims - 1];
    if (dim < 64 || dim > 32768) {
        return;
    }
    uint64_t rows = 1;
    for (int64_t axis = 0; axis + 1 < info.numDims; ++axis) {
        const int64_t extent = info.shape[axis];
        if (extent <= 0) {
            return;
        }
        const uint64_t next = rows * static_cast<uint64_t>(extent);
        if (next / static_cast<uint64_t>(extent) != rows) {
            return;
        }
        rows = next;
    }
    if (rows == 0) {
        return;
    }
    uint32_t dtype;
    if (info.dtype == 0) {
        dtype = kDtypeF32;
    } else if (info.dtype == 1) {
        dtype = kDtypeF16;
    } else if (info.dtype == 2) {
        dtype = kDtypeBf16;
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
