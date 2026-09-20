# Agent 5：编译器 / IR / 算子生成路线参考价值评估

> 日期：2026-09-11（资料访问日）
> 题目：AddRmsNormBias（CANN 9.0.0 / 直调模板 kernel.asc）
> 任务边界：本轮不实现，只判断自动生成/编译器路线的参考价值与不可用边界。
> 本机无 CANN、无 Ascend C 编译器、无昇腾 NPU；下列结论均为公开资料与本地静态核对，不含真机验证。

---

## 1. 路线清单

| # | 路线 | 平台/后端 | 与本题的关系 | 证据等级 |
| --- | --- | --- | --- | --- |
| R1 | Apache TVM / TensorIR | CPU/GPU（LLVM/CUDA），无官方 Ascend 后端 | reduction lowering 的标准范式可借鉴 | A（官方文档已读） |
| R2 | MLIR Linalg | 多后端中间层 | iterator types（parallel/reduction）、parametric tiling、promotion 思想可借鉴 | A（官方文档已读） |
| R3 | IREE | CPU/GPU（Vulkan/ROCm/CUDA/Metal/AMD AIE） | support matrix **无昇腾/NPU**；dispatch 分块思想仅作概念参考 | A（官方首页已读） |
| R4 | OpenXLA / StableHLO | XLA/TPU/GPU | `stablehlo.reduce` 等高层语义；**无昇腾后端** | A（spec 已读） |
| R5 | Triton | NVIDIA GPU 为主 | LayerNorm 教程 = 本题最接近的 GPU 融合范式 | A（官方教程已读） |
| R6 | TorchInductor | CPU/CUDA | reduction split / persistent / 融合策略；生成 Triton 而非 Ascend C | B（PyTorch 2.x 页面已读，细节页部分访问受限） |
| R7 | TileLang / TileIR（microsoft） | NVIDIA/AMD GPU（README 明列） | tile 编程模型可借鉴；**公开仓库未声明 Ascend 支持** | A（GitHub README 已读） |
| R8 | **PyPTO**（CANN org） | **昇腾 NPU**（PTO 虚拟指令 → 可执行代码） | 唯一直接面向 Ascend 的自动/半自动算子框架；与本题提交格式不兼容 | A（官方 README+文档已读） |
| R9 | CANN msopgen / 算子工具链 | 昇腾（本机 `源码/` 即其产物） | 生成的是 **op_host+op_kernel 算子包**，与判题 **直调 kernel.asc** 模板是两套工程 | A（本地已读） |

---

## 2. 可借鉴的 tiling / memory planning 思想

以下思想**可以**迁移到手工 Ascend C 写法，不需要引入任何自动生成工具。

### 2.1 Reduction 分解：init 与 update 分离（TVM）

TVM TensorIR 的 `decompose_reduction` 把

```text
for k:
    Y = Y + A[k]*B[k]   # init 与 update 揉在一起
```

拆成

```text
Y_init:    Y = 0
Y_update:  for k: Y = Y + ...
```

对应到本题：每行先 `sum = 0`（FP32），再分块累加 `sum += tile^2`，与官方 `ReduceSum` 的「先清 dst 再 reduce」语义一致。手工实现时把「初始化」与「分块累加」写成清晰两段，便于核对 UB 生命周期与 workLocal 布局。

### 2.2 沿归约维分块 + 尾块 mask（Triton LayerNorm）

Triton 官方 LayerNorm 教程（结构与 RMSNorm 同构）的骨架：

```text
row = program_id(0)                  # 一程序一行
for off in range(0, N, BLOCK_SIZE):  # 沿最后一维分块
    cols = off + arange(BLOCK_SIZE)
    a = load(X+cols, mask=cols<N, other=0).to(fp32)
    partial += a
mean/partial = sum(partial) / N
# 第二遍再 load 一遍做归一化（两遍扫描）
y = (x-mean)*rstd * w + b
store(Y+cols, y, mask=cols<N)
```

可直接映射到 Ascend C：

| Triton 概念 | Ascend C 对应 |
| --- | --- |
| `program_id(0)` = 行 | `GetBlockIdx()` / 按 outer 行切分 |
| `BLOCK_SIZE` 分块循环 | UB 容量决定的 tile 长度（受 ReduceSum count 上限约束） |
| `mask=cols<N, other=0` | `DataCopyPad` 搬入补 0（补 0 不影响平方和） |
| `.to(tl.float32)` 累加 | 归约链强制 FP32 |
| 两遍扫描 | 方案 1 两遍扫描（Pass1 归约 / Pass2 归一化） |
| 整行放不下时仍可 loop | 大 D（> UB）分多 tile，块间标量合并 |

教程还明确：整行特征 ≤64KB 才走单遍 fused；否则需改策略——与本题「D≤4096 才考虑单遍暂存」的分界思路同构。

### 2.3 Producer–Consumer 融合位置（TVM `reverse_compute_at`）

TVM 把 elementwise 块 `C` 挂到归约块 `Y` 的外层 tile 循环下，使中间结果只在片上短暂存活。对应本题单遍路线：`y` 只在 UB 存活、算完 rms 后立即消费，不写回 GM；两遍路线则等价于「不做 compute_at」，中间量必须落 GM。

**判断**：这不是工具问题，是算法选择问题。用 compute_at 思想自问「这个中间张量有没有必要落 GM」，即可决定单遍/两遍。

### 2.4 按容量反推 tile（MLIR Promotion / Triton MAX_FUSED_SIZE）

MLIR Linalg 的 key transformation 明确包含 *Promotion to Temporary Buffer in Fast Memory* 与 *Parametric Tiling*：先把数据晋升到快速内存的临时缓冲，再按参数化 tile 迭代。

对应手工公式（本题）：

```text
tile 元素数 ≤ UB可用字节 / (工作集字节/元素)
两遍：工作集 ≈ x_fp32 + residual_fp32 + out(原dtype) + gamma/bias 复用
单遍：再加 y_fp32 暂存
再加 ReduceSum 的 dst/workLocal 预留
```

把「UB 容量」当成 fast memory budget 反推 tile，而不是先拍一个 2048/4096 再祈祷能装下。

### 2.5 Parallel vs Reduction 维度显式化（MLIR iterator_types）

MLIR 要求每个 loop 显式标成 `parallel` 或 `reduction`。本题：

- `outer` 维（batch*seq*heads）= **parallel** → 按行分给 AI Core
- `D` 维 = **reduction** → 只能在单核内串行/分块归约，不能跨核无通信切分

这直接否定「把 D 切开多核各算一部分再跨核合并」的天真方案（除非引入跨核同步与二次归约，复杂度远超收益）；并支持方案 7「按行分配 AI Core」作为首选多核策略。

### 2.6 PyPTO 的 32 字节 tile 对齐约束（昇腾侧旁证）

PyPTO 官方 FAQ《set_xxx_tile_shapes最后一维未32字节对齐》原文：

> 硬件指令限制处理的数据需要 32 字节对齐。
> `TileShape[-1] * sizeof(dtype) % 32 == 0`

这是昇腾硬件层面的公开确认，与本题文档中 DataCopyPad/尾块 32B 风险同源。自动生成框架也逃不开该约束——手工实现时更应把对齐写进 tiling 计算，而不是等运行时报错。

---

## 3. 哪些自动生成方案不能直接用于 CANNJudge 提交

判题接口（模板 A 级已读）要求：

```text
kernel.asc
  extern "C" void run_kernel(GM_ADDR x, const TensorGroupInfo& info_x, ...,
                             int64_t availableCoreNum, aclrtStream stream, float epsilon)
  // 内部 add_rms_norm_bias_custom<<<blockNum,nullptr,stream>>>(...)
  // 入口 __global__ __vector__，#include "kernel_operator.h"
```

CMake：`find_package(ASC)`，`LANGUAGES ASC CXX`，`SOC_ARCH=dav-2201`。

| 方案 | 不可直接提交的原因 | 格式 | 平台 | 合规 |
| --- | --- | --- | --- | --- |
| TVM / TensorIR | 无 Ascend 后端；产物是 LLVM/CUDA module，不是 `.asc` | ✗ | ✗ | — |
| MLIR Linalg | 中间层，本身不产 Ascend C；需自写 lowering，工作量等同重写 | ✗ | ✗ | — |
| IREE | support matrix 无昇腾；产物 SPIR-V/hsa/PTX | ✗ | ✗ | — |
| OpenXLA/StableHLO | 高层 op 集，后端是 XLA；无 Ascend codegen | ✗ | ✗ | — |
| Triton | 生成 CUDA/ROCm；即便有第三方 Ascend 移植也非判题模板格式 | ✗ | ✗ | — |
| TorchInductor | 生成 Triton/C++/C-shim，非 Ascend C 直调入口 | ✗ | ✗ | — |
| TileLang/TileIR | README 测试设备仅 NVIDIA/AMD；无公开 Ascend 后端 | ✗ | ✗ | — |
| **PyPTO** | 面向昇腾，但编译链是 Tensor Graph→Tile/Block/Execution Graph→**PTO 虚拟指令→可执行代码**；不是 `run_kernel` + `__global__ __vector__` 的 `kernel.asc` 文本；且依赖 CANN 运行时与配套版本 | ✗ | 部分 ✓ | ✗ 格式不符 |
| **msopgen 工程** | 本机 `源码/` 即其产物：json 算子定义 + `op_host`（tiling 结构体/shape 推断）+ `op_kernel` 骨架 + 完整 CMake 算子包。判题是 **Direct Invocation 直调**模板，不是算子包；`源码/` 的 `BEGIN_TILING_DATA_DEF` 等宏与直调入口无对应关系 | ✗ 两套工程 | ✓ 同为昇腾 | ✗ 上传物不是判题模板 |

**结论**：没有任何一条自动生成路线能产出符合 CANNJudge 直调格式的 `kernel.asc`。自动生成的价值止步于「读思想、抄结构、对齐约束」，提交物必须是按模板手写的 Ascend C。

补充：PyPTO 许可（CANN Open Software License 2.0）限定衍生作品用于华为 AI 处理器系统；即便未来能导出 Ascend C 文本，是否允许竞赛提交、是否满足「核心计算在 Kernel 中完成」的判题合规，均需赛方确认，**当前无证据，按不可用处理**。

---

## 4. 对 14 条方案的启发

对照 coverage-plan §2 的 14 条方案，编译器/IR 路线给出的判断：

| # | 方案 | 编译器侧启发 | 推荐级别（本 Agent 视角） |
| --- | --- | --- | --- |
| 1 | 两遍扫描 | Triton LayerNorm 默认结构；大 D 或 UB 紧时的稳健选择 | 首选（正确性基线） |
| 2 | 单遍暂存 y | TVM compute_at / 单 kernel 融合；仅当 y 的 FP32 副本能放进 UB（约 D≤4096 量级） | 第二路线（大 D 回退） |
| 3 | FP32 全中间计算 | Triton `.to(tl.float32)`、MLIR promotion 到高精度累加；与 golden「先升 FP32 再算」一致 | 首选（强制） |
| 4 | 低精度中间计算 | 编译器一般不这么做归约累加；bf16 顺序归约误差会放大 | 不建议 |
| 5 | 大 tile | Parametric tiling 由 fast-memory budget 决定；需扣掉 ReduceSum workLocal 与双缓冲余量 | 可用，但先算预算 |
| 6 | 小 tile | 增加 GM 往返；仅在 ReduceSum count 上限或 UB 紧张时用 | 谨慎 |
| 7 | 按行分配 AI Core | MLIR parallel iterator；行间无依赖，天然并行 | 首选 |
| 8 | 按 tile 分配 AI Core | 若 tile⊂单行则破坏行内归约（reduction 维跨核）；仅当「整行不可分、多行切块」时等价于 7 | 一般等价于 7，勿切开 D |
| 9 | DataCopyPad 尾块 | Triton mask+other=0 的 Ascend 等价物；PyPTO FAQ 证实 32B 对齐是硬件约束 | 首选（尾块） |
| 10 | 手工尾块 | 无自动工具能替你保证 D=67/129 不踩下一行；仍需真机核对 | 备选 |
| 11 | ReduceSum | 对应 TVM/MLIR 的「库调用/归约原语」；有 count 上限争议时分块 | 首选（配合分块） |
| 12 | 手工向量归约 | 类似 Triton 自己 tree-reduce；实现成本高，仅在 ReduceSum 不可用时 | 备选 |
| 13 | 纯 Ascend C | 判题唯一合法形态；一切 IR 路线最终都要落到这一层 | 首选（唯一提交路径） |
| 14 | CUDA/Triton 迁移参考 | LayerNorm 教程结构高度可映射（见 §2.2 表）；但启动模型、同步、内存层次不同，不能直接翻译语法 | 仅作研究参考 |

---

## 5. 已确认 / 未找到 / 无法确认

### 5.1 已确认

1. TVM TensorIR 提供 `split/reorder/reverse_compute_at/decompose_reduction` 等 schedule 原语，reduction 与 elementwise 融合通过 compute_at 完成（A）。
2. MLIR Linalg 的 key transformations 含 Parametric Tiling、Promotion to Temporary Buffer、Map to Parallel/Reduction Loops、Vectorization；iterator 分 `parallel`/`reduction`（A）。
3. IREE 官方 support matrix 含 Vulkan/ROCm/CUDA/Metal/AMD AIE，**无昇腾**（A）。
4. StableHLO 定义 `reduce`/`all_reduce` 等高层语义，定位是框架与编译器之间的可移植层，不直接产 Ascend C（A）。
5. Triton LayerNorm 教程：按行程序 + 沿 N 分块 + mask 尾块 + FP32 累加 + 两遍扫描；特征维 ≥64KB 整行时拒绝单遍 fused（A）。
6. microsoft/TileIR（原 TileLang）README 测试设备为 NVIDIA H100/A100/V100/4090/3090/A6000 与 AMD MI250/MI300X，**未列昇腾**（A）。
7. PyPTO 是 CANN 组织官方框架：PTO 范式，Tensor→Tile→Block→Execution 多层 IR，CodeGen 到 PTO 虚拟指令再到目标平台可执行代码；提供 `pypto.rms_norm`、`pypto.sum`、`set_vec_tile_shapes` 等 API；配套 CANN 版本发布（A）。
8. PyPTO FAQ 明确 tile 最后一维须 32 字节对齐（`TileShape[-1]*sizeof(dtype)%32==0`），原因是硬件指令限制（A）。
9. 本机 `源码/` 为 msopgen 风格工程（json + op_host tiling 头 + op_kernel），与判题直调模板（`run_kernel` + `TensorGroupInfo`）是**两套不同入口**（A，本地已读）。
10. 判题模板 `kernel.asc` 要求 `extern "C" void run_kernel(...)` 与 `__global__ __vector__` 内核，`#include "kernel_operator.h"`（A，模板已读）。

### 5.2 未找到

1. 华为官方「msopgen 生成结果可转换为直调 kernel.asc」的任何文档或工具。
2. PyPTO 导出「直调模板格式 `.asc` 源码」的公开功能（文档描述的是 JIT/离线二进制编译，不是生成判题文本）。
3. TVM/IREE/MLIR 官方昇腾后端（公开仓库与文档中均未出现）。
4. TileLang/TileIR 对 Ascend NPU 的公开支持声明。

### 5.3 无法确认

1. CANNJudge 判题端是否接受 PyPTO 产物或 msopgen 算子包（格式上模板已否定，但赛方是否另有通道无证据）。
2. PyPTO 在 CANN 9.0.0 / dav-2201 上与本题 15 个测试点的兼容性（本机无环境）。
3. TorchInductor 生成 RMSNorm 的具体 persistent/split 阈值数字（PyTorch 文档站部分 URL 失效，未取得源码级确认；概念以 Triton 教程与 PyTorch 2.x 总览为准）。
4. 自动工具生成代码在昇腾上的精度/性能是否达到判题 rtol=1e-3 与计分要求——**无任何真机数据，不得推测**。

---

## 6. 来源

| # | 标题 | URL | 访问日 | 等级 | 用途 |
| --- | --- | --- | --- | --- | --- |
| S1 | Apache TVM — TensorIR Transformation（Rewrite Reduction / compute_at） | https://tvm.apache.org/docs/deep_dive/tensor_ir/tutorials/tir_transformation.html | 2026-09-11 | A | reduction 分解与融合 |
| S2 | MLIR — 'linalg' Dialect（Key Transformations / iterator types） | https://mlir.llvm.org/docs/Dialects/Linalg/ | 2026-09-11 | A | tiling/promotion/parallel-reduction |
| S3 | Triton 官方教程 — Layer Normalization | https://triton-lang.org/main/getting-started/tutorials/05-layer-norm.html | 2026-09-11 | A | 归约+逐元素融合骨架、尾块 mask、FP32 累加 |
| S4 | IREE 官方首页（Support matrix） | https://iree.dev/ | 2026-09-11 | A | 无昇腾后端 |
| S5 | OpenXLA StableHLO Specification | https://github.com/openxla/stablehlo/blob/main/docs/spec.md | 2026-09-11 | A | reduce 语义层 |
| S6 | microsoft/TileIR README（原 TileLang） | https://github.com/microsoft/TileIR | 2026-09-11 | A | 测试设备仅 NVIDIA/AMD |
| S7 | CANN/pypto README（GitCode 官方仓） | https://gitcode.com/cann/pypto/blob/master/README.md | 2026-09-11 | A | PTO 范式、多层 IR、与 CANN 版本配套 |
| S8 | PyPTO 文档 — FAQ：set_xxx_tile_shapes 最后一维未 32 字节对齐 | https://pypto.gitcode.com/guide/appendix/faq/tileshape-32byte-alignment.html | 2026-09-11 | A | 昇腾 32B 对齐硬件约束 |
| S9 | PyPTO 文档中心（目录含 rms_norm / tiling / JIT / 离线二进制） | https://pypto.gitcode.com/ | 2026-09-11 | A | 产物形态与 API 面 |
| S10 | hw-native-sys/pypto（社区镜像，含 License 说明） | https://github.com/hw-native-sys/pypto | 2026-09-11 | B | 许可与架构补充 |
| S11 | 本地判题模板 kernel.asc / CMakeLists.txt / main.asc | `/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/` | 2026-09-11 | A | 直调入口格式 |
| S12 | 本地 msopgen 风格工程 json / tiling.h / op_host | `/Users/sunyiyang/Desktop/Project/cann/源码/` | 2026-09-11 | A | 算子包结构与直调模板差异 |
| S13 | PyTorch 2.x 总览（torch.compile / Inductor） | https://pytorch.org/get-started/pytorch-2-x/ | 2026-09-11 | B | Inductor 定位（细节页部分 404） |

---

## 7. 给主 Agent 的一句话建议

**提交路径只有手写 Ascend C（方案 13 + 1/7/9/3/11）**；TVM/MLIR/Triton 的价值是把「按行并行、按容量分 tile、尾块补 0、FP32 累加、init/update 分离」写对；PyPTO 与 msopgen 只能当旁证与思想来源，产物格式与判题直调模板不兼容，不要把「有自动工具」误当成「可以自动生成提交物」。
