# Agent 3 调研报告：Ascend 官方开源仓库中的 RMSNorm / AddRmsNorm 实现调研

- 调研日期：2026-09-12
- 调研人：Agent 3（官方 Ascend 开源仓库方向）
- 调研对象：CANN 官方算子库与官方样例仓中与 `AddRmsNormBias`（y=x+residual → 沿最后一维 RMSNorm → ×gamma → +bias）相关的实现
- 目标环境：CANN 9.0.0，SoC `dav-2201`（Atlas A2 / 910B 系，`__CCE_AICORE__ == 220`），直调模式单文件 `kernel.asc`
- 调研方式：仅在线读取（GitHub raw / API、WebFetch），未克隆仓库到项目目录，未上传任何内容
- 本机为 macOS 无 NPU：本报告中一切"官方支持/性能数据"均引自官方仓库文档与源码，**不代表本题目已完成 NPU 编译或验证**

---

## 一、仓库清单与覆盖情况

| # | 仓库 | 官方主地址（GitCode） | GitHub 镜像/同源仓 | 证据等级 | 覆盖深度 |
|---|------|----------------------|--------------------|----------|----------|
| 1 | **ops-nn**（CANN 神经网络算子库，即社区所称 cann-ops / 原 cann-ops-adv 生态的现行仓库） | https://gitcode.com/cann/ops-nn | https://github.com/hicann/ops-nn（master + `v9.0.0` tag） | A | **源码级**：`norm/add_rms_norm` 全部 5 种策略 kernel、tiling 实现、`norm/rms_norm` 公共基类与归约件、`gemma_rms_norm` 对照、构建入口 |
| 2 | **cann-samples**（CANN 官方实战样例仓） | https://gitcode.com/cann/cann-samples | https://github.com/hicann/cann-samples（master） | A | **源码级**：`rms_norm_quant_story` 的 `0_naive.asc`、`5_ub_utilization.asc` 全文 + README；根 `CMakeLists.txt` 确认支持 `dav-2201` |
| 3 | **op-plugin**（PyTorch NPU 插件，任务书所称 ops-transformer 的现行名称） | — | https://github.com/Ascend/op-plugin（活跃，2026-09 仍在更新） | A | 源码级（轻量）：`AddRmsNormV2KernelNpuOpApi.cpp` 调用链确认 |
| 4 | **kv_rms_norm_rope_cache_story**（cann-samples 子样例） | 同 cann-samples | 同 cann-samples | A | README + 目录级：MemBase→RegBase 迁移样例 |
| 5 | **Ascend/samples**（旧官方样例仓，msopgen 教学样例原址） | — | https://github.com/Ascend/samples（最后推送 2023-11-22） | B | 目录级：确认归一化样例已被清空/迁移，`operator/` 下无 RMSNorm 残留 |

覆盖结论：

1. 任务书所列 `Ascend/cann-learning-hub` 在 GitHub 检索中未发现同名活跃仓库（旧 Ascend/samples 教学样例已并入 cann-samples，见 #5）；现行官方教学/实战样例集为 **cann-samples**（覆盖项 #2）。
2. `Ascend/ops-transformer` 在 GitHub 上已不存在，现行对应仓为 **Ascend/op-plugin**（覆盖项 #3）；其中的归一化算子 kernel 实体在 **ops-nn**（覆盖项 #1），op-plugin 只做 aclnn 桥接。
3. "cann-ops-adv / cann-ops" 生态已统一收敛到 GitCode `cann/ops-nn`（README 与贡献指南均指向 GitCode），GitHub `hicann/ops-nn` 为其官方镜像（版权头 Huawei Technologies Co., Ltd.，README 内所有 PR/Issue 链接指向 `gitcode.com/cann/ops-nn`）。
4. ops-nn 的 `norm/` 目录下与本题相关算子共 **16 个**：`add_rms_norm`、`add_rms_norm_cast`、`add_rms_norm_quant(_v2)`、`add_rms_norm_dynamic_quant(_v2/_mx)`、`multi_add_rms_norm_dynamic_quant`、`inplace_add_rms_norm`、`rms_norm`、`rms_norm_quant(_v2/_v3)`、`rms_norm_grad`、`gemma_rms_norm` 等，另有公共目录 `norm/norm_common`。
5. 本报告达成任务验收线：**3 个仓库做到源码级摘录**（ops-nn、cann-samples、op-plugin），全部结论基于已读取的源码/README 原文，无仅凭仓库名的判断。

---

## 二、实现标准卡片

### 卡片 1：ops-nn `norm/add_rms_norm`（与本题语义最接近的官方算子）

- 仓库/分支：`gitcode.com/cann/ops-nn`（GitHub 镜像 `hicann/ops-nn`，master；`v9.0.0` tag 内容与 master 在核心逻辑上一致，仅格式与 950 支持差异）
- 目录：`norm/add_rms_norm/`
- 语言/形式：Ascend C++，入图算子（`op_kernel` + `op_host` tiling + `op_graph` IR + aclnn 接口 + examples）
- 语义：`x = x1 + x2`；`RmsNorm(x_i) = x_i / Rms(x) * g_i`，`Rms(x)=sqrt(1/n·Σx_i² + eps)`。**与本题 AddRmsNormBias 相比：加法融合 + RMSNorm + gamma 完全一致，仅差最后一步 `+bias`**（本题 bias 对应官方实现里加一条 `Adds(yLocal, yLocal, biasValue, numCol)` 即可）。
- dtype 支持：FLOAT16 / FLOAT32 / BFLOAT16（ND 格式）；产品支持含 "Atlas A2 训练/推理系列"（对应本题 dav-2201/910B）。
- 额外输出：`rstd`（fp32）与 `x`（x1+x2 的结果）——这是入图生态需要，本题无此输出。

#### 1.1 多核切分（按行）

`op_host/add_rms_norm_tiling.cpp`（master，行号按下载副本）：

```cpp
static void CalculateBlockParameters(uint32_t numRow, uint32_t numCore, uint32_t& blockFactor,
                                     uint32_t& latsBlockFactor, uint32_t& useCoreNum)
{
    blockFactor = 1U;
    uint32_t tileNum = Ops::Base::CeilDiv(numRow, numCore * blockFactor);
    blockFactor *= tileNum;                    // 每核行数 = CeilDiv(numRow, numCore)
    useCoreNum = Ops::Base::CeilDiv(numRow, blockFactor);
    latsBlockFactor = numRow - blockFactor * (useCoreNum - 1);  // 尾核行数
}
...
context->SetBlockDim(useCoreNum);
```

kernel 侧（`op_kernel/add_rms_norm.h`，Init 内）：

```cpp
blockIdx_ = GetBlockIdx();
if (blockIdx_ < GetBlockNum() - 1) {
    this->rowWork = this->blockFactor;
} else if (blockIdx_ == GetBlockNum() - 1) {
    this->rowWork = this->numRow - (GetBlockNum() - 1) * this->blockFactor;  // 尾核余数行
}
uint64_t calcOffset = static_cast<uint64_t>(blockIdx_) * this->blockFactor * this->numCol;
```

维度展开（`CalculateRowAndColParameters`）：`numRow` = x1 前 `x1DimNum - gammaDimNum` 维连乘，`numCol` = gamma 的 shapeSize——即"最后一维以外的维度全部展平成 outer 行"，与本题 2D/3D/4D 处理方式一致。支持 1~8 维。

#### 1.2 tiling 模式选择（五种模式）

`op_host/add_rms_norm_tiling.cpp`（v9.0.0 与 master 一致）：

```cpp
constexpr uint32_t UB_FACTOR_B16 = 12288;       // fp16/bf16 单行 UB 因子
constexpr uint32_t UB_FACTOR_B32 = 10240;        // fp32
constexpr uint32_t UB_FACTOR_B16_CUTD = 12096;  // SPLIT_D 模式
constexpr uint32_t UB_FACTOR_B32_CUTD = 9696;
constexpr uint32_t SMALL_REDUCE_NUM = 2000;      // MERGE_N 的 D 上限
...
if (numCol > ubFactor) {                          // ① D 装不下一行 → 按 D 切分
    modeKey = MODE_SPLIT_D;
    ubFactor = (dataType == ge::DT_FLOAT) ? UB_FACTOR_B32_CUTD : UB_FACTOR_B16_CUTD;
    uint32_t colTileNum = Ops::Base::CeilDiv(numCol, ubFactor);
    ubFactor = Ops::Base::CeilDiv(numCol, colTileNum * dataPerBlock) * dataPerBlock;  // 对齐到块
} else if (blockFactor == 1 && soc != ASCEND310P) {   // ② 每核只有一行
    modeKey = MODE_SINGLE_N;
} else if (numColAlign <= SMALL_REDUCE_NUM && soc != ASCEND310P) {  // ③ D 小 → 多行合并进 UB
    modeKey = MODE_MERGE_N;
    rowFactor = ubSize / (numColAlign * weight + 260);   // weight: fp32=24, 其他=18
    ubFactor = rowFactor * numColAlign;
    ...
} else if ((dataType == ge::DT_FLOAT16) && numCol == numColAlign) {  // ④ fp16 且 D 天然对齐
    modeKey = MODE_MULTI_N;
    ...
}
// 否则 ⑤ MODE_NORMAL：一次一行，ubFactor=12288/10240
```

tiling key 编码 = `dtype_key*10 + mode_key + norm_key`（dtype：fp16=1/fp32=2/bf16=3；mode：NORMAL=0/SPLIT_D=1/MERGE_N=2/SINGLE_N=3/MULTI_N=4），kernel 侧用 `TILING_KEY_IS(10/11/12/13/14, 20/..., 30/...)` 分发。

tiling 数据结构（`op_host/add_rms_norm_tiling.h`）核心字段：`num_row / num_col / block_factor / row_factor / ub_factor / epsilon / avg_factor / num_col_align / row_loop / row_tail / mul_loop_fp32 / mul_tail_fp32 / dst_rep_stride_fp32 ...`。

#### 1.3 NORMAL 模式 kernel 主流程（`op_kernel/add_rms_norm.h`）

两遍扫描（先算 rstd，再用 rstd 归一化），每核处理 blockFactor 行、每次 rowFactor 行一批：

```cpp
__aicore__ inline void Process()
{
    CopyInGamma();                                    // gamma 只搬一次（预载）
    LocalTensor<T> gammaLocal = inQueueGamma.DeQue<T>();
    uint32_t i_o_max = RmsNorm::CeilDiv(this->rowWork, this->rowFactor);
    uint32_t row_tail = this->rowWork - (i_o_max - 1) * this->rowFactor;
    for (uint32_t i_o = 0; i_o < i_o_max - 1; i_o++) {
        SubProcess(i_o, this->rowFactor, gammaLocal);   // 满批
    }
    SubProcess(i_o_max - 1, row_tail, gammaLocal);       // 尾批行数单独传
    inQueueGamma.FreeTensor(gammaLocal);
}
```

单遍计算（fp16 分支，`Compute()`）——注意全程 FP32 中间：

```cpp
Mul(sqx, x_fp32, x_fp32, numCol);                    // 平方（FP32）
PipeBarrier<PIPE_V>();
Muls(sqx, sqx, avgFactor, numCol);                  // ×1/n（乘法代替除法）
PipeBarrier<PIPE_V>();
ReduceSumCustom(sqx, sqx, reduce_buf_local, numCol);// 归约（见卡片2）
Adds(sqx, sqx, epsilon, 1);                          // +eps（1 个元素）
Sqrt(sqx, sqx, 1);
Duplicate(reduce_buf_local, ONE, 1);
Div(sqx, reduce_buf_local, sqx, 1);                  // rstd = 1/Rms（取倒数）
// 标量取回的标准事件序列
event_t event_v_s = static_cast<event_t>(GetTPipePtr()->FetchEventID(HardEvent::V_S));
SetFlag<HardEvent::V_S>(event_v_s);  WaitFlag<HardEvent::V_S>(event_v_s);
float rstdValue = sqx.GetValue(0);
event_t event_s_v = static_cast<event_t>(GetTPipePtr()->FetchEventID(HardEvent::S_V));
SetFlag<HardEvent::S_V>(event_s_v);  WaitFlag<HardEvent::S_V>(event_s_v);
Muls(yLocal, xLocal, rstdValue, numCol);            // 用标量 Muls 广播
Mul(yLocal, gammaLocal, yLocal, numCol);             // ×gamma
```

bf16 分支的特殊处理：相加与归一化都升 FP32 计算，`Cast(..., RoundMode::CAST_RINT, ...)` 回 bf16 后再 `Cast` 回 FP32 乘 gamma（官方为对齐 torch 逐步降精度语义）。

residual add 的位置（`CopyIn()`）：x1、x2 分别 `DataCopyCustom` 搬入，half 直接 `Add` 后 `Cast` 到 FP32；bf16 先各自 `Cast` 到 FP32 再 `Add`（bf16 无原生 Add 融合精度考虑）；fp32 直接 `Add`。

缓冲区分配（NORMAL）：

```cpp
Ppipe->InitBuffer(inQueueX, BUFFER_NUM, ubFactor * sizeof(T));       // BUFFER_NUM=1
Ppipe->InitBuffer(inQueueGamma, BUFFER_NUM, ubFactor * sizeof(T));
Ppipe->InitBuffer(outQueueY, BUFFER_NUM, ubFactor * sizeof(T));
Ppipe->InitBuffer(outQueueRstd, BUFFER_NUM, rowFactor * sizeof(float));
if (fp16/bf16) Ppipe->InitBuffer(xFp32Buf, ubFactor * sizeof(float)); // FP32 中间
Ppipe->InitBuffer(sqxBuf, ubFactor * sizeof(float));
Ppipe->InitBuffer(reduceFp32Buf, NUM_PER_REP_FP32 * sizeof(float));  // 64*4B
```

#### 1.4 SPLIT_D 模式（D > UB 单行容量，本题 D 最大 32768 时必经）

`op_kernel/add_rms_norm_split_d.h`：

- 外层按行（rowFactor 行一批），内层把一行切成 `j_max = CeilDiv(numCol, ubFactor)` 段，`col_tail` 为尾段长度；
- **第一遍** `ComputeFormer`：逐段 `CopyInAndAdd`（x1+x2 后 `Cast` FP32）→ `ComputeSum`（平方 → ×avgFactor → `ReduceSumFP32ToBlock(sumLocal[i*8], ...)` 每段归约到每行 8 个 fp32 槽位）；行批结束后 `BlockReduceSumFP32(sumLocal, sumLocal, calc_row_num * 8)` 跨行汇总、`Add` 累进 `rstdLocal`；
- `ComputeRstd`：`+eps → Sqrt → Div` 得每行 rstd（向量，calc_row_num 个）；
- **第二遍** `ComputeLatter`：段级重搬 `x`（ADD/PRE 模式下第一遍已把 `x1+x2` 写回 `xGm`（GM 中转，因 UB 放不下整行））→ `Muls(rstd 标量) → Mul(gamma) → CopyOutY`；
- gamma 也按段 `CopyInGamma(j_idx, num)` 搬入。

```cpp
uint32_t j_max = RmsNorm::CeilDiv(numCol, ubFactor);
uint32_t col_tail = numCol - (j_max - 1) * ubFactor;
for (uint32_t j = 0; j < j_max - 1; j++) { ComputeFormer(i_o, rows, j, rstdLocal, sumLocal, ubFactor); }
ComputeFormer(i_o, rows, j_max - 1, rstdLocal, sumLocal, col_tail);   // 尾段
ComputeRstd(rstdLocal, calc_row_num);
for (uint32_t j = 0; j < j_max - 1; j++) { ComputeLatter(i_o, rows, j, rstdLocal, ubFactor); }
ComputeLatter(i_o, rows, j_max - 1, rstdLocal, col_tail);
```

#### 1.5 MERGE_N / SINGLE_N / MULTI_N（小 D 与极端行数策略）

- **MERGE_N**（`add_rms_norm_merge_n.h`，`numColAlign ≤ 2000`）：一次搬 `rowFactor` 行进 UB，`ReduceSumMultiN` 一次归约出 rowFactor 个 rstd，用 `Brcb` 广播 rstd 到每行、`Mul` 的 repeat 参数（`mulLoopFp32/mulTailFp32/dstRepStride...` 由 tiling 预计算）做"按行乘"（rstd 广播、gamma 跨行复用）。输入/输出队列使用 **DOUBLE_BUFFER_NUM=2 双缓冲**。D 非对齐时 `DataCopyCustom(dst, src, numRow, numCol)` 二维版（220 上 `DataCopyPad` + `blockCount=numRow, blockLen=numCol*sizeof(T)`）。
- **SINGLE_N**（`add_rms_norm_single_n.h`，每核仅 1 行）：单核内手工事件流水（`MTE2_V / V_MTE2 / V_MTE3 / MTE3_V / V_S / S_V` 细粒度 Set/Wait），x1、x2、gamma 的搬运与计算重叠，UB 一次性分块 `Ppipe->InitBuffer(unitBuf, 195584)`（≈192KB-512B）。
- **MULTI_N**（fp16、D 对齐）：行连续排布，整块 `DataCopy` 多行，减少 DMA 次数。

#### 1.6 构建与测试入口

- 仓库根 `build.sh`：`bash build.sh --pkg --soc=${soc_version} --ops=add_rms_norm -j16`（QUICKSTART.md，单算子编译，产出 `cann-ops-nn-custom_*.run`）；分支应选与 CANN 配套的 tag：`git clone -b 9.0.0 https://gitcode.com/cann/ops-nn.git`。
- 算子级 CMake（`norm/add_rms_norm/op_host/CMakeLists.txt`）：`add_modules_sources(... OPTYPE add_rms_norm ACLNNTYPE aclnn_exclude DEPENDENCIES norm_common rms_norm)`——**add_rms_norm 的 op_host 依赖 rms_norm 与 norm_common 模块**。
- aclnn 测试样例：`examples/test_aclnn_add_rms_norm.cpp`；图模式：`op_graph/add_rms_norm_proto.h`。

---

### 卡片 2：ops-nn `norm/rms_norm` 公共件（`rms_norm_base.h` + `reduce_common.h`）

add_rms_norm 全部策略复用该层的搬运与归约工具（`#include "../rms_norm/rms_norm_base.h"`）。

- 仓库/分支：`hicann/ops-nn` master（`v9.0.0` 与 master 仅格式差异，已 diff 确认）
- 文件：`norm/rms_norm/op_kernel/rms_norm_base.h`（294 行）、`norm/rms_norm/op_kernel/reduce_common.h`（181 行）
- 语言：Ascend C++ 设备侧内联函数

#### 2.1 尾块搬运 `DataCopyCustom`（`__CCE_AICORE__ == 220` 即 910B 走 DataCopyPad）

```cpp
template <typename T, typename U, typename R>
__aicore__ inline void DataCopyCustom(const U& dstTensorV1, const R& srcTensor, const uint32_t count)
{
#if (defined(__CCE_AICORE__) && __CCE_AICORE__ == 220) || \
    (defined(__NPU_ARCH__) && (__NPU_ARCH__ == 3003 || __NPU_ARCH__ == 3113))
    DataCopyParams copyParams;
    copyParams.blockLen = count * sizeof(T);     // 任意字节粒度
    copyParams.blockCount = 1;
    if constexpr (is_same<U, AscendC::LocalTensor<T>>::value) {
        DataCopyPadParams padParams;
        DataCopyPad(dstTensorV1, srcTensor, copyParams, padParams);  // GM→UB：Pad 补零
    } else {
        DataCopyPad(dstTensorV1, srcTensor, copyParams);             // UB→GM：截断
    }
#else
    // 非 220 架构：对齐 DataCopy + "重搬最后一个 block + 标量修补"尾块技巧
    int32_t numPerBlock = ONE_BLK_SIZE / sizeof(T);
    if (count % numPerBlock == 0) { DataCopy(dstTensorV1, srcTensor, count); }
    else if (count < numPerBlock) { DataCopy(dstTensorV1, srcTensor, numPerBlock); }
    else {
        int32_t num = count / numPerBlock * numPerBlock;
        DataCopy(dstTensorV1, srcTensor, num);
        // MTE3_S/S_MTE3 事件同步后，把尾段 numPerBlock 个元素逐个 GetValue/SetValue 补齐
        ...
    }
#endif
}
```

另有二维版本（多行非对齐 D）：`DataCopyCustom(dst, src, numRow, numCol)`，220 上用 `DataCopyPad` + `blockCount=numRow, blockLen=numCol*sizeof(T)`。

#### 2.2 归约实现（Add 折叠 + WholeReduceSum 两级）

```cpp
// 单行：先分 repeat 用 Add(src1RepStride=0) 把每 64 元素段累到 work，再 WholeReduceSum
__aicore__ inline void ReduceSumFP32(const LocalTensor<float>& dst, const LocalTensor<float>& src,
                                     const LocalTensor<float>& work, int32_t count)
{
    uint64_t mask = NUM_PER_REP_FP32;                        // 64
    int32_t repeatTimes = count / NUM_PER_REP_FP32;
    int32_t tailCount = count % NUM_PER_REP_FP32;
    BinaryRepeatParams repeatParams;
    repeatParams.src0RepStride = ONE_REPEAT_BYTE_SIZE / ONE_BLK_SIZE;
    repeatParams.src0BlkStride = 1;
    repeatParams.src1RepStride = 0;   repeatParams.src1BlkStride = 1;   // 广播同一 work
    repeatParams.dstRepStride = 0;   repeatParams.dstBlkStride = 1;
    Duplicate(work, ZERO, NUM_PER_REP_FP32);
    if (likely(repeatTimes > 0)) { Add(work, src, work, mask, repeatTimes, repeatParams); }
    if (unlikely(tailCount != 0)) { Add(work, src[bodyCount], work, tailCount, 1, repeatParams); }  // 尾段
    AscendCUtils::SetMask<float>(NUM_PER_REP_FP32);
#if defined(__CCE_AICORE__) && __CCE_AICORE__ == 220
    if (g_coreType == AIV) {
        WholeReduceSum<float, false>(dst, work, MASK_PLACEHOLDER, 1, 0, 1, 0);   // 220/AIV 参数
    }
#else
    WholeReduceSum<float, false>(dst, work, MASK_PLACEHOLDER, 1, 1, 1, DEFAULT_REPEAT_STRIDE);
#endif
}
```

多行版 `ReduceSumMultiN`（`reduce_common.h`）：`(N,D)→(N,1)`，`repeat=N`、`repStride=numColAlign/8`，同样先 `Add` 折叠到 `(N,64)` 临时区再 `WholeReduceSum(dst, tmp, elemNum=64, repeat=N, ...)`；`repeat>255` 时分段。注意其约束注释：`require D < 255 * 8`（单段 repeat 上限 255）。

SPLIT_D 用的 `ReduceSumFP32ToBlock`：Add 折叠后接 `BlockReduceSum(dst, work, 1, mask, 1, 1, DEFAULT_REPEAT_STRIDE)`，把每段归约成 8 个 fp32 槽位供跨段累加。

#### 2.3 常量与缓冲约定

```cpp
constexpr int32_t BUFFER_NUM = 1;         // 主模板单缓冲
constexpr int32_t DOUBLE_BUFFER_NUM = 2;  // MERGE_N 用
constexpr int32_t NUM_PER_REP_FP32 = 64;  // 256B/4B
constexpr int32_t NUM_PER_BLK_FP32 = 8;
constexpr int32_t BLOCK_SIZE = 32;         // 32B 对齐
```

`norm/rms_norm` 目录另有 `rms_norm_whole_reduce_sum.h`（910 上多行 WholeReduceSum 直归约变体）、`rms_norm_single_row.h`、`rms_norm_split_d.h` 等，结构与 add_rms_norm 对应版本同构；`rms_norm` 本体 README 产品支持含 Atlas A2（√），约束说明为"无"。

---

### 卡片 3：cann-samples `Samples/2_Performance/rms_norm_quant_story`（官方 RMSNorm 直调渐进优化案例）

- 仓库/分支：`gitcode.com/cann/cann-samples`（GitHub 镜像 `hicann/cann-samples`，master）
- 目录：`Samples/2_Performance/rms_norm_quant_story/`（README.md + Story.md + src/7 个版本 + scripts/gen_data.py + CMakeLists.txt）
- 形式：**单文件 `.asc` 直调样例**（kernel + host main + 数据生成 + golden 校验同文件），核函数签名 `__global__ __aicore__ __vector__ void rms_norm_quant(...)`——**与本题 kernel.asc 直调模式同构**
- 平台说明：README 主测平台为 **Ascend 950PR/950DT（64 Vector Core，dav-3510）**，规格 `x[4096,8192] fp16 → y int8`；但仓库根 CMakeLists `set(VALID_NPU_ARCHS dav-3510 dav-2201)` **明确支持 dav-2201**（用 `-DNPU_ARCH=dav-2201` 配置即可在本题 SoC 编译）
- 优化链路：7693 us → 49.0 us（157x），步骤：naive → gamma 预载 → 多核（64 核按行切分）→ VF MicroAPI → 双缓冲 → UB 多行 → 二分累加

#### 3.1 `0_naive.asc`（标准 MemBase 写法基线，460 行）

Tiling 结构与缓冲：

```cpp
struct RmsnormQuantTilingData { int64_t a; int64_t r; float epsilon; };
static constexpr size_t BUF_NUM = 1;
static constexpr int64_t BLOCK_BYTES = 32;
// TQue<TPosition::VECIN/VECOUT> + TBuf<VECCALC>：xInQueue_/gammaInQueue_/yOutQueue_/xBuf_/gammaBuf_/rmsBuf_/reduceBuf_
```

`CopyInX` 的 DataCopyPad 尾块写法（D 非 32B 倍数安全）：

```cpp
AscendC::DataCopyExtParams dataCopyParams;
dataCopyParams.blockCount = 1;
dataCopyParams.blockLen = tilingData_->r * sizeof(DATA_TYPE);   // 字节粒度
dataCopyParams.srcStride = 0;  dataCopyParams.dstStride = 0;
AscendC::DataCopyPadExtParams dataCopyPadParams{false, 0, 0, static_cast<DATA_TYPE>(0)};
AscendC::DataCopyPad(xInLocalTensor, xGm_[loop * tilingData_->r], dataCopyParams, dataCopyPadParams);
```

`Compute()`（标准 API，FP32 中间）：

```cpp
Cast(gammaLocalTensor, gammaInLocalTensor, CAST_NONE, r);
Cast(xLocalTensor, xInLocalTensor, CAST_NONE, r);
Mul(rmsLocalTensor, xLocalTensor, xLocalTensor, r);
ReduceSum(reduceLocalTensor, rmsLocalTensor, xInLocalTensor.ReinterpretCast<float>(), r);
Duplicate(rmsLocalTensor, reduceLocalTensor, r);     // rstd 广播成向量
Muls(rmsLocalTensor, rmsLocalTensor, rInv_, r);      // ×1/r
Adds(rmsLocalTensor, rmsLocalTensor, epsilon, r);
Sqrt(rmsLocalTensor, rmsLocalTensor, r);
Div(xLocalTensor, xLocalTensor, rmsLocalTensor, r);   // 向量除（naive 用向量 Div，未取标量）
Mul(...); Muls(..., scale_); Adds(..., offset_); Cast(yLocalTensor, ..., CAST_RINT, r);
```

#### 3.2 `5_ub_utilization.asc`（UB 空间模型 + 多行 DMA，621 行）

host 侧 tiling（读取真实平台参数）：

```cpp
auto ascendcPlatform = platform_ascendc::PlatformAscendCManager::GetInstance();
ascendcPlatform->GetCoreMemSize(platform_ascendc::CoreMemType::UB, ubSize);
int64_t coreNum = ascendcPlatform->GetCoreNumAiv();
size_t blockFactor = (a + coreNum - 1) / coreNum;
size_t blockTail   = a - blockFactor * (blockNum - 1);
int64_t maxUbFactor = calcMaxUbFactor(r, ubSize);      // 见下
```

UB 空间模型（`calcMaxUbFactor`，注释即文档）：

```cpp
int64_t rAlign = (r + BLOCK_BYTES - 1) / BLOCK_BYTES * BLOCK_BYTES;   // 行对齐
int64_t fixedSize  = rAlign * (sizeof(dataType) + sizeof(float)) + BLOCK_BYTES;  // gamma 两份 + 对齐余量
int64_t linearCoef = rAlign * (sizeof(dataType)*BUF_NUM + sizeof(outputType)*BUF_NUM) + sizeof(float);
int64_t maxUbFactor = (ubSize - fixedSize) / linearCoef;              // 一次装多少行
```

kernel 侧多核 + 多行 + 双缓冲 + 尾行：

```cpp
if (blockIdx_ == AscendC::GetBlockNum() - 1) { curBlockFactor_ = tilingData_->blockTail; }
curUbLoops_  = CeilDiv(curBlockFactor_, ubFactor_);
ubFactorTail_ = curBlockFactor_ - (curUbLoops_ - 1) * ubFactor_;     // 尾批行数
for (int64_t loop = 0; loop < curUbLoops_; loop++) {
    int64_t ubFactor = loop == (curUbLoops_ - 1) ? ubFactorTail_ : tilingData_->ubFactor;
    CopyInX(loop, ubFactor);  Compute(ubFactor);  CopyOut(loop, ubFactor);
}
```

多行一次 DMA + UB 内行间对齐 padding（**D 非 32B 倍数的多行搬运标准解法**）：

```cpp
// CopyInX：GM→UB，一次搬 ubFactor 行
dataCopyParams.blockCount = ubFactor;                       // 多行合并为一次 DMA
dataCopyParams.blockLen   = tilingData_->r * sizeof(DATA_TYPE);
dataCopyParams.srcStride   = 0;                              // GM 行间无 padding（行紧邻）
dataCopyParams.dstStride   = (rAlign_ - tilingData_->r) * sizeof(DATA_TYPE) / BLOCK_BYTES;  // UB 行间补齐
AscendC::DataCopyPad(xInLocalTensor, xGm_[...], dataCopyParams, dataCopyPadParams);
// CopyOut：UB→GM 反向，srcStride 跳过 UB padding，dstStride=0（GM 紧邻写，不越界）
```

#### 3.3 优化链路关键数据（README 原文）

| Step | 动作 | Duration | 加速比 | 关键观测 |
|---|---|---|---|---|
| 0 | naive | 7693 us | 1.0x | 单核；RVECLD+RVECST 是 RVECEX 的 1.72x（中间结果反复落 UB） |
| 1 | gamma 预载（循环外一次） | 6790 us | 1.13x | 删掉 4095 次重复搬运 |
| 2 | 多核按行切分（64 核） | 113.6 us | 59.8x | 各 ratio 不变，纯并行收益；并行效率 93.4% |
| 3 | VF MicroAPI（寄存器驻留） | 84.1 us | 1.35x | RVECST -85.5%；MTE2 接近新瓶颈时才值得开双缓冲 |
| 4 | 双缓冲 BUF_NUM 1→2 | 54.3 us | 1.55x | **指令数不变、时延 -35%**；vec+mte2=166% 真重叠 |
| 5 | UB 多行（一次 DMA 多行） | 49.0 us | 1.11x | MTE2 次数 -71%、SCALAR -56%（固定成本摊薄） |
| 6 | 二分累加 + Halley rsqrt | 49.5 us | ≈1.0x | 精度收益为主 |

（注：以上为 950PR 上的官方实测数据，dav-2201 上数值会不同，仅方法论可迁移。）

---

### 卡片 4：cann-samples `Samples/2_Performance/kv_rms_norm_rope_cache_story`（README 级）

- 仓库/分支：同 cann-samples master；目录含 `membase/full_load.asc`、`regbase/full_load.asc`、`include/sample_common.h`、`scripts/gen_data.py`、`CMakeLists.txt`。
- 语义：`kv[..., :Dv]` 段做 RMSNorm → v_cache；`kv[..., Dv:]` 做 RoPE → k_cache（含 gamma）。
- 平台：950PR/950DT（`-DNPU_ARCH=dav-3510`），BF16，固定 AIV Vector kernel。
- 构建入口（README 原文）：`cmake -S . -B build -DNPU_ARCH=dav-3510 && cmake --build build --target kv_rms_norm_rope_cache_story`，运行可执行文件自动生成数据并 golden 校验输出 `PASS`。
- 参考价值：README 明确 MemBase→RegBase 迁移原则——"保留 MemBase 已验证的数据流（DataCopyPad 搬运、LocalTensor/TQue/TPipe staging），只替换 Vector 计算部分（RegTensor + `__simd_vf__` + MaskReg 尾控制）"。对本题 dav-2201 + CANN 9.0.0 的定位是 **MemBase 标准 API 路线**（RegBase/VF 是 950 代际能力，220 上可用性未验证）。

---

### 卡片 5：Ascend/op-plugin 中 AddRmsNorm 桥接（调用链证据）

- 仓库/分支：`github.com/Ascend/op-plugin` master（活跃维护）
- 文件：`op_plugin/ops/opapi/AddRmsNormV2KernelNpuOpApi.cpp`
- 内容：`EXEC_NPU_CMD(aclnnInplaceAddRmsNorm, x1, x2, gamma, epsilon, y);`——torch_npu 的 `npu_add_rms_norm` 最终落到 ops-nn 的 `InplaceAddRmsNorm` 算子（与 `AddRmsNorm` **共用同一 kernel 与同一 tiling**：`IMPL_OP_OPTILING(InplaceAddRmsNorm).Tiling(Tiling4AddRmsNorm)`，见 `add_rms_norm_tiling.cpp` 末尾注册）。
- 同目录相关文件：`RmsNormKernelOpApi.cpp`、`RmsNormQuantKernelOpApi(V2).cpp`、`GemmaRmsNormKernelOpApi.cpp`、`AddRmsNormQuantKernelOpApi.cpp`、`QkvRmsNormRopeCacheNpuOpApi.cpp` 等。
- 结论：本题可参考的 kernel 实体唯一来源就是 ops-nn 的 `add_rms_norm` 家族；op-plugin 层对 kernel 写法无增量参考价值。

---

### 卡片 6：ops-nn `norm/gemma_rms_norm`（公式变体对照）

- README（已读）：`GemmaRmsNorm(x_i) = x_i / Rms(x) * (1 + g_i)`——与标准 RMSNorm 的差异仅在 gamma 先 +1。产品支持含 Atlas A2（√）。
- 对本题的启示：`gamma`、`bias` 都是长度 D 的一维向量，官方实现里 gamma 的载入/复用/按行乘法写法（MERGE_N 的 `repeatByRow`、NORMAL 的 `Mul(yLocal, gammaLocal, yLocal, numCol)`）对 bias 同样适用；bias 只是在 gamma 乘法之后追加一条 `Add(yLocal, biasLocal, yLocal, numCol)`（向量加）或 `Adds`（标量），不改变任何搬运结构。

---

## 三、与本题（AddRmsNormBias 直调模式）的差异表

| 维度 | ops-nn add_rms_norm（官方入图） | cann-samples rms_norm_quant_story（官方直调） | 本题 AddRmsNormBias 直调 |
|---|---|---|---|
| 形态 | op_kernel + op_host + op_graph + aclnn，`GET_TILING_DATA` / `TILING_KEY_IS` 分发 | 单 `.asc` 文件：`__global__ __aicore__ __vector__` 核函数 + host main（aclInit/aclrt*），tiling 结构体值传递 | 同左（直调单文件 kernel.asc） |
| tiling 产生方 | op_host Tiling4AddRmsNorm（运行时读平台 UB/核数） | host main 里 `PlatformAscendCManager` + `calcTiling` | 本地实现自定（可沿用 cann-samples 模式） |
| 语义 | x1+x2 → RMSNorm → ×gamma，**无 bias**；额外输出 rstd 与 x=x1+x2 | RMSNorm → ×gamma → ×scale +offset 量化 int8，**无 residual add、无 bias 向量**（offset 是标量） | x+residual → RMSNorm → ×gamma → **+bias**；仅一个输出 |
| 多核 | SetBlockDim(useCoreNum)，行级切分 + 尾核 `latsBlockFactor` | GetBlockNum/GetBlockIdx，blockFactor + blockTail | 行级切分（同思路） |
| D 范围 | 模式覆盖任意 D：>UB 走 SPLIT_D 两遍 + GM 中转 x；≤2000 走 MERGE_N | r=8192 单行可整装 UB，无 SPLIT_D 分支 | D∈[64,32768]：D>12288（fp16）必须处理超 UB 行 |
| 尾块/对齐 | `DataCopyCustom`（220 上 DataCopyPad）；非 220 用对齐拷贝+标量修补 | `DataCopyExtParams + DataCopyPad(Ext)Params`，多行 `blockCount` + `dstStride` padding | 同思路（DataCopyPad） |
| 归约 | `ReduceSumCustom`（Add 折叠 + WholeReduceSum）+ `ReduceSumMultiN`/`BlockReduceSum` 变体 | 标准 `ReduceSum` + `Duplicate` 广播（naive），VF 版寄存器累加 | 需自选（官方主推两级折叠） |
| 中间精度 | fp16/bf16 输入一律 Cast FP32 计算平方/归约/rstd，输出前 Cast 回 | 同（FP32 中间） | 约定一致 |
| 流水 | 主模板 BUFFER_NUM=1；MERGE_N 双缓冲；SINGLE_N 手工事件流水 | BUF_NUM 可 1/2，实测双缓冲 -35% | 本地两遍扫描实现 |
| 平台 | A2/A3/950 等（`__CCE_AICORE__==220` 有专用分支） | CMake 支持 dav-2201；实测数据来自 950PR | dav-2201 + CANN 9.0.0 |
| 测试 | `tests/` + `examples/test_aclnn_add_rms_norm.cpp` | `.asc` 内置 golden 校验（gen_data.py 生成数据） | 判题平台 15 测试点 |

---

## 四、可迁移结论、差异与未验证事项

### 4.1 可迁移结论（均来自源码级阅读）

1. **语义对齐度高**：官方 `AddRmsNorm` 与本题只差最后 `+bias`；在 NORMAL/SPLIT_D/MERGE_N 任一策略的输出段 `Mul(yLocal, gammaLocal, yLocal, numCol)` 之后追加 `Add(yLocal, biasLocal, yLocal, numCol)` 即闭合语义，搬运结构不变（bias 与 gamma 同形状、同生命周期，可同样"预载一次、循环复用"）。
2. **模式选择阈值可照搬**（fp16/bf16 基准）：D > 12288 → 两遍扫描/切 D（本题 D 上限 32768 落在此区间）；D ≤ 2000 且行多 → 多行合并进 UB（减少 DMA 次数与 scalar 开销，cann-samples Step5 实测 MTE2 次数 -71%）；每核仅 1 行 → 单核单行 + 手工事件流水。D∈(2000,12288] → 一次一行（ubFactor 上限内）。
3. **多核切分公式**：`blockFactor = CeilDiv(numRow, coreNum)`、尾核 `numRow - blockFactor*(useCoreNum-1)`、kernel 侧 `rowWork` 按 blockIdx 分三种情况；题面 2D/3D/4D 展平即官方 `CalculateRowAndColParameters` 的做法（前 n-1 维连乘为行数）。
4. **尾块搬运**：dav-2201 上官方统一走 `DataCopyPad`（`DataCopyParams{blockLen=count*sizeof(T)}`，UB→GM 变体无 Pad 参数即截断）；多行搬运用 `blockCount=行数 + dstStride=(rAlign-r)*sizeof(T)/32` 在 UB 侧补行间 padding、GM 侧紧邻写——保证 D 非 32 倍数时不覆盖相邻行。
5. **归约写法**：先 `Muls(avgFactor)` 再 `Add 折叠到 64 槽 + WholeReduceSum`（220/AIV 参数形态 `(MASK_PLACEHOLDER,1,0,1,0)` 且需 `g_coreType == AIV` 判断）；rstd 用 `1/(sqrt(sum+eps))` 取倒数后经 `V_S/S_V` 事件 `GetValue` 取标量、`Muls` 广播——比 `Duplicate` 向量广播（naive 写法）少一条向量链。
6. **FP32 中间精度**：官方在 fp16/bf16 下平方/归约/rstd 全 FP32，bf16 输出用 `CAST_RINT`、fp16 用 `CAST_NONE`；与本项目"归约及中间累加优先 FP32"的约定一致，bf16 精度风险点在输出舍入模式的选择。
7. **gamma/bias 预载 + 双缓冲 + UB 多行**是官方实证的三大收益（1.13x / 1.55x / 1.11x，950PR 数据）；双缓冲在"VEC 与 MTE2 接近平衡"时收益最大。
8. **SPLIT_D 大 D 方案**：第一遍逐段算平方和（每段归约到 8 fp32 槽）、`+eps→Sqrt→Div` 得每行 rstd，第二遍重搬数据（ADD 模式官方把 x1+x2 写回 GM 再读回，因为整行放不下 UB）乘 rstd、乘 gamma；本题两遍扫描若整行能驻留 UB 则无需 GM 中转。
9. **直调 host 侧 tiling**：cann-samples 的 `PlatformAscendCManager::GetInstance()->GetCoreMemSize(UB)/GetCoreNumAiv()` + UB 空间模型（fixed + linear×ubFactor）是直调模式读取真实平台参数的标准写法。

### 4.2 与本题的差异（必须注意）

1. 官方 add_rms_norm 是入图算子（tiling 由 op_host 生成、TILING_KEY 分发、workspace 16MB+256B），直调模式无法照搬分发框架，只能搬 kernel 内计算/搬运结构；tiling 需在 host main 自算（cann-samples 模式）。
2. 官方多输出 rstd/x 在本题不存在，相关 `outQueueRstd`/`xGm` 逻辑应删除。
3. 官方 `getPerformanceFlag` 等针对 ASCEND910B 的特定 shape 走 MERGE_N 性能分支（num_col≤5120、外层维≤512 等），说明 910B 上小 D 多行合并是被官方认可的优化方向。
4. rms_norm_quant_story 的实测数据来自 950PR（64 AIV）；dav-2201 的 AIV 数、UB 大小（`__CCE_AICORE__==220` 分支常量如 195584 的 UB 假设）不同，数值不可直接引用。
5. 950 代际的 VF MicroAPI / RegBase 写法（`__simd_vf__`、RegTensor、MaskReg）在 dav-2201 + CANN 9.0.0 的可用性未在官方材料中确认；本题应走 MemBase 标准 API 路线（与官方 220 分支代码一致）。
6. arch35（`add_rms_norm_tiling_arch35.cpp`、`op_kernel/arch35/`）为 950 服务，与本题无关（已确认存在但未深读）。

### 4.3 未验证事项

1. 全部代码摘录仅经静态阅读，未在 dav-2201/CANN 9.0.0 真机编译或精度/性能验证（本机 macOS 无 NPU）。
2. `WholeReduceSum` 在 220 上的精确签名与 MASK_PLACEHOLDER 语义、`Brcb`/`BlockReduceSum` 的可用性——归 Agent 2 的 API 签名核对，本报告仅记录官方 220 分支的调用形态。
3. MULTI_N 模式的 rowFactor 公式按 fp16 假设推导，bf16/fp32 下该分支官方未启用（tiling 只对 fp16 选 MULTI_N）。
4. GitCode `cann/ops-nn` 页面本身未直接抓取成功（explore 页 404），其内容经 GitHub 官方镜像 + GitCode PR 链接交叉确认；两仓同步性以华为官方同步机制为准。
5. `Ascend/samples` 旧仓 operator 目录已空，无法考证历史 msopgen RMSNorm 教学样例（已被迁移/下线）。
6. cann-samples 各 `.asc` 在 dav-2201 上的实际编译表现未验证（CMake 白名单已含 dav-2201，但样例注释以 dav-3510 为主测）。

---

## 五、来源登记

| # | URL | 标题/内容 | 仓库与分支/commit | 访问日期 | 用途 | 证据状态 | 证据等级 |
|---|-----|-----------|--------------------|----------|------|----------|----------|
| 1 | https://github.com/hicann/ops-nn | ops-nn（CANN 神经网络算子库，GitCode cann/ops-nn 官方 GitHub 镜像） | master（2026-09 快照，经 API） | 2026-09-12 | add_rms_norm/rms_norm/gemma_rms_norm 源码 | verified（读取 README 与多个源码原文） | A |
| 2 | https://github.com/hicann/ops-nn/blob/v9.0.0/norm/add_rms_norm/ | AddRmsNorm v9.0.0 tag 源码 | tag `v9.0.0` | 2026-09-12 | 确认 9.0.0 版本与 master 核心逻辑一致 | verified（diff 基类仅格式差异） | A |
| 3 | https://gitcode.com/cann/ops-nn | ops-nn 官方主仓（GitCode） | 分支 9.0.0 / master | 2026-09-12 | 官方下载入口 `git clone -b 9.0.0`；PR/讨论链接 | partial（经 GitHub 镜像 README 内链确认，页面未直接抓通） | A |
| 4 | https://github.com/hicann/ops-nn/blob/master/norm/add_rms_norm/op_kernel/add_rms_norm.cpp 等 | add_rms_norm 主入口/五种策略/tiling 全套源码 | master | 2026-09-12 | tiling、尾块、归约、多核、流水摘录 | verified（全文读取 .cpp/.h×5/tiling×2） | A |
| 5 | https://github.com/hicann/ops-nn/blob/master/norm/rms_norm/op_kernel/rms_norm_base.h 与 reduce_common.h | RMSNorm 公共基类与归约件 | master | 2026-09-12 | DataCopyCustom/ReduceSum 系列摘录 | verified（全文读取） | A |
| 6 | https://github.com/hicann/cann-samples | CANN-SAMPLES（官方实战样例仓） | master | 2026-09-12 | 样例覆盖与构建 | verified（README + 目录 + CMakeLists） | A |
| 7 | https://github.com/hicann/cann-samples/blob/master/Samples/2_Performance/rms_norm_quant_story/ | RmsNormQuant 算子性能优化指南（story） | master | 2026-09-12 | 直调 .asc 渐进优化全链路 | verified（README 全文 + 0/5 源码全文） | A |
| 8 | https://github.com/hicann/cann-samples/blob/master/Samples/2_Performance/kv_rms_norm_rope_cache_story/README.md | KvRmsNormRopeCache MemBase→RegBase 样例 | master | 2026-09-12 | MemBase/RegBase 边界与构建命令 | verified（README 全文） | A |
| 9 | https://github.com/Ascend/op-plugin | op-plugin（torch_npu 插件，ops-transformer 现名） | master | 2026-09-12 | AddRmsNormV2→aclnnInplaceAddRmsNorm 调用链 | verified（读取桥接源码与目录） | A |
| 10 | https://github.com/Ascend/samples | Ascend/samples（旧官方样例仓） | master（2023-11-22 后停更） | 2026-09-12 | 确认旧 msopgen 样例已迁移 | verified（目录级检查，operator/ 无归一化样例） | B |
| 11 | https://github.com/hicann/ops-nn/blob/master/norm/gemma_rms_norm/README.md | GemmaRmsNorm 算子说明 | master | 2026-09-12 | 公式变体对照（gamma+1） | verified | A |
| 12 | https://github.com/hicann/ops-nn/blob/master/docs/QUICKSTART.md | ops-nn 快速入门（构建命令） | master | 2026-09-12 | `build.sh --pkg --soc= --ops=` 构建入口 | verified | A |
| 13 | 搜索引擎结果页（CSDN gitblog 对 ops-nn InplaceAddRmsNorm 的转载、hicann/ops-nn op_list.md 等） | 辅助定位 | — | 2026-09-12 | 仓库/算子发现线索 | partial（仅摘要，结论均回到官方源码核实） | C |

> 证据等级说明：A=官方仓库/官方样例原文；B=官方旧仓目录级确认；C=社区转载（仅用于发现，未作为结论依据）。`verified`=已读取原始内容；`partial`=仅摘要/README 级。

---

## 附：本报告未做的事（边界声明）

- 未研究任何非 Ascend 仓库（GPU 实现归 Agent 4）。
- 未做 CANN 9.0.0 API 签名逐一核对（归 Agent 2）；本报告中 API 用法摘录自官方 220 分支源码，具体签名以 Agent 2 结论为准。
- 未克隆仓库到项目目录；临时下载文件仅存放于系统 `/tmp`（不属项目工程），报告为本任务唯一落盘产物。
- 未执行 CANNJudge 上传或任何外部提交。
