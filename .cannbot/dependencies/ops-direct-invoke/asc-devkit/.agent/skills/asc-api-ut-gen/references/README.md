# asc-api-ut-gen reference 导航

本目录按内容职责拆分 reference 文档。新增或维护文档时，优先放入对应子目录，避免把 API 约束、工作流命令、通用模板和排障经验混在同一层。

## 目录划分

| 目录 | 内容边界 |
|------|----------|
| [api-guides/](api-guides/) | API 类型专属 UT 指南，只维护该 API 类型的目录、核心类型、数据流、参数结构、分支组合和常见约束 |
| [workflows/](workflows/) | 工作流文档，包括生成后验证、覆盖率扫描、`build/cov_report` 补齐和复查流程 |
| [foundations/](foundations/) | 跨 API 通用基础资料，包括 gtest 骨架、分支覆盖分析和 LocalTensor 内存申请 |
| [troubleshooting/](troubleshooting/) | 常见问题、错误现象和排障索引 |

## API 类型指南

| API 类型 | 文档 |
|----------|------|
| 高阶 API | [adv-api-ut-guide.md](api-guides/adv-api-ut-guide.md) |
| membase AIV API | [membase-api-aiv-ut-guide.md](api-guides/membase-api-aiv-ut-guide.md) |
| membase AIC API | [membase-api-aic-ut-guide.md](api-guides/membase-api-aic-ut-guide.md) |
| regbase API | [regbase-api-ut-guide.md](api-guides/regbase-api-ut-guide.md) |
| C API | [c-api-ut-guide.md](api-guides/c-api-ut-guide.md) |
| SIMT API | [simt-api-ut-guide.md](api-guides/simt-api-ut-guide.md) |
| Utils API | [utils-api-ut-guide.md](api-guides/utils-api-ut-guide.md) |

## 工作流

| 场景 | 文档 |
|------|------|
| UT 生成后的编译、执行和报告 | [automation-guide.md](workflows/automation-guide.md) |
| API UT 覆盖率扫描 | [coverage-scan-guide.md](workflows/coverage-scan-guide.md) |
| 基于 `build/cov_report` 补齐低覆盖 UT | [coverage-report-backfill-guide.md](workflows/coverage-report-backfill-guide.md) |

## 通用基础资料

| 主题 | 文档 |
|------|------|
| API 类别、实现目录和 UT 目录映射 | [api-directory-map.md](foundations/api-directory-map.md) |
| 通用 gtest、参数化测试和结果比较骨架 | [test-templates.md](foundations/test-templates.md) |
| 分支覆盖分析方法 | [branch-coverage-guide.md](foundations/branch-coverage-guide.md) |
| LocalTensor、TPipe、TQue 和临时空间申请 | [local-tensor-memory.md](foundations/local-tensor-memory.md) |
| 生成器结构化约束 | [generation-constraints.json](foundations/generation-constraints.json) |

## 排障

| 主题 | 文档 |
|------|------|
| 常见问题与解决方案 | [faq.md](troubleshooting/faq.md) |

## 维护规则

- API 类型 guide 不重复维护编译命令、覆盖率流程和通用模板，统一链接到 `workflows/` 或 `foundations/`。
- `workflows/` 只描述流程、命令、验证和报告，不维护单个 API 类型的参数细节。
- `foundations/` 只维护跨 API 通用材料，不固化具体 API 的 dtype 支持范围。
- 脚本可读取 `foundations/generation-constraints.json` 和 `../asc-npu-arch/references/npu-arch-facts.json`，不要解析 Markdown 表格或在脚本中复制芯片/dtype/API profile 表；通用模板可直接初始化的 dtype 也应从结构化 facts 派生。
- 常见错误先沉淀到 `troubleshooting/faq.md`，稳定后再回链到对应 API guide 或 workflow。
