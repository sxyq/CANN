# 调研资料

本目录保存 AddRmsNormBias 的 Ascend C 资料、公开来源、CPU 辅助验证和多 Agent 研究记录。源码在 `../源码/`，比赛文档在 `../文档/`，待上传内容在 `../提交/`。

## 文件

| 内容 | 位置 |
| --- | --- |
| Ascend C 实现路线 | [research-report.md](research-report.md) |
| OpenAI Agent 研究与迁移边界 | [openai-agent-research.md](openai-agent-research.md) |
| 来源清单 | [sources.md](sources.md) |
| CPU 辅助验证 | [validation-host-notes.md](validation-host-notes.md) |
| 参考脚本 | [工具/reference_verify.py](工具/reference_verify.py) |
| 多 Agent 深度研究简报 | [多Agent研究/研究简报.md](多Agent研究/研究简报.md) |

## 多 Agent 研究结构

```text
多Agent研究/
├── 研究简报.md
└── 发现/
    ├── 官方资料与学术论文.md
    ├── GitHub开源仓库.md
    ├── 社交平台讨论.md
    ├── 系统工程社区.md
    ├── 国外社区与论坛.md
    ├── 中国社区与昇腾生态.md
    ├── 竞赛与科研流程.md
    └── 多Agent架构模式.md
```

`发现/` 中的内容主要是公开资料核对结果和迁移分析。除明确标注外，外部仓库没有在本机运行；外部数学证明、社区分析和本题 Kernel 代码不能相互替代。

## 证据等级

- A：官方题面、官方 API 文档、官方预印本、官方页面或官方 RSS。
- B：官方仓库、官方样例、官方培训材料、原作者源码。
- C：社区文章、论坛、个人仓库和独立媒体报道。

来源记录统一写入 [sources.md](sources.md)，包含 URL、版本或 commit、访问日期、用途和证据状态。任何真实 CANN/NPU 结论都必须附编译、运行或性能记录；当前环境不满足这个条件。
