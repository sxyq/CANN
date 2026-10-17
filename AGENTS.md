# CANN AddRmsNormBias

比赛已于 2026-10-17 结束。本仓库当前用于保存基础源码和比赛档案，不再作为活跃优化工作区。

## 基础源码

- 唯一基础源码：`基础源码/submission.asc`。
- 来源：R31B V011；Official 15/15，45.16。
- 源码 SHA256：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`。
- N01-N05 的候选实现不拼接进基础源码；全部版本、路线工作树、构建与运行证据集中保存在 `归档/比赛结束-20261017/`。
- 五条路线的分支历史已并入本地 `main`；各路线源码仍按版本独立保存在路线工作树和项目材料中。
- 原 `本地实验/`、`线上结果/`、`研究/`、`调度/`、`管理/`、`证据集成/` 和 `工具/` 路径保留为指向归档内容的符号链接，供历史记录继续使用。

## 五条路线收口

| 路线 | 最后版本 | 收口事实 |
|---|---|---|
| N01 ROW-CORE-GEOMETRY-X（原 REGISTER-LIVE-RANGE-X） | V010 | V010 仅源码留档；V009 最近完成 Compile/Correctness 与 18 组 Local，未见稳定收益 |
| N02 WIDE-UB-RESIDENCY-X（原 CODE-FOOTPRINT-ICACHE-X） | V017 | Compile、ldd 完成；Correctness 与 Local 未运行 |
| N03 REDUCTION-TOPOLOGY-X（原 VECTOR-ILP-DUALCHAIN-X） | V016 | V016 仅完成源码查阅；V015 最近执行结果未见稳定 Local 收益 |
| N04 DMA-PIPELINE-OVERLAP-X（原 GM-CACHE-STREAMING-X） | V020 | Official 41.21；Local 未见稳定收益 |
| N05 PARAM-RESIDENCY-EPILOGUE-X（原 SCALAR-ADDRESS-GENERATION-X） | V016 | Correctness 8/8；四组 Local 区间跨零、无稳定收益；未提交 Official；V011/V012 分数分别为 41.25/41.12 |

逐版本记录和来源材料以 `技术路线/` 与 `归档/比赛结束-20261017/` 中保存的文件为准。路线合并指分支历史并入本地 `main` 并统一归档，不代表生成新的组合源码。

## 数据保存

- 保留全部源码版本、Git 历史、分支、工作树、实验日志、线上结果、失败材料和用户文件。
- 不使用 `reset --hard`、`clean`、force push 或历史改写；不覆盖已有的用户修改。
- 凭据不写入源码或归档日志。
- 本轮改动均保存在本地 Git 历史中，没有推送远端。

## 入口

- 项目说明：`README.md`
- 目录说明：`项目结构.md`
- 比赛归档：`归档/比赛结束-20261017/README.md`
- 路线总表与成绩：`技术路线/技术路线总表.md`、`技术路线/路线成绩表.tsv`
- 全版本记录：`技术路线/全版本记录.tsv`
- 项目规则：`项目规则/`
