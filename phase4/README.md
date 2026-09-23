# Phase4

这是新的 Clean-Room 探索区域。

禁止读取：

`../归档/`

Phase4 第一轮禁止使用 Git 历史恢复旧实现，包括：

```text
git log
git show
git reflog
git branch -a
git tag
git worktree list
git diff <historical-ref>
git show <historical-ref>:<path>
```

禁止访问任何非当前 Phase4 文件的旧 commit、branch 或 tag。

允许的 Git 操作仅限：

```text
git status
git add
git commit
```

以及 Phase4 新产生 commit 的必要操作。

不得通过 GitHub remote branches 搜索历史 kernel。

所有技术规划由下一次全新会话完成。

## Revision evidence format

Every new revision must have one record directory. Preserve all source bytes and keep the original route workspaces in place.

Online records use `phase4/online/<ROUTE>/<REVISION>/` and contain:

```text
result.json
submission.asc
source-meta.json
diff.patch
submission.sha256
```

Local-only records use `phase4/local/<ROUTE>/<REVISION>/` and contain:

```text
local-result.json
submission.asc
source-meta.json
diff.patch
submission.sha256
```

For online records, `submission.asc` holds the exact CANNJudge input. For local-only records, it holds the exact captured candidate source. `submission.sha256` records those bytes. For a multi-file local candidate, retain its required support files beside the record and list each path and digest in `source-meta.json`.

Each record names its direct parent revision and source commit, one `single_hypothesis`, its source commit and source path, the result, and a decision. `diff.patch` compares the candidate with that direct parent's source. If the parent cannot be proven, write `PARENT_UNRESOLVED` and leave the parent fields null. Do not infer a missing source from version order or a nearby candidate.

Use `ONE FACTOR AT A TIME` and `ONE CONCEPTUAL CHANGE PER REVISION`. Each new performance revision has one direct parent, one hypothesis, and one parent diff. Record `SINGLE_CHANGE_AUDIT` as `PASS`, `MULTI_CHANGE`, or `UNKNOWN`.

Use `PROMOTE` only when correctness passes and the Official Score exceeds the direct parent's Official Score. Use `REJECT` when correctness fails or the Official Score does not exceed the parent's. Use `INCONCLUSIVE` when a required score comparison is unavailable. Keep all source and result records for rejected or inconclusive revisions. A later independent change branches from the latest promoted version, not from a rejected performance result. Correctness-only repairs may branch from the affected revision and must not add a performance variable.

Local results remain separate from Judge results. Screen locally before selective online submission. A missing Judge result stays `UNKNOWN`; do not resubmit an old revision just to recover its result.

Main may use exact Git blobs to recover historical submitted sources and create parent diffs for this evidence migration. This exception does not authorize Route Agents to read unrelated historical or archived implementations. It does not change any recorded score or Candidate computation.

`phase4/champions/current.tsv` indexes the Overall Champion, each Route Best with a valid online result, and active slots that have only local evidence. Champion source paths point to the corresponding online snapshot.
