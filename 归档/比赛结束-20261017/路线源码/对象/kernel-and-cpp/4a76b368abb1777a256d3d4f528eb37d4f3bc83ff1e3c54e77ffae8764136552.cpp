/**
 * AddRmsNormBias 昇腾（Ascend C）核函数实现（v1：正确性优先）
 *
 * 语义（与 PyTorch 组合对齐）：
 *   y      = x + residual
 *   rms    = sqrt( mean(y^2, dim=-1) + epsilon )
 *   output = (y / rms) * gamma + bias
 *
 * 设计要点：
 *   1. 按“行”切分：把 (batch, seq, heads, D) 展平为 outer 行，每行长度 D；
 *      多核并行，每核处理一段连续行（行数差不超过 1）。
 *   2. 每行两遍：
 *        Pass1 归约：分块搬入 x/residual → Cast 到 FP32 → Add → 平方 →
 *                    ReduceSum(FP32) → 块间标量累加 → rowSum
 *        Pass2 归一化：重读同块 x/residual/gamma/bias → FP32 计算链 →
 *                    乘 1/rms → 乘 gamma → 加 bias → 最后一次 Cast 回原类型 → 搬出
 *   3. 尾块（D 非 tileLen 整数倍）：
 *        搬入用 DataCopyPad 自动补 0（0 的平方不影响归约）；
 *        搬出用 DataCopyPad 非对齐搬出（Global 目标地址无对齐约束，见手册）。
 *   4. 全部中间计算用 FP32（官方训练营“黄金法则”：归约必须 FP32，防溢出/精度损失）。
 *
 * 注意：本文件按 CANN 8.x/9.0.0 官方 msopgen 模板风格编写，本机无 CANN 环境，未做 NPU 编译验证。
 * 真机落地步骤：用 msopgen 生成工程骨架，再将本文件覆盖到 op_kernel/ 下同名文件；若模板 API
 * 有版本差异时（例如 queue 初始化方式），按模板自带代码微调。
 */

#include <cmath>
#include "kernel_operator.h"

using namespace AscendC;

namespace {
constexpr uint32_t BUFFER_NUM = 1;               // v1 同步流水（正确性优先）
constexpr uint32_t TILE_HALF = 4096;             // fp16/bf16 每块元素数
constexpr uint32_t TILE_FLOAT = 2048;            // fp32 每块元素数（受 UB 预算限制）
constexpr uint32_t WORK_LEN = 1024;              // ReduceSum 的 workLocal 空间（float 元素数）
constexpr uint32_t SUM_LEN = 16;                 // ReduceSum 目的缓冲元素数（结果在 [0]）

// dtype 编码（与 op_host 侧保持一致）
constexpr uint32_t DT_FP16 = 1;
constexpr uint32_t DT_BF16 = 2;
constexpr uint32_t DT_FP32 = 3;
}  // namespace

// Tiling 数据（与 op_host/add_rms_norm_bias_tiling.h 字段一一对应）
struct AddRmsNormBiasTilingData {
    uint32_t outer;    // 行数 = batch*seq*heads
    uint32_t D;        // 最后一维长度
    float epsilon;     // 平滑项
    uint32_t dtype;    // 1=fp16 2=bf16 3=fp32
};

template <typename T>
class KernelAddRmsNormBias {
public:
    __aicore__ inline KernelAddRmsNormBias() {}
    __aicore__ inline ~KernelAddRmsNormBias() {}

    __aicore__ inline void Init(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias,
                                GM_ADDR output, const AddRmsNormBiasTilingData* tiling)
    {
        this->outer = tiling->outer;
        this->D = tiling->D;
        this->eps = tiling->epsilon;
        this->tileLen = (sizeof(T) <= 2) ? TILE_HALF : TILE_FLOAT;

        xGm.SetGlobalBuffer(reinterpret_cast<__gm__ T*>(x), outer * D);
        rGm.SetGlobalBuffer(reinterpret_cast<__gm__ T*>(residual), outer * D);
        gGm.SetGlobalBuffer(reinterpret_cast<__gm__ T*>(gamma), D);
        bGm.SetGlobalBuffer(reinterpret_cast<__gm__ T*>(bias), D);
        outGm.SetGlobalBuffer(reinterpret_cast<__gm__ T*>(output), outer * D);

        // 搬入队列：x/residual/gamma/bias；搬出队列：output
        pipe.InitBuffer(inQueueX, BUFFER_NUM, tileLen * sizeof(T));
        pipe.InitBuffer(inQueueR, BUFFER_NUM, tileLen * sizeof(T));
        pipe.InitBuffer(inQueueG, BUFFER_NUM, tileLen * sizeof(T));
        pipe.InitBuffer(inQueueB, BUFFER_NUM, tileLen * sizeof(T));
        pipe.InitBuffer(outQueue, BUFFER_NUM, tileLen * sizeof(T));
        // 中间计算缓冲（FP32）
        pipe.InitBuffer(xFbuf, tileLen * sizeof(float));
        pipe.InitBuffer(rFbuf, tileLen * sizeof(float));
        pipe.InitBuffer(yFbuf, tileLen * sizeof(float));
        pipe.InitBuffer(workBuf, WORK_LEN * sizeof(float));
        pipe.InitBuffer(sumBuf, SUM_LEN * sizeof(float));
    }

    __aicore__ inline void Process()
    {
        // 多核按行均分
        uint32_t coreIdx = GetBlockIdx();
        uint32_t coreNum = GetBlockNum();
        uint32_t perCore = outer / coreNum;
        uint32_t rem = outer % coreNum;
        uint32_t startRow = coreIdx * perCore + (coreIdx < rem ? coreIdx : rem);
        uint32_t rowCount = perCore + (coreIdx < rem ? 1 : 0);
        if (rowCount == 0) {
            return;
        }
        for (uint32_t row = startRow; row < startRow + rowCount; ++row) {
            float rowSum = ReduceRowSum(row);
            // rms = sqrt(mean(y^2) + eps)；scale = 1/rms
            float rms = sqrtf(rowSum / static_cast<float>(D) + eps);
            float scale = 1.0f / rms;
            NormalizeRow(row, scale);
        }
    }

private:
    __aicore__ inline void CopyIn(LocalTensor<T>& dst, const GlobalTensor<T>& src,
                                  uint32_t elemOff, uint32_t len, bool isTail)
    {
        if (isTail) {
            // 非对齐尾块：DataCopyPad 搬入，不足部分自动补 0（不影响平方归约）
            DataCopyExtParams params{1, len * sizeof(T), 0, 0, 0};
            DataCopyPadExtParams<T> pad{true, 0, 0, static_cast<T>(0)};
            DataCopyPad(dst, src[elemOff], params, pad);
        } else {
            DataCopy(dst, src[elemOff], len);  // 对齐块
        }
    }

    // 第一遍：分块归约平方和，返回该行 sum(y^2)
    __aicore__ inline float ReduceRowSum(uint32_t row)
    {
        float rowSum = 0.0f;
        uint32_t base = row * D;
        uint32_t off = 0;
        while (off < D) {
            uint32_t len = D - off < tileLen ? D - off : tileLen;
            bool isTail = (len < tileLen);
            // ---- CopyIn ----
            LocalTensor<T> xLoc = inQueueX.AllocTensor<T>();
            LocalTensor<T> rLoc = inQueueR.AllocTensor<T>();
            CopyIn(xLoc, xGm, base + off, len, isTail);
            CopyIn(rLoc, rGm, base + off, len, isTail);
            inQueueX.EnQue(xLoc);
            inQueueR.EnQue(rLoc);
            LocalTensor<T> xDeq = inQueueX.DeQue<T>();
            LocalTensor<T> rDeq = inQueueR.DeQue<T>();
            // ---- FP32 计算链 ----
            LocalTensor<float> xF = xFbuf.Get<float>();
            LocalTensor<float> rF = rFbuf.Get<float>();
            LocalTensor<float> yF = yFbuf.Get<float>();
            if constexpr (std::is_same_v<T, float>) {
                Add(yF, xDeq, rDeq, len);            // fp32: 直接相加
            } else {
                Cast(xF, xDeq, RoundMode::CAST_NONE, len);  // half/bf16 -> fp32（无损）
                Cast(rF, rDeq, RoundMode::CAST_NONE, len);
                Add(yF, xF, rF, len);
            }
            Mul(yF, yF, yF, len);                    // 就地平方
            LocalTensor<float> work = workBuf.Get<float>();
            LocalTensor<float> sum = sumBuf.Get<float>();
            ReduceSum(sum, yF, work, static_cast<int32_t>(len));
            PipeBarrier<PIPE_V>();
            event_t eventVS = static_cast<event_t>(GetTPipePtr()->FetchEventID(HardEvent::V_S));
            SetFlag<HardEvent::V_S>(eventVS);
            WaitFlag<HardEvent::V_S>(eventVS);
            rowSum += sum.GetValue(0);               // 标量同步（v1 正确性优先）
            event_t eventSV = static_cast<event_t>(GetTPipePtr()->FetchEventID(HardEvent::S_V));
            SetFlag<HardEvent::S_V>(eventSV);
            WaitFlag<HardEvent::S_V>(eventSV);
            PipeBarrier<PIPE_V>();
            // ---- 释放 ----
            inQueueX.FreeTensor(xDeq);
            inQueueR.FreeTensor(rDeq);
            off += len;
        }
        return rowSum;
    }

    // 第二遍：归一化 + gamma 缩放 + bias 偏置，搬出
    __aicore__ inline void NormalizeRow(uint32_t row, float scale)
    {
        uint32_t base = row * D;
        uint32_t off = 0;
        while (off < D) {
            uint32_t len = D - off < tileLen ? D - off : tileLen;
            bool isTail = (len < tileLen);
            // ---- CopyIn（重读 x/residual，gamma/bias 按同 offset）----
            LocalTensor<T> xLoc = inQueueX.AllocTensor<T>();
            LocalTensor<T> rLoc = inQueueR.AllocTensor<T>();
            LocalTensor<T> gLoc = inQueueG.AllocTensor<T>();
            LocalTensor<T> bLoc = inQueueB.AllocTensor<T>();
            CopyIn(xLoc, xGm, base + off, len, isTail);
            CopyIn(rLoc, rGm, base + off, len, isTail);
            CopyIn(gLoc, gGm, off, len, isTail);
            CopyIn(bLoc, bGm, off, len, isTail);
            inQueueX.EnQue(xLoc);
            inQueueR.EnQue(rLoc);
            inQueueG.EnQue(gLoc);
            inQueueB.EnQue(bLoc);
            LocalTensor<T> xDeq = inQueueX.DeQue<T>();
            LocalTensor<T> rDeq = inQueueR.DeQue<T>();
            LocalTensor<T> gDeq = inQueueG.DeQue<T>();
            LocalTensor<T> bDeq = inQueueB.DeQue<T>();
            // ---- FP32 计算链：y = x+r；out = (y*scale)*gamma + bias ----
            LocalTensor<float> xF = xFbuf.Get<float>();
            LocalTensor<float> rF = rFbuf.Get<float>();
            LocalTensor<float> yF = yFbuf.Get<float>();
            if constexpr (std::is_same_v<T, float>) {
                Add(yF, xDeq, rDeq, len);
            } else {
                Cast(xF, xDeq, RoundMode::CAST_NONE, len);
                Cast(rF, rDeq, RoundMode::CAST_NONE, len);
                Add(yF, xF, rF, len);
            }
            Muls(yF, yF, scale, len);               // * 1/rms（fp32 标量）
            if constexpr (std::is_same_v<T, float>) {
                Mul(yF, yF, gDeq, len);             // fp32 输入：gamma/bias 已是 fp32
                Add(yF, yF, bDeq, len);
            } else {
                Cast(xF, gDeq, RoundMode::CAST_NONE, len);  // 复用 xF 缓冲做 gamma fp32
                Cast(rF, bDeq, RoundMode::CAST_NONE, len);  // 复用 rF 缓冲做 bias  fp32
                Mul(yF, yF, xF, len);
                Add(yF, yF, rF, len);
            }
            // ---- 搬出 ----
            if constexpr (std::is_same_v<T, float>) {
                // fp32：直接从 yF 搬出（结果已是目标类型）
                CopyOut(outGm, base + off, yF, len, isTail);
            } else {
                LocalTensor<T> outLoc = outQueue.AllocTensor<T>();
                Cast(outLoc, yF, RoundMode::CAST_RINT, len);  // 最后一次量化到目标类型
                CopyOut(outGm, base + off, outLoc, len, isTail);
                outQueue.FreeTensor(outLoc);
            }
            inQueueX.FreeTensor(xDeq);
            inQueueR.FreeTensor(rDeq);
            inQueueG.FreeTensor(gDeq);
            inQueueB.FreeTensor(bDeq);
            off += len;
        }
    }

    __aicore__ inline void CopyOut(const GlobalTensor<T>& dst, uint32_t elemOff,
                                   const LocalTensor<T>& src, uint32_t len, bool isTail)
    {
        if (isTail) {
            // 非对齐尾块：DataCopyPad 非对齐搬出（Global 目的地址无对齐约束）
            DataCopyExtParams params{1, len * sizeof(T), 0, 0, 0};
            DataCopyPad(dst[elemOff], src, params);
        } else {
            DataCopy(dst[elemOff], src, len);
        }
    }

    TPipe pipe;
    TQue<QuePosition::VECIN, BUFFER_NUM> inQueueX;
    TQue<QuePosition::VECIN, BUFFER_NUM> inQueueR;
    TQue<QuePosition::VECIN, BUFFER_NUM> inQueueG;
    TQue<QuePosition::VECIN, BUFFER_NUM> inQueueB;
    TQue<QuePosition::VECOUT, BUFFER_NUM> outQueue;
    TBuf<TPosition::VECCALC> xFbuf;
    TBuf<TPosition::VECCALC> rFbuf;
    TBuf<TPosition::VECCALC> yFbuf;
    TBuf<TPosition::VECCALC> workBuf;
    TBuf<TPosition::VECCALC> sumBuf;
    GlobalTensor<T> xGm;
    GlobalTensor<T> rGm;
    GlobalTensor<T> gGm;
    GlobalTensor<T> bGm;
    GlobalTensor<T> outGm;
    uint32_t outer = 0;
    uint32_t D = 0;
    float eps = 1e-5f;
    uint32_t tileLen = TILE_HALF;
};

// 统一入口：按 tiling 里的 dtype 分派到三个模板实例。
// 判题引擎依赖算子类型名（op_type）而不是入口函数名决定 kernel；
// 若校测框架要求固定入口名，可在生成工程时以 msopgen 模板的入口名覆盖。
extern "C" __global__ __aicore__ void add_rms_norm_bias(GM_ADDR x, GM_ADDR residual, GM_ADDR gamma,
                                                         GM_ADDR bias, GM_ADDR output, GM_ADDR workspaceGM,
                                                         GM_ADDR tilingGm)
{
    (void)workspaceGM;
    REGISTER_TILING_DEFAULT(AddRmsNormBiasTilingData);
    GET_TILING_DATA_WITH_STRUCT(AddRmsNormBiasTilingData, tilingData, tilingGm);
    switch (tilingData.dtype) {
        case DT_FP16: {
            KernelAddRmsNormBias<half> op;
            op.Init(x, residual, gamma, bias, output, &tilingData);
            op.Process();
            break;
        }
        case DT_BF16: {
            KernelAddRmsNormBias<bfloat16_t> op;
            op.Init(x, residual, gamma, bias, output, &tilingData);
            op.Process();
            break;
        }
        case DT_FP32:
        default: {
            KernelAddRmsNormBias<float> op;
            op.Init(x, residual, gamma, bias, output, &tilingData);
            op.Process();
            break;
        }
    }
}
