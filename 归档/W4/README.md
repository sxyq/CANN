# W4 五路线收口归档

本目录只归档已经结束的五条 W4 路线：

- `ROW-SCALE-HOIST-X`
- `MODE-DISPATCH-CUTOFF-X`
- `SYNC-BARRIER-ELISION-X`
- `STAGING-LIVENESS-X`
- `ALIGNED-TAIL-DATACOPY-X`

归档基于独立 Integration worktree 的 `origin/main` 快照 `1ea9677ba8e7307a73f13041c7b639ab6a96425c` 建立。未复制 runner、build 目录、性能脚本、临时输出或实验 Candidate 执行链；源码归档仅保存各 Revision 目录的直接 `submission.asc`。

## 盘点

| 项目 | 数量 |
|---|---:|
| Revision 源文件 | 294 |
| 去重后独立源码 | 266 |
| 归档源码文件 | 266 |
| 没有 exact source commit 的源码 | 5 |
| 可定位的 W4 formal Online result | 0 |

五条路线分别为 67、111、81、34、1 个 Revision。重复 SHA 只保存一份，所有 Revision 仍在 `索引/revision-source-index.tsv` 中单独列出。

## 索引与规则

- `索引/revision-source-index.tsv`：Revision -> source path -> exact source commit（若存在）-> SHA-256 -> archived source，并保留 source-meta、sidecar、Local/Online 状态及正式结果查找路径。
- `索引/sha-dedup-index.tsv`：SHA 去重关系和全部 Revision 反向映射。
- `索引/inventory-receipt.tsv`：数量和未提交源码收据。
- `索引/incremental-source-audit-20261009.tsv`：续审中对五个原 worktree 的重哈希、额外未提交源码样本、Git 历史缺口和保留/不归档判定；不改变 294 条 Revision 与 266 个去重源码计数。
- `源码/<SHA-256>.submission.asc`：唯一源码文件名由实际 SHA-256 决定。
- 已提交源码的 `source_commit` 使用真实 Git 路径历史，并检查 commit 内容与当前源码 SHA 一致。
- 未提交源码使用 `source_commit=UNKNOWN`、`source_state=UNCOMMITTED`；不把 worktree 状态伪造为 commit。

## 结果口径

`官方结果摘要.tsv` 只引用当前正式记录能定位的 `线上结果/<ROUTE>/<REVISION>/result.json`。五条 W4 路线均没有该正式结果，因此 Planning 给出的分数只作为 `PLANNING_CLAIM_UNVERIFIED` 保存，不能写入 Official Score 或 Submission ID。具体冲突和 source-meta 解释见 `冲突与证据界限.md`。

Overall Champion 仍为正式记录中的 `R31B V011 / 45.16 / 15/15`，Submission ID 为 `6ab2c10c0304f72a56a0c5cb`，证据为 `线上结果/R31B/V011/result.json`。本归档不改变任何 Champion、历史 Revision 证据或 Main-1 worktree。

W4 的机制总结、失败原因和证据路径见 `W4技术总结.md`。
