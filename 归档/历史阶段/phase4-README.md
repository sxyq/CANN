# Phase4

## Execution contract

BEFORE ANY ROUTE EXECUTION, READ:

1. `phase4/control/project-experiment-playbook.md`
2. `phase4/control/execution-contract.md`
3. `phase4/control/local-timing-protocol.md`

These are the shared execution and measurement authorities; new Route prompts need not repeat their full policy.

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
└── EXPLORE
    ├── MAIN-1
    │   ├── MIX-A-V003 44.69
    │   ├── WIDE-X-FRESH4
    │   ├── MODE-X-R015C
    │   ├── DTYPE-SPECIAL-X
    │   └── EXT-ASCEND-X (PARKED)
    └── MAIN-2 (six routes, worktrees consolidated 2026-09-26)
        ├── SCHED-ROWGROUP-X  OFFICIAL 15/15 22.27
        ├── UB-LIVENESS-X     JUDGE_READY, not submitted
        ├── ALIGN-TAIL-X
        ├── BATCH-RESIDENT-X
        ├── REDUCE-INVSCALE-X
        └── ASYNC-TRIPLE-X
```

R31A / R31B 是固定 EXPLOIT，不得写进 4 条 EXPLORE 方向。详见 `phase4/control/next-round-plan.md`。

EPI-X-FRESH is PARKED; retain its source and local result. Do not treat shared source-seed directories or previous Agent IDs as current ownership.

## MAIN-1 保留的 worktree

以下六个 MAIN-1 worktree 全部保留，本轮未触碰、未清理、未删除：

```text
/Users/sunyiyang/Desktop/Project/cann-sixlane/R31B
/Users/sunyiyang/Desktop/Project/cann-sixlane/R31A
/Users/sunyiyang/Desktop/Project/cann-sixlane/MIX-A
/Users/sunyiyang/Desktop/Project/cann-sixlane/WIDE-X-FRESH4
/Users/sunyiyang/Desktop/Project/cann-sixlane/MODE-X-R015C
/Users/sunyiyang/Desktop/Project/cann-sixlane/DTYPE-SPECIAL-X
```

加上 canonical workspace `/Users/sunyiyang/Desktop/Project/cann`。

`cann-sixlane/ASYNC-TRIPLE-X` 与 `cann-sixlane/EXT-ASCEND-X` 不属于 MAIN-1 保护集，
证据回收后已删除（见下节）。

## MAIN-2 六条 route 最终状态

| Route | Final Revision | Local verdict | Online status | Official result | Disposition | Evidence path |
|---|---|---|---|---|---|---|
| SCHED-ROWGROUP-X | V001 | `ONLINE_CANDIDATE`；same-binary PASS；P/C 33x100 median −51.38% | SUBMITTED | **15/15, 22.27**（parent 17.14，submission `6ab6c41b694b590c3ce2ef67`） | KEEP（正式结果高于 parent） | `phase4/online/SCHED-ROWGROUP-X/V001/`、`phase4/local/SCHED-ROWGROUP-X/V001/`、`phase4/workspaces/SCHED-ROWGROUP-X/` |
| UB-LIVENESS-X | V003（V001/V002 `LOCAL_REJECTED`） | `ONLINE_CANDIDATE`；correctness 全量 `bad=0` | `NOT_FORMALLY_SUBMITTED`（`JUDGE_READY`，无 `result.json`） | 无 | JUDGE_PENDING | `phase4/local/UB-LIVENESS-X/V003/`、`phase4/control/judge-handoff-ub-v003.md` |
| ALIGN-TAIL-X | V001 | `NEEDS_ONE_MORE_LOCAL` + `MEASUREMENT_BLOCKED`；warmup=45 same-binary PASS，4 组 P/C 方向不一致 | NOT_SUBMITTED | 无 | KEEP | `phase4/local/ALIGN-TAIL-X/V001/`、`phase4/workspaces/ALIGN-TAIL-X/` |
| BATCH-RESIDENT-X | V001 | `NEEDS_ONE_MORE_LOCAL` + `MEASUREMENT_BLOCKED`；window qual 2/2 FAIL，warmup=45 候选侧被污染 | NOT_SUBMITTED | 无 | KEEP | `phase4/local/BATCH-RESIDENT-X/V001/`、`phase4/workspaces/BATCH-RESIDENT-X/` |
| REDUCE-INVSCALE-X | V002（V001 `LOCAL_REJECTED`） | `NEEDS_ONE_MORE_LOCAL` + `MEASUREMENT_BLOCKED`；warmup=45 same-binary FAIL | NOT_SUBMITTED | 无 | KEEP | `phase4/local/REDUCE-INVSCALE-X/V001/`、`.../V002/`、`phase4/workspaces/REDUCE-INVSCALE-X/` |
| ASYNC-TRIPLE-X | V001 | `NEEDS_ONE_MORE_LOCAL` + `MEASUREMENT_BLOCKED`；window qual 2/2 FAIL | NOT_SUBMITTED | 无 | KEEP | `phase4/local/ASYNC-TRIPLE-X/V001/`、`phase4/workspaces/ASYNC-TRIPLE-X/` |

SCHED-ROWGROUP-X 的正式结果已在仓库内核实（`phase4/online/SCHED-ROWGROUP-X/V001/result.json`：
15/15、`officialScore` 22.27、`identityStatus=LOCAL_SHA_EQ_REMOTE_SHA`），
control 已同步，**本轮未再次提交**。

两条非 MAIN-1 历史路线：

| Route | Revision | 结论 | Disposition | Evidence path |
|---|---|---|---|---|
| ASYNC-TRIPLE-X（MAIN-1 copy） | V001 | parent 与 candidate 同为 507035，correctness 未评定，无 timing | HISTORICAL_ONLY | `phase4/archive/active-sixlane-20260924/ASYNC-TRIPLE-X/V001/` |
| EXT-ASCEND-X | V001 | `FIX_1..FIX_4` 后 FP32 27/27 仍 FAIL | PARK（`PARK_EXPLORE_SLOT`） | `phase4/local/EXT-ASCEND-X/V001/` |

## WORKTREE CONSOLIDATION STATUS

完整记录：`phase4/control/worktree-consolidation-20260926.md`，
删除前安全核对表：`phase4/control/WORKTREE_DELETION_PLAN.tsv`。

```text
已回收后删除（8）：
  cann-next6/ASYNC-TRIPLE-X      branch exp/next6-async-triple-x      @d5ab9a4
  cann-next6/BATCH-RESIDENT-X    branch exp/next6-batch-resident-x    @e95301e
  cann-next6/SCHED-ROWGROUP-X    branch exp/next6-sched-rowgroup-x    @b246c45
  cann-next6/REDUCE-INVSCALE-X   branch exp/next6-reduce-invscale-x   @d855cfc
  cann-next6/ALIGN-TAIL-X        branch exp/next6-align-tail-x        @ce4abd8
  cann-next6/UB-LIVENESS-X       branch exp/next6-ub-liveness-x       @ccd968e
  cann-sixlane/ASYNC-TRIPLE-X    branch exec/sixlane-20260924-async-triple-x @1632518
  cann-sixlane/EXT-ASCEND-X      branch exec/sixlane-20260923-ext-ascend-x   @a758f65

保留（7）：
  cann                          canonical, exp/independent-breadth
  cann-sixlane/R31B R31A MIX-A WIDE-X-FRESH4 MODE-X-R015C DTYPE-SPECIAL-X   MAIN-1 PROTECTED
```

为什么删：每条非 MAIN-1 路线的 A–H 资产（exact source / SHA / revision metadata /
build logs / correctness / local timing / online result / research handoff）都已先提交到
它自己的 route branch 并 push，再逐字节合入 canonical 对应位置并校验通过；
worktree 删除只删工作目录，不删任何路线证据，也不删任何 branch。

为什么留：canonical 是唯一整合点；六个 MAIN-1 worktree 是受保护的活跃工作树，
各自仍有未提交的 `local-result.json` 修改，与本轮收口无关。


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
✅ Six isolated Route Agents / worktrees / branches / contexts created and run
✅ Non-MAIN-1 worktree consolidation (2026-09-26) — see WORKTREE CONSOLIDATION STATUS
🔵 Next route tree decided by Main Review

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
