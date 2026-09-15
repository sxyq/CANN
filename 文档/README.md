# 2026 CANN 挑战赛 · 西南赛区（初赛）AddRmsNormBias

> 独立 CANN 比赛工作区。本目录为 2026 年 CANN 挑战赛西南赛区初赛题 AddRmsNormBias 的全部准备、源码与验证记录，并单独使用 Git 管理。
> 目录结构见下文「目录说明」；所有外链来源与证据等级见 `../调研/归档/调研2/sources.md`（来源总表，235 条 S001–S291）。

## 1. 比赛概况

| 项 | 值 |
| --- | --- |
| 赛事 | 2026 年 CANN 挑战赛 · 西南赛区（初赛） |
| 赛事 ID | 2094722165106008066 |
| 题目 | AddRmsNormBias（算子核函数工程，beta） |
| 题面 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias |
| 提交页 | https://cannjudge.cn/public/op_challenge_xinan_prelim/addrmsnormbias/submit（需登录） |
| 判题环境 | CANN 9.0.0（题面标注 vector 计算） |
| 开始 | 2026/09/05 00:00:00 |
| 截止 | 2026/10/17 18:00:00 |
| 提交限制 | 每天最多 50 次（用户提供，平台页面未明文） |
| 数据规模 | 15 个测试点，全部通过才计分（平台题目 API desc 明文，2026-09-11 核实） |
| 平台当前数据(2026-09-10) | 通过率 53%，通过 39 人 / 尝试 74 人 |

*注：上表"每天 50 次"条目需在能登录提交页后复核平台规则页/判题说明确认；平台抓取页面未展示该信息。"15 个测试点"已由题目 API（https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85）的 desc 原文确认为官方明文。*

> **方向定夺请先读 [调研2 综合报告](../调研/归档/调研2/research-report.md)**：全项目的"方向建议 / 推荐路线 / 风险与下一步"汇总文档（**2026-09-12 十代理深度调研后重做**）。配套文件：来源总表 `../调研/归档/调研2/sources.md`（235 条）、统一方案矩阵 `../调研/归档/调研2/方案矩阵.md`、精度测试矩阵 `../调研/归档/调研2/精度测试矩阵.md`、本机 CPU 验证记录 `../调研/归档/调研6/validation-host-notes.md`、覆盖计划 `../调研/归档/调研2/覆盖计划.md`，以及 10 份子代理报告 `../调研/归档/调研2/`。
>
> 说明：`调研/归档/调研1/` 是 2026-09-11 上一轮的调研归档，若干结论（如 `DataCopyPadExtParams` 字段顺序、`ReduceSum` count 上限、写方向 padding 语义）在 `文档/problem-add-rms-norm-bias.md` 中仍有记录，本轮已由调研2 独立复核并升级，**以调研2 为准**。

### 算子语义（已确认，来自官网题面）

```text
y       = x + residual                    # 残差加法，逐元素
rms     = sqrt( mean(y^2, dim=-1) + eps ) # 沿最后一维的 RMS
z       = y / rms * gamma                 # 归一化 + 逐通道缩放
output  = z + bias                        # 逐通道偏置（融合关键点）
```

等价 PyTorch 组合（官方参考基准）：
`torch.nn.functional.rms_norm(x + residual, normalized_shape, weight=gamma, eps=epsilon) + bias`

## 2. 当前状态（2026-09-11）

- [x] 阶段一 环境核对：**本机无 CANN / 无 NPU**（结论见下「环境结论」），本地只能静态审阅 + Host 参考验证。
- [x] 阶段二 工作区整理：已迁移到独立目录 `/Users/sunyiyang/Desktop/Project/cann/`，不再属于 `master-goods` 的 Git 工作区。
- [x] 阶段三 文档：本 README + `competition-rules.md` + `problem-add-rms-norm-bias.md` + `../调研/归档/调研1/research-report.md` + `submission-checklist.md`。
- [x] 阶段四 调研：见 `../调研/归档/调研1/research-report.md`；关键 API 能力（ReduceSum FP32、DataCopyPad 双向非对齐、Atlas A2 支持）已确认。
- [x] 阶段五 源码：`../源码/` 下已按 msopgen 工程结构编写 op_host / op_kernel。
- [x] 阶段六 验证：无 NPU 环境下完成源码静态审阅 + Host 参考实现数值验证；117 组中 115 组通过本地诊断口径，2 组为 BF16、D=32768 的量化边界差异。
- [ ] 阶段七 上报：本 README 为汇报的一部分，最终如需上传/提交需用户明确确认（见 `submission-checklist.md`）。

## 3. 环境结论（阶段一）

| 项目 | 结果 |
| --- | --- |
| git 状态 | 独立 Git 仓库，分支 `main`；当前提交版本为 `V001` |
| CANN 工具链（ccec/ccec_compiler/bisheng/msopgen/msopst） | 未找到（PATH 与常见安装路径均无） |
| npu-smi / ascend-dmi | 未找到 |
| CANN 环境变量 / /usr/local/Ascend | 无 |
| NPU 设备 | 无（本机为 macOS 笔记本，无昇腾设备） |
| Docker / 昇腾镜像 | 无 Docker |
| 既有比赛源码 / 提交包 / 下载资料 | 已从 `master-goods` 迁移到当前独立目录；`提交/混合方案/H001-正确性优先/V001/kernel.asc` 已生成 |

**结论：本机为「无 CANN、无 NPU」环境。** 依据用户要求（规则 6），本工作区**不宣称**任何 NPU 编译、精度或性能通过；所有 NPU 侧结论一律标注为“未在真实 NPU 验证”。

## 4. 目录说明

```text
cann/
├── README.md                        # 仓库入口
├── 文档/                            # 比赛文档
│   ├── README.md                    # 本文档：概况 / 状态 / 入口
│   ├── competition-rules.md         # 规则、时限、提交额度、精度要求、合规边界
│   ├── problem-add-rms-norm-bias.md # 题面分析：公式、约束、边界、风险
│   ├── source-build.md              # 源码编译说明
│   └── submission-checklist.md      # 提交核对单
├── 源码/                            # Ascend C 源码和构建配置
│   ├── add_rms_norm_bias.json       #   原型定义（msopgen gen 输入）
│   ├── CMakeLists.txt  build.sh  CMakePresets.json
│   ├── op_host/                     #   Host 侧：原型注册 + tiling
│   └── op_kernel/                   #   Kernel 侧：Ascend C 核函数
├── 调研/                            # 调研和验证资料
│   ├── README.md                    #   调研入口
│   ├── openai-agent-research.md     #   OpenAI 公开 Agent 研究方式与迁移边界
│   ├── 多Agent研究/                  #   多 Agent、数学复核和算子竞赛流程证据
│   ├── 工具/reference_verify.py     #   参考验证脚本
│   └── 调研1/                       #   2026-09-11 多代理深度调研归档
│       ├── research-report.md       #     综合调研报告（15 章）
│       ├── sources.md               #     来源总表
│       ├── validation-host-notes.md #     CPU 辅助验证记录
│       ├── 方向建议.md              #     唯一方向建议
│       └── Agent01~10-*.md          #     10 份子代理报告
├── 提交/                            # 按 V001、V002、V003 管理的提交包
│   └── V001/kernel.asc              # 当前提交版本
├── 缓存/                            # 公开资料缓存
└── 临时/                            # 临时验证结果
```

## 5. 后续执行入口

1. **在有 CANN + NPU 的机器上复核提交包结构**：登录提交页获取上传格式（zip?、工程目录?、单文件?），与 `submission-checklist.md` 同步。
2. **用 msopgen 生成工程骨架**：`msopgen gen -i 源码/add_rms_norm_bias.json -c ai_core-<soc> -out <dir>`，再用 `源码/op_host`、`源码/op_kernel` 覆盖模板对应文件（或直接整包上传）。
3. **NPU 编译**：在 msopgen 生成工程中执行 `ASCENDC_GENERATED_PROJECT_DIR=/path/to/generated 源码/build.sh`，按 `提交核对单` 记录编译产物。
4. **精度验证**：用 msopst 或自建用例，覆盖 FP32/FP16/BF16 × 2D/3D/4D × 边界 D，逐测试点记录到 `提交/result-<日期>.md`。
5. **性能验证**：题面要求“在保证精度前提下优化性能”，记录每个测试点耗时并优化 tiling。
6. **提交**：仅在本机/服务端全部验证通过、且经用户明确同意后，才执行 CANNJudge 上传（当前不做）。

## 6. 已确认事实 vs 待确认事项

**已确认（证据：官网题面抓取 + 官方 API 文档 + 开源仓库页面）**

- 完整算子语义、输入输出 dtype（fp16/bf16/fp32）、ND 格式、gamma/bias 维度 (D,)。
- D ∈ [64, 32768]；batch ∈ [1,8192]；seq_len ∈ [1,32768]；2D/3D/4D。
- 非对齐要求：D 可能不是 32 的整数倍。
- 精度：fp32 相对误差<1e-4 且绝对误差<1e-4；fp16/bf16 相对<1e-3 且绝对<1e-3。
- NaN/Inf 需正常执行不崩溃（不要求特殊修复）。
- Atlas A2 支持 ReduceSum(half/float) 与 DataCopyPad 双向非对齐搬运（CANN 8.x 手册即支持，9.0.0 兼容性需在真机确认）。
- ops-transformer（gitcode.com/cann/ops-transformer）存在 AddRmsNorm 系列官方实现可作迁移参考。
- OpenAI 相关公开资料只用于提炼“拆解问题、并行探索、工具验证、独立复核”的研究组织方式；不作为 AddRmsNormBias 的算法来源，详见 `../调研/openai-agent-research.md`。

**待确认（列出即 blocker，提交前必须闭环）**

- CANN 官方学习仓的公开提交 Skill 给出了 `tiling_h`、`tiling_key_h`、`host_cpp`、`kernel_cpp` 等源码字段；提交页实际表单、字段版本和是否仍需要工程包，仍需登录后确认。
- “15 个测试点、每天 50 次”的平台明文出处（用户提供，页面未抓取到）。
- 判题 SoC 型号是哪一款（题面 `vector` + CANN 9.0.0 → 大概率 Atlas A2 系列；已按 A2 特性实现，真机需用实际 SoC 重新编译并核对 CMakePresets.json）。
- 题面精度表末行 “int32 要求完全准确” 疑似模板遗留（输入类型不含 int32），需官方澄清，不影响实现。
- epsilon 属性实际范围（题面写默认 1e-5，取值通常 1e-5~1e-6，按 float 处理）。

## 7. 约束与合规

- 核心计算必须由 Ascend C Kernel 在 NPU 完成；禁止 Host 代算、写死结果、绕过计算流程。
- 保留主项目既有用户改动：CANN 工作区已经迁出 `master-goods`，后续比赛内容只在当前独立仓库内增改。
- 凭据零写入：所有文档、源码、日志不写密码/Token/Cookie/授权头。提交页登录凭据只在浏览器内使用。
