# Worktree Integration Inventory

## Final closeout status

`INTEGRATION_BRANCH_RECONCILED=YES`。本报告更新提交位于 Integration 工作树 `/Users/sunyiyang/Desktop/Project/cann/worktrees/integration/w4-closeout-20261008`，分支 `integration/w4-closeout-20261008`；更新前 HEAD=`6ec8c174c7352677ff623e760d6ddd55dc48e84e`。

`PRIMARY_MAIN_UPDATED=NO`；`PRIMARY_MAIN_UPDATE=DEFERRED_DIRTY_17_AND_PENDING_REVIEW`；`MAIN_MERGE_BLOCKED=YES`。Primary main HEAD=`e7f669692a2f5815c4ce444cfb52dde1d24a93a2`，仍有 9 tracked dirty + 8 untracked，未修改；当前没有获授权的主线写入者。`origin/main=1ea9677ba8e7307a73f13041c7b639ab6a96425c`。

Evidence copy completed for W4 R01-R15 where available。Candidate 与生产 kernel 未合入。总核验 `1566 manifest rows = 1097 included + 469 excluded`；`1097/1097 source/archive blobs 一致`；cross-batch source key duplicates=`0`；archive selected source extensions `.asc/.c/.cc/.cpp/.cxx/.h/.hpp`=`0`。

| Batch | Commit(s) | Files | Manifest rows | Included | Excluded |
|---|---|---:|---:|---:|---:|
| Batch-01 Research | `c235eb00` | 140 | 164 | 139 | 25 |
| Batch-02 R01/R02 | `4df78bc6`, `97cdf851` | 360 | 484 | 359 | 125 |
| Batch-03 R07 | `c9ff35a7` | 3 | 2 | 2 | 0 |
| Batch-04 R08/R09 | `cf95a3d7`, `4fb3441c` | 88 | 219 | 87 | 132 |
| Batch-05 R10/R11/R14 | `6ec8c174` | 511 | 697 | 510 | 187 |
| Total | 7 commits | 1102 | 1566 | 1097 | 469 |

`Files` 为该批次归档路径下的 tracked 文件数，包含 manifest；纳入/排除数按 manifest 行统计。

所有 33 个注册 worktree 均保留，`SAFE_TO_RETIRE=NO`。原因包括 Route lifecycle 未被 Planning 关闭、Candidate branch 仍需保留、W2/W3 独有 evidence 未全部整合、R02/R08/R09/R14 旧 runtime/command UNKNOWN、R01/W3-crossrow/M2 ignored evidence、external detached dirty checkout，以及 primary dirty。

本轮没有运行 Compile、Correctness、Local、NPU、Online；没有删除、clean、reset、rebase、force-push；`PUSH=NO`。

盘点时间：2026-10-09（最终收口）。纳入 R14、R04、W2 Tiny、W2 Selective 关闭回执、Main 状态回执、服务器只读补充、Record Owner 两个规则提交、三次 local/origin main merge 回执及本轮五批 evidence copy。最新可用 Main 图状态回执：HEAD=`e7f669692a2f5815c4ce444cfb52dde1d24a93a2`，ahead 16 / behind 2；Main 只读状态回执确认 9 tracked + 8 untracked、未暂存，精确路径见下文。未进入 primary main，未读取或复制 dirty 内容。最终报告提交前 Integration HEAD=`6ec8c174c7352677ff623e760d6ddd55dc48e84e`。

工作位置：`/Users/sunyiyang/Desktop/Project/cann/worktrees/integration/w4-closeout-20261008`。本报告覆盖 `git worktree list` 的全部 33 个注册点：32 个项目工作树（含 primary `main` 与本 integration 工作树），以及 1 个仓库外 detached Codex 检出 `/Users/sunyiyang/.codex/worktrees/f0e7/cann`。外部检出的 owner、agent、command 未知；dirty 仅记录回执计数 9 tracked + 8 untracked，不查看其未提交内容，也不移除。primary `main` 未进入。其他工作树只使用 worktree 元数据、Git 提交对象、允许的状态名称或用户/共享记录回执；未读取其他工作树的未提交文件内容。

`BASE_COMMIT` 是该 HEAD 与本地 `main` 的 merge-base。`UNIQUE_COMMITS` 是 `main..HEAD` 的提交数；长列表可由该范围复现，R14 和本轮有更新的两条 W2 分支另列完整信息。`MERGED_EVIDENCE` 记 `git cherry main HEAD` 的 patch-equivalent 提交数；`UNMERGED_EVIDENCE` 记尚未等价出现在 main 的提交数。`KERNEL_DIFF` 是 base 到 HEAD 的源文件路径数，含 Candidate、Parent probe 与 runner 支持源；不等同 Candidate 文件数。

`SAFE_TO_CLOSE` 只判断 Agent/运行上下文是否可以结束，要求证据足以说明没有在途命令。`SAFE_TO_RETIRE` 判断工作树是否可从后续收口工作中退役，不表示允许删除工作树或分支。本轮没有删除、归档或关闭任何工作树/分支。

## Inventory

以下 TSV 每行一份项目工作树，路径及状态字符串均按可见 Git 元数据或来源回执记录。

```tsv
WORKTREE_PATH	ROUTE/OWNER	BRANCH	HEAD	BASE_COMMIT	UNIQUE_COMMITS	TRACKED_DIRTY	UNTRACKED	IGNORED_EVIDENCE	ACTIVE_AGENT	RUNNING_COMMAND	MERGED_EVIDENCE	UNMERGED_EVIDENCE	KERNEL_DIFF	SAFE_TO_CLOSE	SAFE_TO_CLOSE_BASIS	SAFE_TO_RETIRE	SAFE_TO_RETIRE_BASIS	BLOCK_REASON
/Users/sunyiyang/Desktop/Project/cann	PRIMARY main	main	e7f669692a2f5815c4ce444cfb52dde1d24a93a2 (latest graph receipt; exact-path receipt does not refresh HEAD); de70b634813dea80783fc57716d6e95c158edeec	0 vs itself; 16 ahead / 2 behind origin at latest graph receipt	9 tracked + 8 untracked, none staged; exact names in Main dirty-state receipt below	8 exact paths from latest read-only Main receipt below	UNKNOWN	UNKNOWN	UNKNOWN	0	0	0	NO	primary not entered; no shutdown receipt	NO	primary dirty state preserved; outside permitted worktree	MAIN_DIRTY_STATE_PRESERVED=YES; 17 dirty entries; INTEGRATION_BRANCH_RECONCILED=YES; PRIMARY_MAIN_UPDATED=NO; PRIMARY_MAIN_UPDATE=DEFERRED_DIRTY_17_AND_PENDING_EVIDENCE_REVIEW
/Users/sunyiyang/.codex/worktrees/f0e7/cann	External detached / UNKNOWN	DETACHED	8f9338f8d01230635e617ff037e5951bb77bf67f	8f9338f8d01230635e617ff037e5951bb77bf67f	0	9 tracked (names/content not read)	8 untracked (names/content not read)	UNKNOWN	UNKNOWN	UNKNOWN	0	0	0	NO	owner/agent/command UNKNOWN; no close receipt	NO	external detached checkout remains registered; no removal authorized	external dirty registration retained
/Users/sunyiyang/Desktop/Project/cann/worktrees/integration/w4-closeout-20261008	Integration Owner	integration/w4-closeout-20261008	6ec8c174c7352677ff623e760d6ddd55dc48e84e (before final report commit)	e7f669692a2f5815c4ce444cfb52dde1d24a93a2	16 unique commits vs main (13 patch-unique)	2 report paths modified for final commit	0	0	current Integration Owner	no device command started here; global server3 attribution UNKNOWN	0 patch-equivalent commits	13 patch-unique commits by git cherry	0	YES	closeout report commit contains only the two authorized report paths	NO	all 33 registrations retained; evidence, Candidate branches and unresolved Route lifecycle preserved	no merge conflict; PRIMARY_MAIN_UPDATED=NO
/Users/sunyiyang/Desktop/Project/cann/worktrees/m2/vector	VECTOR-MATH-X	m2/vector-math	4320c8382b4c6846e137edc0e8243d744ad82f49	7fd752f75fb38c3f95dd29374ea79cb8375e03c3	10	0	0	78 ignored entries under 本地实验	UNKNOWN	UNKNOWN	0 patch-equivalent	10 unique commits	2 source paths: S3-TAIL-PROBE/tail_probe.asc; V003/submission.asc	NO	no current owner shutdown or running-command receipt	NO	10 unique commits and Candidate evidence remain outside main	separate VECTOR-MATH-X workstream; not W4 closeout scope
/Users/sunyiyang/Desktop/Project/cann/worktrees/support/arch-hardware	Support / arch-hardware	support/arch-hardware	503b98cb22ae88798bb6bfca83a46676933f3600	503b98cb22ae88798bb6bfca83a46676933f3600	0	0	0	0	UNKNOWN	UNKNOWN	0 patch-equivalent	0	0	NO	owner/runtime shutdown not established by current receipt	NO	retirement awaits Support owner completion confirmation	owner status UNKNOWN
/Users/sunyiyang/Desktop/Project/cann/worktrees/support/community-intelligence	Support / community-intelligence	support/community-intelligence	503b98cb22ae88798bb6bfca83a46676933f3600	503b98cb22ae88798bb6bfca83a46676933f3600	0	0	0	0	UNKNOWN	UNKNOWN	0 patch-equivalent	0	0	NO	owner/runtime shutdown not established by current receipt	NO	retirement awaits Support owner completion confirmation	owner status UNKNOWN
/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/case14-intrarow-parallelism	W2 / CASE14	w2/m1/case14-intrarow-parallelism	2c1ad6d67b3fbb9e59304d47f98d6a24a18992e8	3711b4ea4af0847ebd92b5dc764ff056e5ca2a3b	7	SNAPSHOT_PENDING	SNAPSHOT_PENDING	UNKNOWN	UNKNOWN	UNKNOWN	0 patch-equivalent	7 unique commits	0	NO	current Agent and command state were not refreshed	NO	historical work remains only in its branch; keep until evidence references are consolidated	W2 current status snapshot not available
/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/case47-small-cluster	W2 / CASE47	w2/m1/case47-small-cluster	e790de82aa023ded6f464345764ac7297d6d8cc2	3711b4ea4af0847ebd92b5dc764ff056e5ca2a3b	26	SNAPSHOT_PENDING	SNAPSHOT_PENDING	UNKNOWN	UNKNOWN	UNKNOWN	0 patch-equivalent	26 unique commits	7 source paths	NO	current Agent and command state were not refreshed	NO	26 unique commits and Candidate evidence remain outside main	W2 current status snapshot not available
/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/selective-fastpath	W2 / SELECTIVE-FASTPATH	w2/m1/selective-fastpath	36756e8a750bf41628286213ca951e183899d5b1	ed860e392d7604694ac6664da60aff1fc1f4c04f	14 vs main; +1 vs origin per owner receipt	0 (owner-verified clean)	0 (prior untracked file is committed)	UNKNOWN	CLOSED (Main close receipt; creator identity UNKNOWN)	NONE in close receipt; global server3 process attribution UNKNOWN	0 patch-equivalent	14 unique commits	11 source paths	YES (Agent only)	Main confirms Agent close; close receipt reports no task command	NO	correctness evidence path awaits integration disposition; creator identity UNKNOWN	evidence integration pending; retirement not authorized
/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/sync-topology	W2 / SYNC	w2/m1/sync-topology	1d3eb27f6bf07139fc94f28b7a02fb4e982e410e	ed860e392d7604694ac6664da60aff1fc1f4c04f	47	SNAPSHOT_PENDING	SNAPSHOT_PENDING	UNKNOWN	UNKNOWN	UNKNOWN	0 patch-equivalent	47 unique commits	7 source paths	NO	current Agent and command state were not refreshed	NO	47 unique commits and experiment evidence remain outside main	W2 current status snapshot not available
/Users/sunyiyang/Desktop/Project/cann/worktrees/w2/m1/tiny-fixed-overhead	W2 / TINY-FIXED-OVERHEAD	w2/m1/tiny-fixed-overhead	f1868765509966ca829e6b3456135e237fd669fd	3711b4ea4af0847ebd92b5dc764ff056e5ca2a3b	15 vs main; +1 vs origin per owner receipt	0 (owner-verified clean)	0 (prior untracked file is committed)	UNKNOWN	CLOSED (Main close receipt; creator identity UNKNOWN)	REMOTE_DETACHED_TASK=UNKNOWN	0 patch-equivalent	15 unique commits	8 source paths, including Parent codegen support wrapper	YES (Agent only)	Main confirms Agent close; detached remote task remains unverified	NO	Parent codegen evidence and detached remote state remain unresolved	creator identity UNKNOWN; do not retire
/Users/sunyiyang/Desktop/Project/cann/worktrees/w3/m1/adaptive-core-ownership	W3 / R2	w3/m1/adaptive-core-ownership	6321ad4427b1819d172c719ebde190eb447ce9ae	503b98cb22ae88798bb6bfca83a46676933f3600	42	0	0	0	UNKNOWN	UNKNOWN	0 patch-equivalent	42 unique commits	280 source paths	NO	no current Agent shutdown receipt	NO	42 unique commits and historical Candidate/evidence remain in branch	W3 historical route; preserve committed record
/Users/sunyiyang/Desktop/Project/cann/worktrees/w3/m1/crossrow-full-pipeline	W3 / R5	w3/m1/crossrow-full-pipeline	1efa0863611f1a9b76ab13dd361eba161b32b46d	503b98cb22ae88798bb6bfca83a46676933f3600	29	0	0	531 ignored entries under 管理	UNKNOWN	UNKNOWN	0 patch-equivalent	29 unique commits	252 source paths	NO	no current Agent shutdown receipt	NO	29 unique commits and historical Candidate/evidence remain in branch	W3 historical route; preserve committed record and ignored evidence
/Users/sunyiyang/Desktop/Project/cann/worktrees/w3/m1/multirow-panel-rms	W3 / R4	w3/m1/multirow-panel-rms	ce6c6dc568256ac8b80b674096bd7ca081b897cb	503b98cb22ae88798bb6bfca83a46676933f3600	34	0	0	0	UNKNOWN	UNKNOWN	0 patch-equivalent	34 unique commits	186 source paths	NO	no current Agent shutdown receipt	NO	34 unique commits and historical Candidate/evidence remain in branch	W3 historical route; preserve committed record
/Users/sunyiyang/Desktop/Project/cann/worktrees/w3/m1/phase3-integration	W3 integration	w3/m1/phase3-integration	1ea9677ba8e7307a73f13041c7b639ab6a96425c	de70b634813dea80783fc57716d6e95c158edeec	2	0	0	0	UNKNOWN	UNKNOWN	0 patch-equivalent	2 unique commits	0	NO	no current Agent shutdown receipt	NO	2 W3 record commits require decision alongside main/origin/main reconciliation	main/origin/main divergence includes W3 calibration commits
/Users/sunyiyang/Desktop/Project/cann/worktrees/w3/m1/record-owner	W3 Record Owner	w3/m1/record-owner	cecec26bd852540c52b1e932cba7b9cd041331cc	503b98cb22ae88798bb6bfca83a46676933f3600	28	0	0	0	UNKNOWN	UNKNOWN	0 patch-equivalent	28 unique commits	0	NO	no current Agent shutdown receipt	NO	28 record commits need path-level comparison against current main records	record branch carries W4 history and may overlap canonical records
/Users/sunyiyang/Desktop/Project/cann/worktrees/w3/m1/tiny-minimal-kernel	W3 / R3	w3/m1/tiny-minimal-kernel	584c590cef81349969c4dcf014a5c04d05032e9d	503b98cb22ae88798bb6bfca83a46676933f3600	12	0	0	0	UNKNOWN	UNKNOWN	0 patch-equivalent	12 unique commits	60 source paths	NO	no current Agent shutdown receipt	NO	12 unique commits and historical Candidate/evidence remain in branch	W3 historical route; preserve committed record
/Users/sunyiyang/Desktop/Project/cann/worktrees/w3/m1/ub-bank-layout	W3 / R1	w3/m1/ub-bank-layout	64e32f535348cdde1828cd8a8fd89fad100aaa33	503b98cb22ae88798bb6bfca83a46676933f3600	12	0	0	0	UNKNOWN	UNKNOWN	0 patch-equivalent	12 unique commits	50 source paths	NO	no current Agent shutdown receipt	NO	12 unique commits and historical Candidate/evidence remain in branch	W3 historical route; preserve committed record
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R01-selective-param-pipeline-x	W4-R01 / selective parameter pipeline	w4/r01-selective-param-pipeline-x	4b9dcade629bd7e0fdbe3194c740bd1c7824d107	de70b634813dea80783fc57716d6e95c158edeec	5	0	0	71 ignored entries under 本地实验	NONE_REPORTED	NONE_REPORTED	0 patch-equivalent	5 unique commits	14 source paths	YES	latest task row is queued and worktree status names are clean	NO	W4 route lifecycle has no close decision; Candidate history remains in its route branch	W4 route lifecycle remains open
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R02-selective-tile-traversal-x	W4-R02 / selective tile traversal	w4/r02-selective-tile-traversal-x	e100b5d9b17e8b95ee5e70fcbbef19d022db6ca0	de70b634813dea80783fc57716d6e95c158edeec	2	SNAPSHOT_PENDING	SNAPSHOT_PENDING	UNKNOWN	UNKNOWN (old agent ID not_found)	UNKNOWN (current stage/command)	0 patch-equivalent	2 unique commits	7 source paths	NO	old ID not_found is not a close receipt; current stage UNKNOWN	NO	W4 route lifecycle remains open; no retirement decision	R02 runtime and remote command UNKNOWN
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R03-owner-occupancy-guard-x	W4-R03 / owner occupancy guard	w4/r03-owner-occupancy-guard-x	5e95f9bc9d9c44fab03aecf9c84c29c0b947614b	de70b634813dea80783fc57716d6e95c158edeec	2	0	0	0	NONE_REPORTED	NONE_REPORTED	0 patch-equivalent	2 unique commits	0	YES	latest task row is queued; clean status-name snapshot	NO	Planning has not changed the W4 route lifecycle	ROUTE_REVIEW_REQUIRED in shared matrix
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R04-ub-bank-xr-layout-x	W4-R04 / UB-bank XR layout	w4/r04-ub-bank-xr-layout-x	766bb53acb85db70a4c2648feae72511ac20a10e	de70b634813dea80783fc57716d6e95c158edeec	4	0 (owner-verified clean)	0 (owner-verified clean)	UNKNOWN	CLOSED (Main close receipt)	NONE in close receipt; global server3 process attribution UNKNOWN	0 patch-equivalent	4 unique commits; latest 766bb53 adds 93 evidence files, no Candidate	9 source/support paths; latest commit adds probe helpers, not Candidate	YES (Agent only)	Main confirms Agent close; closeout and clean HEAD confirmed	NO	93 timing/profiler evidence files still require integration disposition	retirement awaits evidence integration
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R05-ub-bank-param-out-layout-x	W4-R05 / UB-bank parameter/output layout	w4/r05-ub-bank-param-out-layout-x	1a31a3b527d81d4db0fce20dda091b85889330b6	de70b634813dea80783fc57716d6e95c158edeec	2	0	0	0	NONE_REPORTED	NONE_REPORTED	0 patch-equivalent	2 unique commits	7 source paths	YES	latest task row is queued and worktree status names are clean	NO	W4 route lifecycle has no close decision; Candidate history remains in its route branch	W4 route lifecycle remains open
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R06-mte2-xres-issue-x	W4-R06 / MTE2 x/res issue	w4/r06-mte2-xres-issue-x	b40c09e62adcd85f51258ed1cef3723b80b010c8	de70b634813dea80783fc57716d6e95c158edeec	3	0	0	0	NONE_REPORTED	NONE_REPORTED	0 patch-equivalent	3 unique commits	0	YES	latest task row is queued; clean status-name snapshot	NO	W4 route lifecycle has no close decision	W4 route lifecycle remains open
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R07-mte3-store-queue-x	W4-R07 / MTE3 store queue	w4/r07-mte3-store-queue-x	bca492a32476d579f44b60516f8aeaac6e36b6d5	de70b634813dea80783fc57716d6e95c158edeec	2	0	0	0	NONE_REPORTED	NONE_REPORTED	0 patch-equivalent	2 unique commits	7 source paths	YES	latest task row is queued and worktree status names are clean	NO	W4 route lifecycle has no close decision; Candidate history remains in its route branch	W4 route lifecycle remains open
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R08-crossrow-prefetch-x	W4-R08 / cross-row prefetch	w4/r08-crossrow-prefetch-x	1bc84959fbc5dea190ad06a3dcef8b8b3005cae1	de70b634813dea80783fc57716d6e95c158edeec	5	SNAPSHOT_PENDING	SNAPSHOT_PENDING	UNKNOWN	UNKNOWN (old agent ID not_found)	UNKNOWN (current stage; RUNNING_DEVICE_OPERATION=UNKNOWN)	0 patch-equivalent	5 unique commits	7 source paths	NO	old ID not_found is not a close receipt; current stage UNKNOWN	NO	W4 route lifecycle remains open; no retirement decision	R08 runtime and remote command UNKNOWN
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R09-event-barrier-min-x	W4-R09 / event barrier	w4/r09-event-barrier-min-x	ec24f3ac550d3539df5cb1dca3d49c55634f9430	de70b634813dea80783fc57716d6e95c158edeec	3	SNAPSHOT_PENDING	SNAPSHOT_PENDING	UNKNOWN	UNKNOWN (old agent ID not_found)	UNKNOWN (current stage; RUNNING_DEVICE_OPERATION=UNKNOWN)	0 patch-equivalent	3 unique commits	7 source paths	NO	old ID not_found is not a close receipt; current stage UNKNOWN	NO	W4 route lifecycle remains open; no retirement decision	R09 runtime and remote command UNKNOWN
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R10-active-core-d-aware-x	W4-R10 / active-core D-aware	w4/r10-active-core-d-aware-x	205ef6702b37101bbebc4d900dc889bea679ee25	de70b634813dea80783fc57716d6e95c158edeec	5	0	0	0	NONE_REPORTED	NONE_REPORTED	0 patch-equivalent	5 unique commits	13 source paths	YES	latest task row is queued; clean status-name snapshot	NO	W4 route lifecycle has no close decision; Candidate history remains in its route branch	W4 route lifecycle remains open
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R11-row-remainder-balance-x	W4-R11 / row remainder balance	w4/r11-row-remainder-balance-x	1b528176920d57936a9a889c13c3af8293b2a85b	de70b634813dea80783fc57716d6e95c158edeec	4	0	0	0	NONE_REPORTED	NONE_REPORTED	0 patch-equivalent	4 unique commits	7 source paths	YES	latest task row is queued; clean status-name snapshot	NO	W4 route lifecycle has no close decision; Candidate history remains in its route branch	W4 route lifecycle remains open
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R12-tiny-dispatch-minimal-x	W4-R12 / tiny dispatch minimal	w4/r12-tiny-dispatch-minimal-x	e697ddf35d4c42e735236e597338a148bfb437f2	de70b634813dea80783fc57716d6e95c158edeec	2	0	0	0	NONE_REPORTED	NONE_REPORTED	0 patch-equivalent	2 unique commits	2 source paths	YES	latest task row is queued; clean status-name snapshot	NO	W4 route lifecycle has no close decision	W4 route lifecycle remains open
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R13-wide-fp32-cache-tail-x	W4-R13 / wide FP32 cache tail	w4/r13-wide-fp32-cache-tail-x	f417e4361ddac6b626ced1e62cfb75de8e3695ee	de70b634813dea80783fc57716d6e95c158edeec	3	0	0	0	NONE_REPORTED	NONE_REPORTED	0 patch-equivalent	3 unique commits	11 source paths	YES	latest task row is queued; clean status-name snapshot	NO	W4 route lifecycle has no close decision	W4 route lifecycle remains open
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R14-param-dma-granularity-x	W4-R14 / parameter DMA granularity	w4/r14-param-dma-granularity-x	64bb1b04c02aee792179e6e75c9f4ffb655d49e1	de70b634813dea80783fc57716d6e95c158edeec	4 unique commits: db11b221, fcbd1814, 5a49704f, 64bb1b04	0 (owner-verified clean)	0 (owner-verified clean)	0	UNKNOWN (old agent ID not_found)	UNKNOWN (current stage; RUNNING_DEVICE_OPERATION=UNKNOWN)	0 patch-equivalent	4 unique commits	7 Parent fingerprint harness paths in receipt; no Candidate path	NO	old ID not_found; current stage and running operation UNKNOWN	NO	Parent fingerprint evidence still needs integration disposition; Route lifecycle remains open	R14 runtime UNKNOWN; do not infer close
/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R15-safe-multirow-dma-x	W4-R15 / safe multirow DMA	w4/r15-safe-multirow-dma-x	744f3ad313f7586c8368a0765021ba3cac8c138e	de70b634813dea80783fc57716d6e95c158edeec	3	0	0	0	NONE_REPORTED	NONE_REPORTED	0 patch-equivalent	3 unique commits	0	YES	latest task row has no active slot; clean status-name snapshot	NO	route review awaits Planning; do not infer lifecycle completion from Agent closure	ROUTE_REVIEW_REQUIRED
```

### R14 receipt refresh

Owner receipt at 2026-10-09: branch `w4/r14-param-dma-granularity-x`, HEAD `64bb1b04c02aee792179e6e75c9f4ffb655d49e1`, clean, seven previously untracked files are now committed. `BASE_COMMIT=de70b634813dea80783fc57716d6e95c158edeec`; the four unique commits are:

1. `db11b221635f630ead503fabce87b4c6cf9f92a2` — V001 revision-readiness note.
2. `fcbd1814764337b89bc3b9fb6950d37241feab5a` — Parent MTE2 fingerprint and timing limits.
3. `5a49704ff4d67428dfd42954e9291fcb1c2f823a` — Parent task/event mapping.
4. `64bb1b04c02aee792179e6e75c9f4ffb655d49e1` — Parent MTE2 profiling harness; adds exactly:
   - `本地实验/W4-R14/V001/support/param-mte2-fingerprint/CMakeLists.txt`
   - `本地实验/W4-R14/V001/support/param-mte2-fingerprint/kernel.asc`
   - `本地实验/W4-R14/V001/support/param-mte2-fingerprint/main.asc`
   - `本地实验/W4-R14/V001/support/param-mte2-fingerprint/profile_16x16384_fp16.sh`
   - `本地实验/W4-R14/V001/support/param-mte2-fingerprint/profile_1x32768_fp32.sh`
   - `本地实验/W4-R14/V001/support/param-mte2-fingerprint/profile_2x8192_fp32.sh`
   - `本地实验/W4-R14/V001/support/param-mte2-fingerprint/profile_3x6144_fp32.sh`

These are Parent/probe support files. The report records no Candidate path in this commit. The latest Agent shutdown state remains UNKNOWN.

### W2 owner receipt refresh

Both closeout commits are now present in worktree metadata and Git objects. Their one-file commits turn the previously untracked paths into committed evidence; file creation identity remains UNKNOWN.

| Route | HEAD / unique-to-main count | New commit | New path | Current status | SAFE_TO_CLOSE | SAFE_TO_RETIRE |
|---|---|---|---|---|---|---|
| W2 Tiny | `f1868765509966ca829e6b3456135e237fd669fd` / 15 | `f1868765509966ca829e6b3456135e237fd669fd` | `本地实验/TINY-FIXED-OVERHEAD-CHAMPION-X/V002-parent-loop-proof/support/parent_codegen_wrapper.asc` | clean per owner receipt; prior untracked item committed | NO — remote detached process state UNVERIFIED | NO — evidence not yet integrated |
| W2 Selective | `36756e8a750bf41628286213ca951e183899d5b1` / 14 | `36756e8a750bf41628286213ca951e183899d5b1` | `本地实验/SELECTIVE-FASTPATH-CHAMPION-X/V001/support/run_correctness.sh` | clean per owner receipt; prior untracked item committed; source report references `correctness-run-001` | YES — closeout complete, clean HEAD, no running command reported | NO — evidence not yet integrated |

### R04 owner receipt refresh

R04 closeout is complete and independently verified per user receipt. Branch `w4/r04-ub-bank-xr-layout-x` is clean at `766bb53acb85db70a4c2648feae72511ac20a10e`. Its four unique commits from base `de70b634813dea80783fc57716d6e95c158edeec` are `b6890ad19bd60560fdd9febce98760078982e51f`, `8c3b6659014e7904c1b14371e24045e68aaa8bee`, `8ffc6b6962921dfcd4d9912185eebfc940fe85a0`, and `766bb53acb85db70a4c2648feae72511ac20a10e`. The latest commit adds 93 original V002 timing-attribution/profiler evidence files, including 768 samples, per-call task mapping and raw profiler output; the latest commit changes no Candidate. The older `8ffc6b...` commit is retained in B because it contains the R04 Candidate revision.

`SAFE_TO_CLOSE=YES` for the R04 Agent because the closeout and clean status were independently verified. `SAFE_TO_RETIRE=NO` until the 93 evidence files have an integration disposition. Existing raw exports have whitespace/trailing-empty-line warnings under `git diff --check`; they remain byte-for-byte unchanged in the source branch and are not included in this report commit.

### Main dirty-state receipt

`MAIN_DIRTY_STATE_PRESERVED=YES`; dirty total remains 17 (9 tracked + 8 untracked), none staged. The latest read-only Main status receipt provides the exact names below. The latest available graph receipt remains HEAD `e7f669692a2f5815c4ce444cfb52dde1d24a93a2`, ahead 16 / behind 2 versus `origin/main`; the path-only receipt did not state a newer HEAD. Main worktree contents were not read, copied, overwritten, staged, or modified.

Tracked modified paths (9): `工具/cannjudge-submit.mjs`; `归档/README.md`; `归档/任务看板/index.html`; `技术路线/全版本记录.tsv`; `技术路线/技术路线图.md`; `技术路线/路线成绩表.tsv`; `研究/主代理/MAIN-1-W2/campaign-status.md`; `调度/当前任务.tsv`; `调度/服务器设备使用.tsv`.

Untracked paths (8): `.claude/agents/dashboard-builder.md`; `CANN西南赛区_群聊情报交接文档.docx`; `CANN西南赛区_群聊情报交接文档.md`; `归档/任务看板/index.template.html`; `归档/任务看板/refresh.mjs`; `归档/任务看板/submit-events.tsv`; `提分技术讨论_原文.csv`; `提分技术讨论_清单.md`. These names come only from the Main read-only status receipt. No contents were read, copied, overwritten, staged, or changed.

### Remote process and Agent-state receipt

At one read-only `cann-server3` snapshot, `ps/screen/tmux` showed transient user-owned `clx_ref_parent_` PID `2250021`, elapsed about 6 seconds. An immediate `/proc/<pid>` cwd/exe lookup found it gone. Route attribution is UNKNOWN: `SERVER3_TRANSIENT_PROCESS_UNATTRIBUTED=clx_ref_parent_`. Global remote `RUNNING_COMMAND` is not safely reported as NONE. The local process scan found no matching Compile/Correctness/Local/profiler command; that does not establish remote quiescence or make any worktree eligible for retirement. No other-user process was inspected or stopped.

Runtime lookup returned `not_found` for the older schedule agent IDs associated with W4-R02/R05/R08/R09/R12/R14. This is not a close receipt. Current-stage and running-operation fields for R02/R08/R09/R14 are UNKNOWN; no Route is closed from this snapshot. R04 and W2 Selective have separate Main Agent close receipts; W2 Tiny Agent is closed while its detached remote task remains UNKNOWN. Agent closure and physical worktree retirement are separate decisions; every current retirement recommendation remains NO.

| Route | ACTIVE_AGENT snapshot | RUNNING_COMMAND snapshot | SAFE_TO_CLOSE | SAFE_TO_RETIRE |
|---|---|---|---|---|
| W4-R02 | older task ID `not_found`; ACTIVE_AGENT=UNKNOWN | current stage UNKNOWN; `RUNNING_DEVICE_OPERATION=UNKNOWN`; local scan no match | NO | NO |
| W4-R05 | older task ID `not_found` | local scan no match; server3 UNKNOWN | NO | NO |
| W4-R08 | older task ID `not_found`; ACTIVE_AGENT=UNKNOWN | current stage UNKNOWN; `RUNNING_DEVICE_OPERATION=UNKNOWN`; local scan no match | NO | NO |
| W4-R09 | older task ID `not_found`; ACTIVE_AGENT=UNKNOWN | current stage UNKNOWN; `RUNNING_DEVICE_OPERATION=UNKNOWN`; local scan no match | NO | NO |
| W4-R12 | older task ID `not_found` | local scan no match; server3 UNKNOWN | NO | NO |
| W4-R14 | old ID `not_found`; current row says ACTIVE_AGENT=UNKNOWN | `RUNNING_DEVICE_OPERATION=UNKNOWN`; global attribution UNKNOWN | NO | NO |

### Ignored evidence / retirement holds

| Worktree | Ignored evidence snapshot | Retirement |
|---|---|---|
| W4-R01 | 71 ignored profiler files | `SAFE_TO_RETIRE=NO`; ignored files remain untouched |
| W3 crossrow full-pipeline | 531 ignored Chromium profile files | `SAFE_TO_RETIRE=NO`; ignored files remain untouched |
| M2 vector | 78 ignored result files | `SAFE_TO_RETIRE=NO`; ignored files remain untouched |

Local `pgrep` found no matching device command, but the remote `clx_ref_parent_` process was briefly visible and unattributed. Do not infer retirement eligibility from a clean tracked status or local process scan.

### Latest Main merge receipt

Main commit `e7f669692a2f5815c4ce444cfb52dde1d24a93a2` (parent `566e329e31f0b971e0c66bebeb60f73e467ee7b7`) was merged into integration by `6433b48cea810881530db6126fbefe5bddfe4914`. It updates four shared-record paths only. Latest Main state: 9 tracked dirty + 8 untracked, none staged; exact current path names UNKNOWN. `origin/main=1ea9677ba8e7307a73f13041c7b639ab6a96425c`; directional count local ahead 16 / behind 2. `MERGE_HEAD=ABSENT`; this merge had no conflicts.

### Required policy commits

`e7c553ec0da4f96af461baa0dfdb02d4730f8fc7` is the Record Owner policy commit (parent `82a45cb2db775d6b6787450c1e0c914aa7dbdefb`), changing seven policy files: `AGENTS.md`; `.agents/skills/cann-main-orchestrator/SKILL.md`; `.agents/skills/cann-online-owner/SKILL.md`; `项目规则/W4持续探索控制契约.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/线上提交规范.md`. Follow-up `566e329e31f0b971e0c66bebeb60f73e467ee7b7` changes only wording in `.agents/skills/cann-online-owner/SKILL.md`.

## Snapshot limits

- R02, R08 and R09 still use their last dispatch receipts; current command state is UNKNOWN, so their status fields are `SNAPSHOT_PENDING`.
- W2 CASE14, CASE47 and SYNC were not refreshed after the two named closeout tasks; their status fields are `SNAPSHOT_PENDING`.
- Main was not entered. Dirty path names come from the `82a45` Main receipt; at `566e`, only the total count 17 is confirmed. The main/origin graph and conflict preview were read from Git objects in the integration worktree.
- Ignored evidence is counted where enumerated. `UNKNOWN` means ignored entries were not enumerated; it does not mean none exist.
- The exact W4 authorization and six qualification items are captured in the updated Evidence Integration Plan from commits `e7c553ec...` / `566e329e...`. This Integration Owner performs no Online action.
