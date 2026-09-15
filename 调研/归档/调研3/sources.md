# 来源清单（调研2）

> 访问日期：2026-09-11 ~ 2026-09-12
> 等级：A=官网题面/官方 API/官方模板；B=官方样例/源码/测试/原作者；C=社区；D=未核验摘要

## A 级

| ID | 来源 | URL / 位置 | 用途 | 状态 |
| --- | --- | --- | --- | --- |
| A1 | CANNJudge 题面 JSON | `cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85` | 语义、IO、dtype、D 范围、精度阈值 | verified |
| A2 | CANNJudge 比赛 JSON | `.../api/contests/name/op_challenge_xinan_prelim?groupId=69c8ffcea2bcbfa3591f481e` | 时间、15 点、计分、封榜 | verified |
| A3 | 官方模板包 | `.../api/problems/.../package` | kernel.asc/main.asc/run.sh/golden | verified |
| A4 | 排行 API | `.../api/problems/.../ranking` | tbest、得分公式旁证 | verified |
| A5 | 模板可编辑树 | `.../api/problems/.../template` | 仅 kernel.asc 可编辑 | verified |
| A6 | editor.js | `cannjudge.cn/js/pages/problem/editor.js` | 提交表单、保护文件 | verified |
| A7 | 提交 stats | `.../api/submissions/contest/.../stats` | 规模、通过率 | verified |
| A8 | CANN 9.0.0 DataCopyPad | hiascend …/atlasascendc_api_07_0265.html | 签名、字段表、示例 | verified（API2） |
| A9 | CANN 9.0.0 ReduceSum | …/atlasascendc_api_07_0078.html | sharedTmpBuffer、dtype | verified |
| A10 | CANN 9.0.0 Cast / RoundMode | …/atlasascendc_api_07_0073.html | CAST_NONE / CAST_RINT | verified |
| A11 | CANN 9.0.0 Add/Mul/Muls/Sqrt/Rsqrt | 07_0035/0037/0055/0029/0030 | 无 bf16 算术 | verified |
| A12 | SetFlag/WaitFlag / PipeBarrier | 07_0270 / 07_0271 | V_S、FetchEventID、PIPE_S 禁用 | verified |
| A13 | REGISTER_TILING / GET_TILING | 07_00003 / 07_0215 | 直调不支持 | verified |
| A14 | GetReduceRepeatSumSpr | 07_0225 | 官方可选读数接口 | verified |
| A15 | 模板 golden | `scripts/AddRmsNormBias.py` | FP32 链语义 | verified |
| A16 | 模板 verify | `scripts/verify_result.py` | rtol/atol/tol=0.001 | verified |
| A17 | 模板 CMake / run.sh | 模板根目录 | ASC、dav-2201、timeout 120 | verified |
| A18 | 本地 源码/op_kernel | `源码/op_kernel/add_rms_norm_bias.cpp` | 两遍+FP32 基线 | verified |
| A19 | 本地 提交/V002 | `提交/V002/kernel.asc` | 逐字段赋值 DataCopyPad | verified |
| A20 | PyTorch RMSNorm / NVIDIA 混合精度指南 | 官方文档 | FP32 归约共识 | verified |
| A21 | TVM/MLIR/IREE/Triton/PyPTO 官方文档 | 各官方站 | 自动工具不可直接提交 | verified |

## B 级

| ID | 来源 | URL | 用途 | 状态 |
| --- | --- | --- | --- | --- |
| B1 | op-plugin AddRmsNorm | github.com/Ascend/op-plugin …/AddRmsNormKernelNpu.cpp | 官方语义无 bias | verified |
| B2 | op-plugin API 文档 | …/torch_npu-npu_add_rms_norm.md | rstd=FP32、产品矩阵 | verified |
| B3 | mojo_opset fused_add_rms_norm.py | github.com/Ascend/mojo_opset … | 两遍 Triton 对照 | verified |
| B4 | Ascend/samples Add_tile | github.com/Ascend/samples …/Add_tile | Ascend C 工程骨架 | verified |
| B5 | Apex / Liger / flash-attn / Triton 教程 | 官方仓 | GPU 骨架与 post-bias | verified |
| B6 | tilelang-ascend #1682 / #1717 / #1762 / PR#1777 | GitHub Issue/PR | DataCopyPad/BF16/UB 失败模式 | verified |
| B7 | triton-ascend #1638/#1640 | GitHub | UB 溢出 | verified |
| B8 | Liger #1335 / FlagGems #4156 | GitHub | int32 偏移溢出 | verified |
| B9 | InfiniCore #1519 | GitHub | block_idx 保留字 CE | verified |
| B10 | RMSNorm 原文 arXiv 1910.07467 / Mixed Precision 1710.03740 | arXiv | 精度理论 | verified |

## C 级

| ID | 来源 | URL | 用途 | 状态 |
| --- | --- | --- | --- | --- |
| C1 | Linux DO 昇腾部署帖 | linux.do/t/topic/2710138 等 4 帖 | 驱动版本、现场 | topic 页可读；部分作者未暴露 |
| C2 | V2EX 昇腾帖 | v2ex.com/t/1231122 等 6 帖 | 社区讨论 | API 核验 |
| C3 | 社区 UB=192KB 说法 | 多篇培训/社区 | 预算口径 | 未独立核验 9.0.0 正文 |
| C4 | GitCode 赛事 intro | competition.gitcode.com/…/intro | 报名 | SPA，正文不可提取 |

## D 级 / 未找到

| 对象 | 结果 |
| --- | --- |
| 官方 AddRmsNormBias Ascend C kernel | **未找到**（开源侧无） |
| cann-samples 独立仓库 | 404；实际名 `Ascend/samples` |
| ops-transformer / ops-nn | 404 / 不存在 |
| 判题 15 点 shape/dtype/eps | 平台不开放 |
| 每日 50 次提交 | 题面/前端无明文 |
| 西南决赛细则 | 无独立页面 |

详细逐条 URL 与抓取方式见 `agents/agent01` … `agent10` 各报告「来源清单」节。
