# 本地结构整理报告（2026-09-18）

分支：`chore/canonicalize-2026-09-18`（基于 `main` @ `dfa0af0`）。未 push，未执行任何 git 突变性操作（本报告写入本身不改动 Git 状态）。

## 1. 提交列表

| # | SHA | 说明 |
| --- | --- | --- |
| 1 | `97a9f1e` | chore: checkpoint local evidence before canonical cleanup |
| 2 | `21c61c4` | chore: establish canonical experiment and documentation layout |
| 3 | `90c5ccd` | docs: mark legacy route records as historical |
| 4 | `fa69a98` | docs: fix management README link text to 脚本/README.md |

说明：提交 1–3 为整理主线；第 4 条为链接文案修正。本报告自身可作为后续小提交入库。

## 2. 实际完成事项

### 2.1 清理前快照

- 快照目录：`临时/pre-cleanup-2026-09-18/`
- 内容：`git-status`、`diff`、`untracked`、`head`

### 2.2 分支与检查点

- 已创建分支 `chore/canonicalize-2026-09-18`，**未 push**
- 检查点提交（`97a9f1e`）纳入：
  - 对话全文
  - workbuddy memory
  - V005 服务器证据（移动前原位）
  - `脚本/`
  - `package.json`、`.gitignore`、`cannjudge.py`
  - tools README
  - 删除旧 browser-submit 与旧 scripts README 的变更
  - audit report
  - pre-cleanup 快照
  - V003 kernel tile 编辑（作为证据保留）

### 2.3 规范化实验目录

- 新建：`实验/server/V005`、`实验/online/`、`实验/local/`、`实验/manifests/`，各含 README
- `git mv 临时/服务器证据/V005 → 实验/server/V005`：20 个文件，内容 SHA `afd5b8ee…`，与 `混合方案/H001-正确性优先/V005` 一致
- `git mv 临时/LOCAL_FULL_STATE_REPORT.md → 调研/归档/审计/LOCAL_FULL_STATE_REPORT_2026-09-18.md`

### 2.4 规范文档

- 核心文档落位：`文档/当前状态.md`、`文档/路线树.md`、`文档/实验纪律.md`、`文档/CANNJudge流程.md`、`文档/代码与结果溯源.md`、`文档/README.md`
- 根 `README.md` 重写为入口说明
- `AGENTS.md` 执行策略段已更新
- `管理/README` 链接修正至 `脚本/README.md`（见提交 4 占位）
- 4 份历史材料加 `HISTORICAL` 横幅；新建 `管理/路线状态/README.md`

## 3. 明确未做事项

- 未向 CANNJudge 提交
- 未 push 任何分支
- 未开展 V002 / V003 实验
- 未开展 FULL-R013 实验
- 未删除 对话全文、H001、H002、external track（外部 20）
- 未对 R029-V003 双 SHA 做裁定
- 未提交 browser profile（仅记录路径）
- 未伪造 submission id / SHA 绑定
- 未虚构本地缺失的 FULL-R014 / FULL-R016 / FULL-R013 源码或结果

## 4. 对象保留状态

| 对象 | 状态 |
| --- | --- |
| 对话全文 | 保留，已入检查点 |
| workbuddy memory | 保留，已入检查点 |
| V005 服务器证据 | 保留，已迁移至 `实验/server/V005`（SHA 一致） |
| H001 / H002 | 保留，未删除 |
| external track（外部 20） | 保留，未删除 |
| browser profile | 仅保留路径记录，未提交 profile 本体 |

## 5. 规范文档职责（简表）

| 文档 | 职责 |
| --- | --- |
| `文档/当前状态.md` | 当前真相入口；含 FULL-R013 = NEXT |
| `文档/路线树.md` | 路线层级与编号总览 |
| `文档/实验纪律.md` | 单点修改、回退与证据要求 |
| `文档/CANNJudge流程.md` | 提交与验收流程 |
| `文档/代码与结果溯源.md` | 源码/结果与提交、实验的对应关系 |
| 根 `README.md` | 仓库入口与阅读顺序 |

## 6. 工具验证

| 项 | 结果 |
| --- | --- |
| `node --check cannjudge-submit.mjs` | OK |
| `npm run cannjudge:submit -- --help` | OK |
| `python3 -m py_compile cannjudge.py` | OK |
| runtime profile | 已被 ignore，tracked 文件 0 |
| V005 迁移 20 文件 SHA | 与 H001/V005 一致（`afd5b8ee…`） |
| `文档/当前状态.md` | FULL-R013 = NEXT |
| 提交后 working tree | clean |

## 7. 遗留问题（如实列出）

- R029-V003 双 SHA 未裁定
- 历史 R013 / R014 / R016 / R006 与 FULL 代次并存，语义关系未完全对齐
- 上游 FULL-R014 / FULL-R016 结果、FULL-R013 候选源码：**本地尚无对应文件**
- 多数官方分数缺少 `submission_id` 与 git 绑定
- `.workbuddy` 历史 memory 中仍有 `scripts/README` 等旧路径表述（历史记录，预期保留）
- “无有效平台分”表述仍出现在带 HISTORICAL 横幅的历史/归档上下文或引述中

## 8. 最终判定

**YES** — 本地结构已整理，可接收上游最终交接包；**尚未开始 FULL-R013**。
