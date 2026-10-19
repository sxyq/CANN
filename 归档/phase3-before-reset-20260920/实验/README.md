# 实验目录

本目录集中存放实验与结果证据，按来源分为四块：

```text
实验/
├── manifests/   # 源码 SHA-256 ↔ git commit ↔ 线上 submission 的绑定清单
├── server/      # 服务器侧编译 / 正确性 / 性能证据（V005 等）
├── online/      # 线上结果快照（submission id、15 case、Official Score）
└── local/       # 本机 CPU / 本地实验（非服务器权威证据）
```

- `server/V005/`：由 `临时/服务器证据/V005/` 迁入（迁移由清理流程中的其他 Agent 执行）。
- `online/`：当前对 FULL-R013 及之后的路线**尚未**产生快照；见该目录 README。
- `manifests/`：见该目录 README。
- 分数与路线状态的权威来源仍是 `文档/当前状态.md`；本目录只存证据文件本身。
