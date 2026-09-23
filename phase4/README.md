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

Main exception（provenance recovery only）：Main 可用 exact Git blobs 恢复历史提交源码并生成 parent diff / evidence migration。该例外不授权 Route Agent 读取无关历史实现，也不改任何记录分数。

## Submission Script Hardening (CURRENT)

正式提交必须：

```bash
npm run cannjudge:submit -- --yes --source <本地源码路径>
```

暂时禁用：

```bash
npm run cannjudge:submit -- --yes
```

即：禁止 clipboard / terminal paste / stdin 作正式提交。

提交前打印 source path / line count / byte count / SHA256 / first non-empty line / last non-empty line。

同目录若存在 `submission.sha256`，必须自动核对；不一致立即停止。

提交后读取 Judge `files[].content`，重算 remote kernel SHA，要求 `LOCAL_SHA == REMOTE_SHA`。否则 `INPUT_IDENTITY_MISMATCH`，该 submission 不得作为正式实验结果。

实现：`脚本/cannjudge-submit.mjs`。说明：`脚本/README.md`。

## Infrastructure Incident (CLOSED)

R31A-V016 historical:

- SHA: `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0`
- Result: 15/15, 45.00

Failed:

| submission | remote kernel SHA | classification |
|---|---|---|
| `6ab35deb…` | `ab5bb308…` | DIFFERENT_INPUT_CONTENT |
| `6ab3a527…` | `0e52d372…` | PARTIAL_SOURCE |

这两条不是 V016 regression。

记录为：

```text
INFRASTRUCTURE INCIDENT
ROOT_CAUSE = INPUT_DIFFERENT
```

证据：`phase4/online/R31A/V016/root-cause-note.md`、`submission-json-probe.json`。

## Main 边界

Main 只负责：规划、review、Git、Judge、结果、文档、provenance、调度。

Main 不修改：`.asc` / `.cpp` / `.h` / `.hpp` / CMake / Kernel / runner / host wrapper。

Kernel 问题交回对应 Route Agent。

## 当前路线树（EXPLOIT / EXPLORE 分离）

```text
PROJECT
│
├── EXPLOIT ×2
│   ├── R31B-V011 45.16
│   └── R31A-V016 45.00
│
└── EXPLORE ×4
    ├── MIX-A-V003 44.69
    ├── WIDE-X-FRESH4
    ├── MODE-X-R015C
    └── EXT-ASCEND-X (NOT_STARTED)
```

R31A / R31B 是固定 EXPLOIT，不得写进 4 条 EXPLORE 方向。详见 `phase4/control/next-round-plan.md`。

EPI-X-FRESH is PARKED; retain its source and local result. The six Route Agents, isolated worktrees, branches, and contexts await a session that can create them. Do not treat shared source-seed directories or previous Agent IDs as current ownership.

## 当前阶段（同步完成，等待 Agent-capable session）

```text
✅ Source Recovery
✅ Server3 Repro
✅ V016 Root Cause (INPUT_DIFFERENT)
✅ Submission Hardening
✅ Full Review
✅ Local/Online Calibration
✅ Champion Review
✅ Six-route state and parent rules synchronized
🔵 Create six isolated Route Agents / worktrees / branches / contexts

No Kernel experiment, server3 run, or CANNJudge submission is part of this synchronization.
```

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

修改前必须记录：

```text
ROUTE
REVISION
DIRECT_PARENT
PARENT_SOURCE_SHA
PARENT_SCORE
SINGLE_HYPOTHESIS
CONTEXT_CLASS
```

版本号不是自动 Parent。

Use `PROMOTE` only when correctness passes and the Official Score exceeds the direct parent's Official Score. Use `REJECT` when correctness fails or the Official Score does not exceed the parent's. Use `INCONCLUSIVE` when a required score comparison is unavailable. Keep all source and result records for rejected or inconclusive revisions. A later independent change branches from the latest promoted version, not from a rejected performance result. Correctness-only repairs may branch from the affected revision and must not add a performance variable.

Candidate 只和 Direct Parent 比。正确性 PASS 且 Official Score > Parent：`PROMOTE`。否则 `REJECT` 或 `INCONCLUSIVE`。

REJECT 后：下一个独立 hypothesis，重新从 Best Parent 开始。禁止在 regression 上继续叠第二个性能修改。

Local results remain separate from Judge results. Screen locally before selective online submission. A missing Judge result stays `UNKNOWN`; do not resubmit an old revision just to recover its result.

`phase4/champions/current.tsv` indexes the Overall Champion, each Route Best with a valid online result, and active slots that have only local evidence. Champion source paths point to the corresponding online snapshot.

## Local-first

```text
Single Hypothesis
        ↓
Compile / Link
        ↓
Correctness
        ↓
Paired Local
       /       \
Local Reject   Promising
                 ↓
          ONLINE_CANDIDATE
                 ↓
             CANNJudge
             /       \
        PROMOTE      REJECT
```

不要把 compile PASS 当成 local performance score。

## Judge payload provenance

`EXACT_GIT_BLOB` means the route source bytes were recovered from Git and match the online snapshot. It is **not** proof of the judge compile unit.

Record separately in `source-meta.json`:

- `route_source_status` / `route_source_sha256`
- `judge_payload_status` (`EXACT_VERIFIED` | `TRANSFORMED_VERIFIED` | `UNVERIFIED` | `MISSING`)
- `judge_payload_sha256`
- `judge_payload_generation`

CANNJudge upload puts the source into `files[0].path=kernel.asc`. The judge then compiles a multi-file template whose `main.asc` `#include "kernel.asc"`. Prefer `TRANSFORMED_VERIFIED` when that wrapper is documented. `result.json` `source.sha256` is a submit-client hash of uploaded content, not a judge-returned digest. See `phase4/control/judge-payload-provenance-audit.md`.
