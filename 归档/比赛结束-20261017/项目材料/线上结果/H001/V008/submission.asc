#include <cmath>
#include "kernel_operator.h"

// H001 small-D / high-R AddRmsNormBias. npu_kernel_dev kernel.asc body.
// No anonymous namespace. No C++17-only forms. Judge provides TensorInfo,
// TensorGroupInfo, aclrtStream, and ACL runtime decls before this file.

struct H001TilingData {
    uint32_t rows;
    uint32_t cols;
    uint32_t colsPad;
    uint32_t hotRows;
    uint32_t mode;
    float epsilon;
    float colsInv;
};

constexpr uint32_t H001_BYTES_PER_BLOCK = 32;
constexpr uint32_t H001_HOT_D = 1024;
constexpr uint32_t H001_MAX_HOT_ROWS = 64;
constexpr uint32_t H001_HOT_BUDGET = 96 * 1024;
constexpr uint32_t H001_CHUNK_D = 1024;
constexpr int32_t H001_DTYPE_F32 = 0;
constexpr int32_t H001_DTYPE_F16 = 1;
constexpr int32_t H001_DTYPE_BF16 = 2;

template <typename T>
struct H001TypeOps {
    __aicore__ inline static void ToFloat(AscendC::LocalTensor<float> dst, AscendC::LocalTensor<T> src, uint32_t count)
    {
        AscendC::Cast<float, T>(dst, src, AscendC::RoundMode::CAST_NONE, static_cast<int32_t>(count));
    }
    __aicore__ inline static void FromFloat(AscendC::LocalTensor<T> dst, AscendC::LocalTensor<float> src, uint32_t count)
    {
        AscendC::Cast<T, float>(dst, src, AscendC::RoundMode::CAST_ROUND, static_cast<int32_t>(count));
    }
};

template <>
struct H001TypeOps<float> {
    __aicore__ inline static void ToFloat(AscendC::LocalTensor<float> dst, AscendC::LocalTensor<float> src, uint32_t count)
    {
        AscendC::Adds(dst, src, 0.0f, static_cast<int32_t>(count));
    }
    __aicore__ inline static void FromFloat(AscendC::LocalTensor<float> dst, AscendC::LocalTensor<float> src, uint32_t count)
    {
        AscendC::Adds(dst, src, 0.0f, static_cast<int32_t>(count));
    }
};

template <typename T>
class H001Kernel {
public:
    __aicore__ inline H001Kernel(AscendC::TPipe *pipe)
    {
        pipe_ = pipe;
    }

    __aicore__ inline void Init(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
        const __gm__ H001TilingData *tiling)
    {
        tiling_ = tiling;
        xGm_.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(x));
        residualGm_.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(residual));
        gammaGm_.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(gamma));
        biasGm_.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(bias));
        outputGm_.SetGlobalBuffer(reinterpret_cast<__gm__ T *>(output));

        const uint32_t colsPad = tiling_->colsPad;
        const uint32_t rowPadBytes = colsPad * static_cast<uint32_t>(sizeof(T));
        if (tiling_->mode == 1) {
            hotRows_ = tiling_->hotRows;
        } else {
            hotRows_ = 1;
        }
        const uint32_t tileBytes = (tiling_->mode == 1) ? (hotRows_ * rowPadBytes) : (H001_CHUNK_D * sizeof(T));
        const uint32_t workElems = (tiling_->mode == 1) ? colsPad : H001_CHUNK_D;
        pipe_->InitBuffer(xQueue_, 1, tileBytes);
        pipe_->InitBuffer(residualQueue_, 1, tileBytes);
        pipe_->InitBuffer(outputQueue_, 1, tileBytes);
        pipe_->InitBuffer(gammaQueue_, 1, workElems * sizeof(T));
        pipe_->InitBuffer(biasQueue_, 1, workElems * sizeof(T));
        pipe_->InitBuffer(gammaF32Buf_, workElems * sizeof(float));
        pipe_->InitBuffer(biasF32Buf_, workElems * sizeof(float));
        pipe_->InitBuffer(workABuf_, workElems * sizeof(float));
        pipe_->InitBuffer(workBBuf_, workElems * sizeof(float));
        pipe_->InitBuffer(partialBuf_, 32);
        pipe_->InitBuffer(reduceTmpBuf_, 8192);
    }

    __aicore__ inline void Process()
    {
        const uint32_t core = static_cast<uint32_t>(AscendC::GetBlockIdx());
        const uint32_t coreCount = static_cast<uint32_t>(AscendC::GetBlockNum());
        const uint32_t rows = tiling_->rows;
        const uint32_t rowsPerCore = (rows + coreCount - 1) / coreCount;
        const uint32_t rowBegin = core * rowsPerCore;
        uint32_t rowEnd = rowBegin + rowsPerCore;
        if (rowEnd > rows) {
            rowEnd = rows;
        }
        if (rowBegin >= rowEnd) {
            return;
        }
        if (tiling_->mode == 1) {
            ProcessHot(rowBegin, rowEnd);
        } else {
            ProcessWide(rowBegin, rowEnd);
        }
    }

private:
    __aicore__ inline void FillCopy(AscendC::DataCopyExtParams &params, uint32_t blockCount, uint32_t blockLen)
    {
        params.blockCount = static_cast<uint16_t>(blockCount);
        params.blockLen = blockLen;
        params.srcStride = 0;
        params.dstStride = 0;
        params.rsv = 0;
    }

    __aicore__ inline void FillPad(AscendC::DataCopyPadExtParams<T> &params, bool isPad)
    {
        params.isPad = isPad;
        params.leftPadding = 0;
        params.rightPadding = 0;
        params.paddingValue = static_cast<T>(0);
    }

    __aicore__ inline void CopyRows(AscendC::LocalTensor<T> dst, AscendC::GlobalTensor<T> src, uint32_t rowBegin,
        uint32_t rowCount, uint32_t rowBytes)
    {
        const uint32_t cols = tiling_->cols;
        const uint64_t srcBase = static_cast<uint64_t>(rowBegin) * cols;
        if (rowCount > 1 && rowBytes % H001_BYTES_PER_BLOCK == 0 &&
            tiling_->colsPad * static_cast<uint32_t>(sizeof(T)) == rowBytes) {
            AscendC::DataCopyExtParams copyParams;
            FillCopy(copyParams, rowCount, rowBytes);
            AscendC::DataCopyPadExtParams<T> padParams;
            FillPad(padParams, false);
            AscendC::DataCopyPad(dst, src[srcBase], copyParams, padParams);
            return;
        }
        for (uint32_t row = 0; row < rowCount; ++row) {
            AscendC::DataCopyExtParams copyParams;
            FillCopy(copyParams, 1, rowBytes);
            AscendC::DataCopyPadExtParams<T> padParams;
            FillPad(padParams, true);
            AscendC::DataCopyPad(dst[row * tiling_->colsPad], src[srcBase + row * cols], copyParams, padParams);
        }
    }

    __aicore__ inline void CopyRow(AscendC::LocalTensor<T> dst, AscendC::GlobalTensor<T> src, uint32_t rowBytes)
    {
        AscendC::DataCopyExtParams copyParams;
        FillCopy(copyParams, 1, rowBytes);
        AscendC::DataCopyPadExtParams<T> padParams;
        FillPad(padParams, true);
        AscendC::DataCopyPad(dst, src, copyParams, padParams);
    }

    __aicore__ inline void StoreRows(AscendC::LocalTensor<T> src, AscendC::GlobalTensor<T> dst, uint32_t rowBegin,
        uint32_t rowCount, uint32_t rowBytes)
    {
        const uint32_t cols = tiling_->cols;
        const uint64_t dstBase = static_cast<uint64_t>(rowBegin) * cols;
        if (rowCount > 1 && rowBytes % H001_BYTES_PER_BLOCK == 0 &&
            tiling_->colsPad * static_cast<uint32_t>(sizeof(T)) == rowBytes) {
            AscendC::DataCopyExtParams copyParams;
            FillCopy(copyParams, rowCount, rowBytes);
            AscendC::DataCopyPad(dst[dstBase], src, copyParams);
            return;
        }
        for (uint32_t row = 0; row < rowCount; ++row) {
            AscendC::DataCopyExtParams copyParams;
            FillCopy(copyParams, 1, rowBytes);
            AscendC::DataCopyPad(dst[dstBase + row * cols], src[row * tiling_->colsPad], copyParams);
        }
    }

    __aicore__ inline void LoadGammaBiasResident()
    {
        AscendC::LocalTensor<T> gammaAlloc = gammaQueue_.AllocTensor<T>();
        AscendC::LocalTensor<T> biasAlloc = biasQueue_.AllocTensor<T>();
        const uint32_t rowBytes = tiling_->cols * static_cast<uint32_t>(sizeof(T));
        CopyRow(gammaAlloc, gammaGm_, rowBytes);
        CopyRow(biasAlloc, biasGm_, rowBytes);
        gammaQueue_.EnQue(gammaAlloc);
        biasQueue_.EnQue(biasAlloc);
        AscendC::LocalTensor<T> gamma = gammaQueue_.DeQue<T>();
        AscendC::LocalTensor<T> bias = biasQueue_.DeQue<T>();
        AscendC::LocalTensor<float> gammaF32 = gammaF32Buf_.Get<float>();
        AscendC::LocalTensor<float> biasF32 = biasF32Buf_.Get<float>();
        H001TypeOps<T>::ToFloat(gammaF32, gamma, tiling_->cols);
        H001TypeOps<T>::ToFloat(biasF32, bias, tiling_->cols);
        gammaQueue_.FreeTensor(gamma);
        biasQueue_.FreeTensor(bias);
    }

    // V008 single change vs V007: ReduceSum for wide-path sum(u*u) too (hot ApplyRow already uses it).
    __aicore__ inline float ChunkSumSquares(AscendC::LocalTensor<T> xRow, AscendC::LocalTensor<T> residualRow,
        uint32_t count)
    {
        AscendC::LocalTensor<float> x32 = workABuf_.Get<float>();
        AscendC::LocalTensor<float> r32 = workBBuf_.Get<float>();
        AscendC::LocalTensor<float> partial = partialBuf_.Get<float>();
        AscendC::LocalTensor<float> reduceTmp = reduceTmpBuf_.Get<float>();
        H001TypeOps<T>::ToFloat(x32, xRow, count);
        H001TypeOps<T>::ToFloat(r32, residualRow, count);
        AscendC::Add(x32, x32, r32, static_cast<int32_t>(count));
        AscendC::Mul(r32, x32, x32, static_cast<int32_t>(count));
        AscendC::ReduceSum<float, true>(partial, r32, reduceTmp, static_cast<int32_t>(count));
        return partial.GetValue(0);
    }

    __aicore__ inline void ApplyInvRms(AscendC::LocalTensor<T> outputRow, AscendC::LocalTensor<T> xRow,
        AscendC::LocalTensor<T> residualRow, AscendC::LocalTensor<float> gammaF32,
        AscendC::LocalTensor<float> biasF32, float invRms, uint32_t count)
    {
        AscendC::LocalTensor<float> x32 = workABuf_.Get<float>();
        AscendC::LocalTensor<float> r32 = workBBuf_.Get<float>();
        H001TypeOps<T>::ToFloat(x32, xRow, count);
        H001TypeOps<T>::ToFloat(r32, residualRow, count);
        AscendC::Add(x32, x32, r32, static_cast<int32_t>(count));
        AscendC::Muls(x32, x32, invRms, static_cast<int32_t>(count));
        AscendC::Mul(x32, x32, gammaF32, static_cast<int32_t>(count));
        AscendC::Add(x32, x32, biasF32, static_cast<int32_t>(count));
        H001TypeOps<T>::FromFloat(outputRow, x32, count);
    }

    // V007 single change vs V006: ReduceSum for sum(u*u) on the hot ApplyRow path.
    __aicore__ inline void ApplyRow(AscendC::LocalTensor<T> outputRow, AscendC::LocalTensor<T> xRow,
        AscendC::LocalTensor<T> residualRow, AscendC::LocalTensor<float> gammaF32,
        AscendC::LocalTensor<float> biasF32, uint32_t count)
    {
        AscendC::LocalTensor<float> x32 = workABuf_.Get<float>();
        AscendC::LocalTensor<float> r32 = workBBuf_.Get<float>();
        AscendC::LocalTensor<float> partial = partialBuf_.Get<float>();
        AscendC::LocalTensor<float> reduceTmp = reduceTmpBuf_.Get<float>();
        H001TypeOps<T>::ToFloat(x32, xRow, count);
        H001TypeOps<T>::ToFloat(r32, residualRow, count);
        AscendC::Add(x32, x32, r32, static_cast<int32_t>(count));
        AscendC::Mul(r32, x32, x32, static_cast<int32_t>(count));
        AscendC::ReduceSum<float, true>(partial, r32, reduceTmp, static_cast<int32_t>(count));
        const float sum = partial.GetValue(0);
        AscendC::Duplicate(r32, sum * tiling_->colsInv + tiling_->epsilon, 1);
        AscendC::Sqrt<float>(r32, r32, 1);
        const float invRms = 1.0f / r32.GetValue(0);
        AscendC::Muls(x32, x32, invRms, static_cast<int32_t>(count));
        AscendC::Mul(x32, x32, gammaF32, static_cast<int32_t>(count));
        AscendC::Add(x32, x32, biasF32, static_cast<int32_t>(count));
        H001TypeOps<T>::FromFloat(outputRow, x32, count);
    }

    __aicore__ inline void ProcessHot(uint32_t rowBegin, uint32_t rowEnd)
    {
        LoadGammaBiasResident();
        AscendC::LocalTensor<float> gammaF32 = gammaF32Buf_.Get<float>();
        AscendC::LocalTensor<float> biasF32 = biasF32Buf_.Get<float>();
        const uint32_t rowBytes = tiling_->cols * static_cast<uint32_t>(sizeof(T));
        for (uint32_t tileBegin = rowBegin; tileBegin < rowEnd; tileBegin += hotRows_) {
            uint32_t tileRows = rowEnd - tileBegin;
            if (tileRows > hotRows_) {
                tileRows = hotRows_;
            }
            AscendC::LocalTensor<T> xAlloc = xQueue_.AllocTensor<T>();
            AscendC::LocalTensor<T> residualAlloc = residualQueue_.AllocTensor<T>();
            CopyRows(xAlloc, xGm_, tileBegin, tileRows, rowBytes);
            CopyRows(residualAlloc, residualGm_, tileBegin, tileRows, rowBytes);
            xQueue_.EnQue(xAlloc);
            residualQueue_.EnQue(residualAlloc);
            AscendC::LocalTensor<T> x = xQueue_.DeQue<T>();
            AscendC::LocalTensor<T> residual = residualQueue_.DeQue<T>();
            AscendC::LocalTensor<T> outputAlloc = outputQueue_.AllocTensor<T>();
            for (uint32_t row = 0; row < tileRows; ++row) {
                ApplyRow(outputAlloc[row * tiling_->colsPad], x[row * tiling_->colsPad],
                    residual[row * tiling_->colsPad], gammaF32, biasF32, tiling_->cols);
            }
            outputQueue_.EnQue(outputAlloc);
            AscendC::LocalTensor<T> output = outputQueue_.DeQue<T>();
            StoreRows(output, outputGm_, tileBegin, tileRows, rowBytes);
            outputQueue_.FreeTensor(output);
            xQueue_.FreeTensor(x);
            residualQueue_.FreeTensor(residual);
        }
    }

    __aicore__ inline void ProcessWide(uint32_t rowBegin, uint32_t rowEnd)
    {
        AscendC::LocalTensor<float> gammaF32 = gammaF32Buf_.Get<float>();
        AscendC::LocalTensor<float> biasF32 = biasF32Buf_.Get<float>();
        AscendC::LocalTensor<float> rmsTmp = workABuf_.Get<float>();
        const uint32_t cols = tiling_->cols;
        for (uint32_t row = rowBegin; row < rowEnd; ++row) {
            const uint64_t rowOff = static_cast<uint64_t>(row) * cols;
            float sum = 0.0f;
            for (uint32_t colBegin = 0; colBegin < cols; colBegin += H001_CHUNK_D) {
                uint32_t count = cols - colBegin;
                if (count > H001_CHUNK_D) {
                    count = H001_CHUNK_D;
                }
                const uint32_t chunkBytes = count * static_cast<uint32_t>(sizeof(T));
                AscendC::LocalTensor<T> xAlloc = xQueue_.AllocTensor<T>();
                AscendC::LocalTensor<T> residualAlloc = residualQueue_.AllocTensor<T>();
                CopyRow(xAlloc, xGm_[rowOff + colBegin], chunkBytes);
                CopyRow(residualAlloc, residualGm_[rowOff + colBegin], chunkBytes);
                xQueue_.EnQue(xAlloc);
                residualQueue_.EnQue(residualAlloc);
                AscendC::LocalTensor<T> x = xQueue_.DeQue<T>();
                AscendC::LocalTensor<T> residual = residualQueue_.DeQue<T>();
                sum += ChunkSumSquares(x, residual, count);
                xQueue_.FreeTensor(x);
                residualQueue_.FreeTensor(residual);
            }
            AscendC::Duplicate(rmsTmp, sum * tiling_->colsInv + tiling_->epsilon, 1);
            AscendC::Sqrt<float>(rmsTmp, rmsTmp, 1);
            const float invRms = 1.0f / rmsTmp.GetValue(0);
            for (uint32_t colBegin = 0; colBegin < cols; colBegin += H001_CHUNK_D) {
                uint32_t count = cols - colBegin;
                if (count > H001_CHUNK_D) {
                    count = H001_CHUNK_D;
                }
                const uint32_t chunkBytes = count * static_cast<uint32_t>(sizeof(T));
                AscendC::LocalTensor<T> xAlloc = xQueue_.AllocTensor<T>();
                AscendC::LocalTensor<T> residualAlloc = residualQueue_.AllocTensor<T>();
                AscendC::LocalTensor<T> gammaAlloc = gammaQueue_.AllocTensor<T>();
                AscendC::LocalTensor<T> biasAlloc = biasQueue_.AllocTensor<T>();
                CopyRow(xAlloc, xGm_[rowOff + colBegin], chunkBytes);
                CopyRow(residualAlloc, residualGm_[rowOff + colBegin], chunkBytes);
                CopyRow(gammaAlloc, gammaGm_[colBegin], chunkBytes);
                CopyRow(biasAlloc, biasGm_[colBegin], chunkBytes);
                xQueue_.EnQue(xAlloc);
                residualQueue_.EnQue(residualAlloc);
                gammaQueue_.EnQue(gammaAlloc);
                biasQueue_.EnQue(biasAlloc);
                AscendC::LocalTensor<T> x = xQueue_.DeQue<T>();
                AscendC::LocalTensor<T> residual = residualQueue_.DeQue<T>();
                AscendC::LocalTensor<T> gamma = gammaQueue_.DeQue<T>();
                AscendC::LocalTensor<T> bias = biasQueue_.DeQue<T>();
                H001TypeOps<T>::ToFloat(gammaF32, gamma, count);
                H001TypeOps<T>::ToFloat(biasF32, bias, count);
                AscendC::LocalTensor<T> outputAlloc = outputQueue_.AllocTensor<T>();
                ApplyInvRms(outputAlloc, x, residual, gammaF32, biasF32, invRms, count);
                outputQueue_.EnQue(outputAlloc);
                AscendC::LocalTensor<T> output = outputQueue_.DeQue<T>();
                AscendC::DataCopyExtParams copyParams;
                FillCopy(copyParams, 1, chunkBytes);
                AscendC::DataCopyPad(outputGm_[rowOff + colBegin], output, copyParams);
                outputQueue_.FreeTensor(output);
                xQueue_.FreeTensor(x);
                residualQueue_.FreeTensor(residual);
                gammaQueue_.FreeTensor(gamma);
                biasQueue_.FreeTensor(bias);
            }
        }
    }

    AscendC::TPipe *pipe_;
    const __gm__ H001TilingData *tiling_;
    AscendC::GlobalTensor<T> xGm_;
    AscendC::GlobalTensor<T> residualGm_;
    AscendC::GlobalTensor<T> gammaGm_;
    AscendC::GlobalTensor<T> biasGm_;
    AscendC::GlobalTensor<T> outputGm_;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> xQueue_;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> residualQueue_;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> gammaQueue_;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> biasQueue_;
    AscendC::TQue<AscendC::TPosition::VECOUT, 1> outputQueue_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> gammaF32Buf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> biasF32Buf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> workABuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> workBBuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> partialBuf_;
    AscendC::TBuf<AscendC::TPosition::VECCALC> reduceTmpBuf_;
    uint32_t hotRows_;
};

template <typename T>
__aicore__ inline void H001Run(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    GM_ADDR tiling)
{
    AscendC::TPipe pipe;
    H001Kernel<T> kernel(&pipe);
    kernel.Init(x, residual, gamma, bias, output, reinterpret_cast<const __gm__ H001TilingData *>(tiling));
    kernel.Process();
}

extern "C" __global__ __vector__ void h001_add_rms_norm_bias_fp16(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output, GM_ADDR tiling)
{
    H001Run<half>(x, residual, gamma, bias, output, tiling);
}

extern "C" __global__ __vector__ void h001_add_rms_norm_bias_bf16(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output, GM_ADDR tiling)
{
    H001Run<bfloat16_t>(x, residual, gamma, bias, output, tiling);
}

extern "C" __global__ __vector__ void h001_add_rms_norm_bias_fp32(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output, GM_ADDR tiling)
{
    H001Run<float>(x, residual, gamma, bias, output, tiling);
}

extern "C" void run_kernel(
    GM_ADDR x, const TensorGroupInfo& info_x,
    GM_ADDR residual, const TensorGroupInfo& info_residual,
    GM_ADDR gamma, const TensorGroupInfo& info_gamma,
    GM_ADDR bias, const TensorGroupInfo& info_bias,
    GM_ADDR output, const TensorGroupInfo& info_output,
    int64_t availableCoreNum, aclrtStream stream, float epsilon)
{
    (void)info_residual;
    (void)info_gamma;
    (void)info_bias;
    (void)info_output;
    if (info_x.numTensors < 1 || info_x.tensors == nullptr ||
        info_x.tensors[0].shape == nullptr || info_x.tensors[0].numDims < 2) {
        return;
    }
    const TensorInfo& xInfo = info_x.tensors[0];
    if (xInfo.numDims > 4) {
        return;
    }
    const int64_t dim = xInfo.shape[xInfo.numDims - 1];
    if (dim < 64 || dim > 32768) {
        return;
    }
    uint64_t rows = 1;
    for (int64_t axis = 0; axis + 1 < xInfo.numDims; ++axis) {
        if (xInfo.shape[axis] <= 0) {
            return;
        }
        rows = rows * static_cast<uint64_t>(xInfo.shape[axis]);
    }
    if (rows == 0 || rows > 0xffffffffULL || dim > 0xffffffffULL) {
        return;
    }
    const uint32_t rows32 = static_cast<uint32_t>(rows);
    const uint32_t cols = static_cast<uint32_t>(dim);
    int32_t dtype = xInfo.dtype;
    uint32_t elemBytes = 0;
    if (dtype == H001_DTYPE_F32) {
        elemBytes = 4U;
    } else if (dtype == H001_DTYPE_F16 || dtype == H001_DTYPE_BF16) {
        elemBytes = 2U;
        if (dtype == H001_DTYPE_BF16) {
            dtype = H001_DTYPE_BF16;
        }
    } else if (dtype == 27) {
        dtype = H001_DTYPE_BF16;
        elemBytes = 2U;
    } else {
        return;
    }

    H001TilingData tiling;
    tiling.rows = rows32;
    tiling.cols = cols;
    tiling.colsPad = ((cols * elemBytes + H001_BYTES_PER_BLOCK - 1U) / H001_BYTES_PER_BLOCK * H001_BYTES_PER_BLOCK) / elemBytes;
    tiling.epsilon = epsilon;
    tiling.colsInv = 1.0f / static_cast<float>(cols);

    const uint32_t rowPadBytes = tiling.colsPad * elemBytes;
    uint32_t maxRows = H001_HOT_BUDGET / (3U * rowPadBytes);
    if (maxRows < 1U) {
        maxRows = 1U;
    }
    if (maxRows > H001_MAX_HOT_ROWS) {
        maxRows = H001_MAX_HOT_ROWS;
    }
    tiling.hotRows = maxRows;
    const bool hot = (cols <= H001_HOT_D) ? true : false;
    tiling.mode = hot ? 1U : 0U;

    uint32_t blockNum = static_cast<uint32_t>(availableCoreNum > 0 ? availableCoreNum : 1);
    if (blockNum > 40U) {
        blockNum = 40U;
    }
    if (blockNum > rows32) {
        blockNum = rows32;
    }
    if (blockNum == 0U) {
        return;
    }

    GM_ADDR tilingDevice = nullptr;
    if (aclrtMalloc(reinterpret_cast<void**>(&tilingDevice), sizeof(tiling),
                    ACL_MEM_MALLOC_HUGE_FIRST) != ACL_SUCCESS) {
        return;
    }
    if (aclrtMemcpy(tilingDevice, sizeof(tiling), &tiling, sizeof(tiling),
                    ACL_MEMCPY_HOST_TO_DEVICE) != ACL_SUCCESS) {
        aclrtFree(tilingDevice);
        return;
    }

    if (dtype == H001_DTYPE_F16) {
        h001_add_rms_norm_bias_fp16<<<blockNum, nullptr, stream>>>(x, residual, gamma, bias, output, tilingDevice);
    } else if (dtype == H001_DTYPE_BF16) {
        h001_add_rms_norm_bias_bf16<<<blockNum, nullptr, stream>>>(x, residual, gamma, bias, output, tilingDevice);
    } else {
        h001_add_rms_norm_bias_fp32<<<blockNum, nullptr, stream>>>(x, residual, gamma, bias, output, tilingDevice);
    }
    aclrtSynchronizeStream(stream);
    aclrtFree(tilingDevice);
}
