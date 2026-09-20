# 调研2 目录

本目录是 2026-09-12 对 AddRmsNormBias 的深度技术方案调研产物。

| 文件 | 说明 |
| --- | --- |
| `coverage-plan.md` | 平台与 10 代理覆盖计划 |
| `research-report.md` | **主报告**（15 节：范围/约束/API/开源/迁移/精度/性能/环境/失败/矩阵/首选/第二路线/风险/未确认/实验计划） |
| `sources.md` | 汇总来源清单（A/B/C/D） |
| `agents/agent01-competition.md` | 题面、规则、提交接口 |
| `agents/agent02-api.md` | CANN 9.0.0 官方 API |
| `agents/agent03-official-samples.md` | 官方开源仓库 |
| `agents/agent04-gpu-migration.md` | GPU/CUDA/Triton 迁移映射 |
| `agents/agent05-compiler-ir.md` | 编译器/IR/算子生成 |
| `agents/agent06-precision.md` | 数值精度与测试矩阵 |
| `agents/agent07-perf-ub.md` | 性能、UB、tiling |
| `agents/agent08-linux-env.md` | Linux/环境/真机工程 |
| `agents/agent09-failure-cases.md` | 竞赛失败模式 |
| `agents/agent10-synthesis.md` | 证据审阅与 14 条方案合并 |

**边界**：本机无 CANN/NPU；本轮未上传 CANNJudge；未声称编译/精度/性能通过。

**主线结论**：V003 基线 = 两遍扫描 + FP32 强制 + 分块≤4096 + 按行分核 + DataCopyPad（逐字段赋值）+ ReduceSum + 纯手写 `kernel.asc`。性能第二路线在真机精度闭环后按 P1 常驻 → P2 向量累加 → P3 单遍 → P4 双缓冲推进。
