# 平台与研究覆盖计划（调研2）

> 日期：2026-09-11
> 题目：AddRmsNormBias（CANN 9.0.0 / vector kernel / 直调模板）
> 目标：方案空间调研，不写最终实现；所有结论标注证据等级。

## 0. 本地核对结论（只读）

| 项 | 状态 |
| --- | --- |
| 工作目录 | `/Users/sunyiyang/Desktop/Project/cann` |
| 分支 | `main`，与 `origin/main` 同步（`https://github.com/sxyq/CANN.git`） |
| 工作区变更 | AGENTS/文档/提交 README 有未提交修改；`调研/` 旧文件已删除（本轮不恢复） |
| 标准 Ascend C / msopgen 工程 | `源码/`：json 原型 + op_kernel + op_host + CMake |
| CANNJudge 直调提交代码 | `提交/V001`（CE）、`提交/V002`（过渡废弃）、V003 规划中 |
| 调研文档 | 本轮新建 `调研/调研2/` |
| 下载模板 | `/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/` 共 8 个文件 |

### 模板实际内容（A 级，逐文件已读）

| 文件 | 关键事实 |
| --- | --- |
| `kernel.asc` | 入口 `extern "C" void run_kernel(..., int64_t availableCoreNum, aclrtStream stream, float epsilon)`；注释说明内部应用 `add_rms_norm_bias_custom<<<blockNum,nullptr,stream>>>`；`TensorInfo.dtype` 0=fp32 1=fp16 2=bf16 |
| `main.asc` | 本地样例 FP16 shape=`[1,64]`，epsilon=`1e-5`；用 `ACL_DEV_ATTR_VECTOR_CORE_NUM` 取核数；`#include "kernel.asc"` |
| `CMakeLists.txt` | `find_package(ASC)`；`LANGUAGES ASC CXX`；默认 `SOC_ARCH=dav-2201`；可 `-DNPU_ARCH=` 覆盖 |
| `run.sh` | 要求 `ASCEND_HOME_PATH`；cmake+make；gen_data；timeout 120 跑可执行文件；verify_result case 0 |
| `scripts/AddRmsNormBias.py` | golden：先转 FP32，`y=x+r`，`rms=sqrt(mean(y^2)+eps)`，`out=y/rms*g+b`，再转回原 dtype |
| `scripts/gen_data.py` | 仅生成 1 个 case：FP16 `[1,64]`，eps=1e-5 |
| `scripts/verify_result.py` | case0：`rtol=0.001, atol=0.001, tol=0.001`（允许 0.1% 元素失配） |
| `data_utils.h` | 读写 bin 的辅助，无计算逻辑 |

### 语义锁定（与 golden 一致）

```text
y = x + residual
rms = sqrt(mean(y^2, dim=-1) + epsilon)
output = y / rms * gamma + bias
```

约束：FP16/BF16/FP32；2D/3D/4D；最后一维归约；D 可非 32 倍数；核心计算必须在 Ascend C Kernel；禁止 Host 代算/空 Kernel/写死结果。

**本机无 CANN、无 Ascend C 编译器、无昇腾 NPU。本轮不得声称 NPU 编译、精度或性能通过。禁止上传 CANNJudge。**

---

## 1. 十个子代理职责与不重叠边界

| Agent | 名称 | 平台 | 主题（互不重叠） | 输出文件 |
| --- | --- | --- | --- | --- |
| 1 | 比赛题面与提交接口 | CANNJudge、GitCode 比赛页 | 接口/shape/dtype/eps/测试点/计分/上传格式/SoC | `agents/agent01-competition.md` |
| 2 | 官方 Ascend C API | hiascend、asc.gitcode | DataCopy/DataCopyPad/ReduceSum/Cast/TPipe 等签名 | `agents/agent02-api.md` |
| 3 | 官方开源仓库实现 | Ascend/cann-samples、ops-* | RMSNorm/AddRmsNorm 样例源码与构建 | `agents/agent03-official-samples.md` |
| 4 | GPU/CUDA/Triton 迁移 | PyTorch/Triton/GPU MODE | 融合 RMSNorm kernel 与 Ascend 差异映射 | `agents/agent04-gpu-migration.md` |
| 5 | 编译器/IR/算子生成 | TVM/MLIR/IREE/TileLang | reduction lowering、自动 tiling 可参考性 | `agents/agent05-compiler-ir.md` |
| 6 | 数值精度与验证 | PyTorch/NumPy/MindSpore | FP32 累加、sqrt/rsqrt、误差矩阵设计 | `agents/agent06-precision.md` |
| 7 | 性能/UB/硬件 | Ascend 架构与 Profiling | UB 容量、tile、两遍 vs 单遍、多核 | `agents/agent07-perf-ub.md` |
| 8 | Linux/环境/真机工程 | Linux DO、V2EX、SO | CANN 安装、编译错误、远程 NPU | `agents/agent08-linux-env.md` |
| 9 | 竞赛失败案例 | GitHub Issue、历史赛题 | CE/WA/RE/TLE、BF16、尾块、提交包 | `agents/agent09-failure-cases.md` |
| 10 | 证据审阅与方案合并 | 前 9 份报告 + 本地代码 | 矛盾、反例、方案矩阵、推荐路线 | `agents/agent10-synthesis.md` |

### 每个代理的统一契约

- **输入范围**：本文件 §0 事实 + 题目语义 + 已有文档 `文档/problem-add-rms-norm-bias.md` 要点；不读其他 Agent 的未完成输出（Agent 10 除外）。
- **输出**：中文 Markdown，写入 `调研/调研2/agents/` 对应文件；含「已确认 / 未找到 / 无法确认」三段。
- **验收标准**：至少 5 条可引用来源（URL+标题+日期+证据等级 A/B/C/D）；方案相关结论写明「可迁移 / 不可直接迁移」。
- **禁止范围**：不修改 `源码/`、`提交/`、`文档/` 主线文件；不创建备份工程；不写 Token/密码；不声称本机 NPU 验证；不上传 CANNJudge。

### 证据等级

- **A**：官网题面、官方 API、官方仓库源码。
- **B**：官方样例、官方测试、原作者资料。
- **C**：社区文章、论坛、个人仓库。
- **D**：仅摘要或未核验线索。

---

## 2. 统一方案探索主题（比较维度）

必须比较至少：

1. 两遍扫描（Pass1 平方和，Pass2 归一化+gamma+bias）
2. 单遍暂存中间 y
3. FP32 全中间计算（推荐基线）
4. 低精度中间计算（仅对照）
5. 大 tile
6. 小 tile
7. 按行分配 AI Core
8. 按 tile 分配 AI Core
9. DataCopyPad 尾块
10. 手工尾块
11. ReduceSum
12. 手工向量归约
13. 纯 Ascend C
14. CUDA/Triton 迁移参考

评估表字段：`方案 | 算法 | 访存次数 | UB 占用 | 精度风险 | 尾块风险 | 性能潜力 | 实现难度 | CANN 兼容风险 | 推荐级别`

推荐级别：首选 / 可作为第二路线 / 仅作研究参考 / 不建议。

---

## 3. 执行波次

| 波次 | Agent | 说明 |
| --- | --- | --- |
| W1 | 1, 2, 3 | 题面与官方源（最高优先） |
| W2 | 4, 5, 6 | 迁移、编译器、精度 |
| W3 | 7, 8, 9 | 性能、环境、失败案例 |
| W4 | 10 | 合并审阅 |
| 收尾 | 主 Agent | 更新 `research-report.md`、`sources.md` 及必要文档交叉引用 |

---

## 4. 安全边界（本轮强制）

- 只做资料核对、源码审阅、CPU 参考计算与静态分析。
- 不得声称 CANN 编译通过、NPU 精度通过、性能达标。
- 严格禁止：上传 CANNJudge、消耗提交次数、写入凭据、创建重复工程、覆盖无关文件、把 GPU 实现直接标为 Ascend C 可用。
