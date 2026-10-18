# CANN AddRmsNormBias

比赛已于 2026-10-17 结束。本仓库保存基础源码、所有技术路线版本、比赛结果和历史资料，整理范围覆盖整个 `cann/` 目录。

## 基础源码与路线

- 唯一基础源码：`基础源码/submission.asc`，来源 R31B V011，Official 15/15、45.16。
- `技术路线/路线主线/` 有 132 个路线目录、1,140 个版本说明页；每页记录源码入口、说明和可查到的成绩。研究事件、占位、无候选及暂未找回源码的版本，在对应页面标明原因。
- N01-N10 保存源码路线，W5-R01-R05 保存研究路线；历史路线仍按原阶段和版本归档。
- 旧 W4 的五条技术路线新增 294 个版本目录，源码入口复用比赛归档中的 266 个去重源码对象；版本成绩 JSON 分开保留正式结果、Local 状态及用户提供但未由正式结果文件确认的分数。
- N06-N10 的最高实测 Local 分别为 96.815574、102.610179、99.440183、99.257867、99.089207，均未形成可靠 Local Best；归档没有新增 Official 分数。
- 各路线没有拼接成未经验证的新 Kernel；源码和结果依原路线、原版本保存。

## Agent 与插件资料

- `.agents/skills/` 保存项目可复用的 Ascend C 技能和 CANN 路线技能。
- `.codex/agents/` 与 `.codex/cannbot-plugin.json` 保存 Codex Agent 定义及插件清单；共享清单不包含本机插件安装路径。
- `.cannbot/plugins/ops-direct-invoke/` 保存许可文本和 hook 资料；`.cannbot/dependencies/ops-direct-invoke/` 保存 `asc-devkit`、`ops-tensor` 与 `tensor_api` 的本机源码快照及许可文件。上游仓库仍在 GitCode 独立维护，父仓只收录文件快照，不包含嵌套 Git 元数据。来源版本和快照范围见 [`.cannbot/README.md`](.cannbot/README.md)。

## 归档索引

- 路线源码对象：`归档/比赛结束-20261017/路线源码/对象/`
- 原始路线材料：`归档/比赛结束-20261017/项目材料/`
- N06-N10 结果摘要：`归档/比赛结束-20261017/实验记录/结果摘要/`
- 工具归档：`归档/比赛结束-20261017/项目材料/工具/route-runner/`（未验证 Runner 源码、fixture 和失败诊断；无 Correctness、Canonical Local 或 Official 结果）。
- Vector-Math V003 历史校验：`归档/比赛结束-20261017/项目材料/研究/VECTOR-MATH-X/V003/`（Canonical Local `60.477249`、POOR/LOCAL_NEUTRAL，不是 W5-R02 成绩）。
- 历史补充日志：`归档/比赛结束-20261017/实验记录/历史补充/`（纯文本编译/配置诊断，不含构建目录或二进制）。
- 来源映射与 Git 版本索引：`归档/比赛结束-20261017/索引/`
- 工作树与仓外内容快照收据：`归档/比赛结束-20261017/元数据/工作树快照-20261018.md`（历史快照、当前 7 个活动 worktree、未跟踪内容分类、bundle 校验和清理边界）
- 旧 W4 五路线版本页：`技术路线/路线主线/ROW-SCALE-HOIST-X/`、`MODE-DISPATCH-CUTOFF-X/`、`SYNC-BARRIER-ELISION-X/`、`STAGING-LIVENESS-X/`、`ALIGNED-TAIL-DATACOPY-X/`
- 路线入口：`技术路线/路线主线/`
- 路线状态和成绩：`技术路线/技术路线总表.md`、`技术路线/全项目成绩与技术路线盘点.md`、`技术路线/路线成绩表.tsv`
- 逐版本记录：`技术路线/全版本记录.tsv`

归档索引记录 4,006 个源码/构建输入对象、22,198 条来源路径映射、3,480 个 Git 源码对象和 9,414 条源码版本变更。N06-N10 另有 2,156 份可直接查看的结果摘要；旧 W4 的 294 条版本来源与 266 份去重源码由 W4 原始索引和统一对象索引共同记录。

## Git 状态

本地 `main` 已包含此前的本地路线历史和 `origin/main` 历史，并已推送到 GitHub。原有 96 条非 `main` 远端分支尖端均可从 `main` 追溯，远端现只保留 `main`。
截至 2026-10-18，`origin/main` 和归档主线均为 `90e618e0f3c3789617bf9d5c23d2188e080f4ba5`。归档 Git 元数据当前登记 7 个活动 worktree（主线、N02、N07、N08、N09、N10 和 hotloop-addr）；另有 19 个外部路线目录作为普通文件快照保存在 `worktrees-current-20261018/`，不再作为活动 linked worktree。主仓完整副本及恢复入口为 `/home/data4t2/lelinfeng/cann-archive-20261017.Gz8SZ7/worktrees-current-20261018/cann/`；原 `/home/data4t2/lelinfeng/cann` 主工作目录已在 bundle 验证后移除。最终 refs bundle 位于 `/home/data4t2/lelinfeng/cann-archive-20261017.Gz8SZ7/cann-all-refs-final-20261018.bundle`，当前 SHA256 以仓外 `FINAL_HANDOFF-20261018.txt` 为准。

历史兼容入口 `本地实验/`、`线上结果/`、`研究/`、`调度/`、`管理/`、`证据集成/`、`工具/` 仍指向归档内容。目录细节见 [`项目结构.md`](项目结构.md)，归档事实见 [`归档/比赛结束-20261017/README.md`](归档/比赛结束-20261017/README.md)。
