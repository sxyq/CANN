# LOCAL_FULL_STATE_REPORT

- 项目根：`/Users/sunyiyang/Desktop/Project/cann`
- 审计方式：只读全量检索（8 个并行子代理 + 主会话交叉核对）
- 审计时间：2026-09-18（本机）
- 约束：未删除/移动/重命名/覆盖文件；未执行 git reset/clean/rebase/merge/checkout；未发起任何 CANNJudge 在线提交；未输出任何 cookie/token/密码内容
- 证据分层：`[G]` Git 可验证事实 · `[F]` 文件/文档明确记录 · `[I]` 推断

---

# 1. Executive Summary

1. `[G]` 当前分支 `main`，HEAD `dfa0af0`（Validate V005 on server 3…，2026-09-17），领先 `origin/main` **2 个未推送提交**（`bd0b062`、`dfa0af0`）；无其他分支、无 stash、仅 1 个 worktree。
2. `[G]` 工作区未提交变更显著：6 个 modified、2 个 deleted（`scripts/README.md`、`调研/工具/cannjudge-browser-submit.js`）、约 24 个 untracked 文件；暂存区为空。
3. `[G]` 全部 `FULL-*` / `B0-PURE` / `PURE-*` / `R031` 时代证据只存在于 **未提交** 的 `对话全文.md` 工作副本（+17586 行）和 `.workbuddy/memory/2026-09-17.md`（+25 行）中；git HEAD 中 **零** FULL-R 内容。一次 checkout/stash 会丢掉 29.01 之后的全部记录。
4. `[F]` 本地对话归档结束于 **2026-09-17 22:25**（最后一行：“现在直接提交 FULL-R014-V001”），早于上游 FULL-R014（00:24:36）与 FULL-R016（02:20:54）结果。
5. `[F]` 对照上游 9 条：**6 条完全一致**（B0-PURE 17.52、PURE-R012 21.37、PURE-R009 17.64、PURE-R010 17.37、R031 38.36、FULL-R006 1/15 冻结）；**3 条本地缺失**（FULL-R014 20.09、FULL-R016 17.14+COMPILEFIX-A、FULL-R013 候选）。
6. `[F]` 全树对 `20.09`、`17.14`、`COMPILEFIX` 为 **零命中**；本地在线轨道 **落后于上游**，但在本地 git HEAD **之前**（工作区比 HEAD 新）。
7. `[F]` 磁盘上 **没有** 任何 `FULL-*` / `B0-PURE` / `PURE-R*` / `R031` 独立 kernel 文件；这些源码只嵌在 67,375 行的 `对话全文.md` 里（含 R031 全文 2,959 行、FULL-R014 全文、FULL-R006 生成脚本）。
8. `[F]` 本地另有 8 个不在上游清单的官方分：R029-V003=28.68、R029-V004=29.01、R006-PARTIALS=28.80、R016-V001=28.72、R030=28.62、R012-on-R031=36.69、R009-FASTPATH@R031=38.36、R010-SPLIT@R031=38.15。
9. `[F]` 所有官方分均 **未绑定 git commit**；RESULT 层普遍缺 `source_sha256` 与 `submission_id`（仅 H001 两次 CE 和旁路 311559/311894/316974 有 sub id）。
10. `[F]` 磁盘可哈希 kernel 共约 28–31 个，**仅一组字节相同**：H001/V005 `kernel.txt` ≡ `临时/服务器证据/V005/server3-kernel-20260917.txt`（`afd5b8ee…`）。
11. `[G][F]` H001 V003 身份断裂：文档描述 R001 two-pass，实际文件 ≈ 外部 `002-对照A-7680`，工作区还有未提交 `kTileElems 7680→8192`；原始 R001 V003 仅存于 git 历史（`92d10c0d…`，393 行）。
12. `[F]` 同名冲突：H002 与外部轨道两套 `MIX-R014-*` kernel/结果 SHA 全不同（相似度约 0.18–0.20）；`R029-V003` 双副本（沙箱 `aebe961d…` 837 行 vs 对话重建 `b1470ef5…` 2034 行）都写 28.68。
13. `[F]` 路线编号跨代复用：外部轨道 `R006/R013/R014/R016/R029` ≠ `FULL-R*` 代；仓库 `R029=官方Tiling五模式` ≠ 外部 `R029=wide cached-y`；`R016-V001=28.72` ≠ `FULL-R016=17.14`。
14. `[F]` 正式索引全部过期：`调研/路线最高分.md` 仍写“无有效平台分”；`管理/路线状态/R001–R029.json` 的 `official_score`/`champion` 全空；`提交/README` 外部最佳仍写 28.68/29.01。
15. `[F]` 不存在仓库级 `CURRENT_STATE*` / `AGENT_HANDOFF*` / `ROUTE_TREE*` / `RESULTS*` 文件；状态碎片分布在 AGENTS、统一流程、结果.md、路线 JSON、workbuddy memory、对话全文。
16. `[F]` 正式 V005 服务器证据（20 文件）在 `临时/服务器证据/V005/`，**未跟踪且未 ignore**，存在丢失风险；与 AGENTS 要求的 `实验/V00N/` 目录不一致（该目录不存在）。
17. `[F]` 新 CANNJudge 提交工具迁移未完成入库：`脚本/cannjudge-submit.mjs`、`package.json` 均 untracked；旧 browser-submit 已删未提交；`管理/README.md` 仍断链指向 `../scripts/README.md`。
18. `[F]` 敏感面集中在 gitignored 的 `管理/提交队列/runtime/cannjudge-browser-profile/`（约 360M，含 Cookies/Login Data/Token 库）；项目内容区未发现 `.env` 或明文密钥。
19. `[F]` 全仓 **0 个 ZIP**；无本地 `源码/` build 产物；`提交/单方案/R001–R026` 为空目录，R027–R029 仅 README。
20. `[I]` 本地最可信 canonical source of truth = 工作区 `调研/归档/外部对话-ChatGPT编译并修复/原始记录/对话全文.md`（67,375 行）+ SHA 绑定层 `提交/外部轨道-ChatGPT编译并修复/MANIFEST.md`（旧 20 版）；二者互补，均无法覆盖上游 09-18 的 R014/R016/R013。

---

# 2. Directory Overview

总占用约 **575M**（含依赖与浏览器 profile）。

## 2.1 关键目录树

```text
cann/                                    575M
├── AGENTS.md / README.md / package.json / package-lock.json
├── node_modules/                        18M   playwright（gitignored）
├── .agents/ .cannbot/ .codex/           ~184M 工具/供应商依赖（gitignored）
├── .workbuddy/memory/                   24K   MEMORY + 2026-09-17.md（跟踪，有未提交修改）
├── 临时/                                620K  27 文件
│   ├── R001-V003-* / R001-V005-*          草稿（已跟踪）
│   └── 服务器证据/V005/                   20 文件重要证据（**未跟踪**）
├── 提交/                                1.2M  77 文件
│   ├── README / 版本实验记录
│   ├── 混合方案/H001/{V001,V002,V003,V005}/
│   ├── 混合方案/H002/{MIX-…-V001, MIX-…-Parent}/
│   ├── 单方案/R001…R029/                 26 空 + 3 仅 README
│   └── 外部轨道-ChatGPT编译并修复/
│       ├── 源码/  (20 版本 kernel+结果)
│       └── 重建脚本/ (6 py)
├── 文档/                                184K  10 文件（题面/规则/流程/提交核对）
├── 源码/                                216K  工程骨架 + 参考 kernel
├── 管理/                                360M  几乎全是 cannjudge-browser-profile
│   ├── config.json / templates/
│   ├── 路线状态/R001.json … R029.json
│   └── 提交队列/{queued,running,completed}/ + runtime/（空队列）
├── 缓存/                                60K   README + 无关 PDF
├── 脚本/                                24K   cannjudge-submit.mjs（**未跟踪**）
└── 调研/                                4.6M  130 文件
    ├── 总结 / 结果 / 路线最高分 / 工具
    └── 归档/调研1–6、汇总1–3、外部对话-…/原始记录/对话全文.md (1.7M, 67375 行)
```

## 2.2 期望目录：存在 vs 缺失

| 名称 | 状态 |
| --- | --- |
| `文档/` `源码/` `调研/` `提交/` `管理/` `临时/` `缓存/` `脚本/` | **PRESENT** |
| `实验/` | **ABSENT**（V005 证据落在 `临时/服务器证据/`） |
| `results/` `docs/` `code/` `kernels/` `archive/` `scripts/`（小写英文） | **ABSENT** |

## 2.3 有效 / 归档 / 临时 / 重复职责

| 分类 | 目录 | 说明 |
| --- | --- | --- |
| **当前有效工作区** | `提交/混合方案/H001…V005`、`提交/外部轨道…`、`脚本/`、`临时/服务器证据/V005/`、`文档/`、`源码/` | 主线候选 V005；外部轨道 20 版；新提交工具 |
| **历史归档** | `调研/归档/调研1–6`、`汇总1–3`、`外部对话-…`（对话原文）、`提交/单方案/R001–R029` 空壳 | 路线研究材料与占位 |
| **临时实验** | `临时/R001-*` 草稿；`临时/服务器证据/`（名不副实，实为正式证据） | 清理前需先迁移服务器证据 |
| **重复职责** | `提交/**` vs `源码/**`（双处 kernel）；`脚本/` vs `调研/工具/`（已声明分工）；`提交/外部轨道` vs `调研/归档/外部对话`（源码结果 vs 对话原文）；`管理/README` 断链 `scripts/` | 中等风险 |
| **体积/凭据集中** | `管理/提交队列/runtime/cannjudge-browser-profile/` 360M | gitignored；Cookies/Login Data |

---

# 3. Git State

## 3.1 分支 / HEAD / 跟踪

| 项目 | 值 |
| --- | --- |
| 当前分支 | `main` |
| HEAD | `dfa0af05ac63c0c1dc7d9c50cf16a58ab7aa5dd6` |
| HEAD 主题 | Validate V005 on server 3 against online-scored external versions |
| HEAD 日期 | 2026-09-17T17:20:05+08:00 |
| 上游 | `origin/main` → `https://github.com/sxyq/CANN.git` |
| ahead/behind | **ahead 2，behind 0**（origin 仍停在 `0b59c26`） |
| 本地领先未推送 | `bd0b062` Sync external track…；`dfa0af0` Validate V005… |
| stash | 无 |
| worktree | 仅 `/Users/sunyiyang/Desktop/Project/cann [main]` |
| 总提交数 | 15 |
| 其他分支 | 无 |

## 3.2 git status 摘要

- Staged：空
- Modified（6）：`.gitignore`（+node_modules/）、`.workbuddy/memory/2026-09-17.md`、`提交/混合方案/H001-正确性优先/V003/kernel.txt`（kTileElems 7680→8192）、`调研/工具/README.md`、`调研/工具/cannjudge.py`（poll 15s→2s）、`调研/归档/外部对话-ChatGPT编译并修复/原始记录/对话全文.md`（**+17586 行**）
- Deleted 但仍跟踪（2）：`scripts/README.md`、`调研/工具/cannjudge-browser-submit.js`（历史可 `git show` 恢复）
- Untracked（约 24）：`package.json`、`package-lock.json`、`脚本/`（README + cannjudge-submit.mjs）、`临时/服务器证据/`（20 文件）
- Ignored（约 17 条顶层）：`node_modules/`、`.agents/`、`.cannbot/`、`.codex/`、`管理/提交队列/runtime/`、`.DS_Store` 等

## 3.3 近期关键 commits（全库 15）

| Hash | 日期 | 主题 |
| --- | --- | --- |
| `dfa0af0` | 2026-09-17 | Validate V005 on server 3 against online-scored external versions（正文含 R029 28.68/29.01、msprof） |
| `bd0b062` | 2026-09-17 | Sync external track, audit report and server snapshot（删旧 submit 脚本） |
| `0b59c26` | 2026-09-16 | Sync V005 kernel state（= origin/main） |
| `ec52268` | 2026-09-16 | Update H001 V005 validation records |
| `ba6df60` | 2026-09-15 | Add H001 candidates and validation evidence |
| `cf2446d` | 2026-09-15 | Archive route research and score index |
| `6ce67fd` | 2026-09-11 | Adopt V001 submission version layout |
| `ff7ba79`…`18a427c` | 2026-09-11 | 初始化与首个候选 |

**提交主题中无任何 FULL / B0 / PURE / R031 字样**；这些编号只出现在树内路径与未提交内容。

## 3.4 未提交重要对象

| 对象 | 重要性 |
| --- | --- |
| `对话全文.md` 工作副本 +17586 | **最高**：B0→R031→FULL 时代唯一全文 |
| `.workbuddy/memory/2026-09-17.md` +25 | 高：R031=38.36、FULL-R006 1/15、PURE 分数摘要 |
| `临时/服务器证据/V005/**` 20 文件 | 高：V005 编译/精度/性能唯一 NPU 证据链 |
| `V003/kernel.txt` 单行 tile 修改 | 中：身份已混乱的 kernel 再被改 |
| `脚本/` + `package.json` + `.gitignore` + 工具 README/py | 中：提交工具迁移未入库 |
| 计划中的 `submit_enqueue.py` 等 | 文档提及但 **无实现文件** |

## 3.5 跟踪文件盘点

共 **273** 个跟踪文件：调研 125、提交 76、管理 39、源码 9、文档 9、临时 7、缓存 2、workbuddy 2、scripts 1、根 3。  
模式：`*FULL*`=0、`*B0*`=0、`*PURE*`=0（文件名层面）；`kernel*`≈34。

---

# 4. Kernel / Code Inventory

## 4.1 优先路线文件在场性（核心结论）

| 路线 | 独立磁盘 kernel？ | 记录位置 | 已知结果 | 角色 |
| --- | --- | --- | --- | --- |
| B0-PURE-V001 | **NO** | `对话全文.md` L58652–60360 | 15/15 **17.52** @09-17 19:52:39 | pure（仅归档） |
| PURE-R012-V001-32B-ROWGROUP | **NO** | 同上 L59834–60680 | 15/15 **21.37** @20:39:00 | pure（仅归档） |
| PURE-R009-V001-ALIGNED-DATACOPY | **NO** | 同上 L60449–61185 | 15/15 **17.64** @20:47:53 | pure（仅归档） |
| PURE-R010-V001-MANUAL-TAIL | **NO** | 同上 L61725 | 15/15 **17.37** @21:33:52 | pure（仅归档） |
| R031-V001-FULL-MULTIMODE-REWRITE | **NO** | 同上 L54810–56826；SHA `e22188ef…`（2959 行，文档记录） | 15/15 **38.36** @17:26:33 | integrated（冠军参考） |
| FULL-R006-V001-REDUCTION-ARCH | **NO**（生成脚本嵌在对话） | 同上 L62654–64815；memory L125 | **1/15** C2–15 WA，冻结 | failed/frozen |
| FULL-R014-V001-PARAMETER-RESIDENCY | **NO 文件**；**全文嵌在对话** L66212–67349 | 状态“待提交” | 本地无结果（上游 20.09） | pending（本地） |
| FULL-R016-V001 | **NO** | 仅计划队列名 L63659 等 | 本地无（上游 CE→17.14） | pending/缺失 |
| FULL-R013-DOUBLE-BUFFER-PIPELINE | **NO** | 仅计划队列名 L63660 等 | 本地无（上游候选在） | pending/缺失 |
| 其余 FULL-R002/005/015/019/029/030… | **NO** | 仅计划列表 | 无 | planned |
| 提交/源码/临时 中 FULL/PURE/B0/ROWGROUP 关键词 | **0 命中**（代码文件） | — | — | Git 事实 |

⚠ memory L127：R031 时代源码经沙箱工具粘贴，本地 Downloads 已清理 → **无法从本仓库恢复为独立文件**（除非从对话全文手工抽取）。

## 4.2 磁盘主要 kernel 表（含 SHA256 / 行数 / Git）

### H001 混合方案

| 路径 | SHA256 | 行数 | Git | 已知结果 | 角色 |
| --- | --- | --- | --- | --- | --- |
| `提交/混合方案/H001-正确性优先/V001/kernel.asc` | `23b2f3d3d5cc50df828564229b6db80275a0a5df4d3b131429024b52b25291fa` | 412 | ba6df60…6309117 | 15/15 CE，sub `6aa37d94…` | failed/legacy |
| `…/V002/kernel.asc` | `2aa9f65483f7ca8c772aaf252c1131bea77f25c487f84de2a9b721e5517a970b` | 2955 | ba6df60 | 历史 CE（载荷截断） | pending |
| `…/V003/kernel.txt` | WT `6c54b4e70d8397fdd3d26acf6cd8e6a57c78861b628cc9ea6a2b2e3d0afa247f` / HEAD `fb9f8c83bc23ffe3e99af56dfbab6547ca9c38f49675a5c1572de4b958c52e8a` | 1053 | ec52268, ba6df60；**未提交 M** | 结果.md 描述 R001，与文件不符 | **unknown/混乱** |
| `…/V004/` | — | — | 无目录 | 仅诊断记录 | failed（无源） |
| `…/V005/kernel.txt` | `afd5b8eef071e2e849064b1913290b485e0689cf9da40987fb1ffc67dc36bd9c` | 394 | bd0b062, ec52268, ba6df60 | 服务器编译/40-case NPU/msprof 10.580µs；**无官方分** | **integrated 主线** |

Git 历史 kernel（已删路径）：
- `提交/首版/kernel.asc` ≡ 当前 V001（`23b2f3d3…`）
- 原始 `V003/kernel.asc`（`92d10c0d8f05ada8febed5b653b5c9bc9b2104464ed8fc7971d933d2f58cab51`，393 行）— 仅 `git show` 可得
- `V005/kernel.asc.pre-repair` ≡ 当前 V005 `.txt`（`afd5b8ee…`）

### H002 性能组合

| 路径 | SHA256 | 行数 | 角色 |
| --- | --- | --- | --- |
| `…/MIX-R014-R002-R019-V001/kernel.asc` | `5d0fe4c1aa2f0c7d62d114eb653db7ec2c6cb0e785a4968a44091bb2e83b9634` | 459 | integrated（服务器 A/B） |
| `…/MIX-R014-R002-V001-Parent/kernel.asc` | `00df77a28ec051b073be09d6c91a1d7e288b8f1e940ffbe1b6a7f3589fd0a7b6` | 456 | baseline 对照（重构，无结果文件） |

### 外部轨道 20 版（均 git 跟踪，`提交/外部轨道-ChatGPT编译并修复/源码/`）

| 目录 | SHA256 | 行数 | 已知结果 | 角色 |
| --- | --- | --- | --- | --- |
| `000-首版2048tile` | `74e58a557a1e745cf91b8018b22c14c66748cf902a310a5a8aad9f1ec37388d8` | 406 | 未提交（诊断） | baseline |
| `001-首版4096tile` | `fe037f3a948614e16f6f6bce089c72b5cc4f2b081e6a5fb9c675991770fc4e47` | 1058 | 15/15 Pass | baseline |
| `002-对照A-7680` | `55fd90d7c0c16580302a96eb0ea3dfcc011ead3a6e3d5aa5f35a2c1f71752df9` | 1053 | 15/15 Pass | baseline（≈V003 HEAD 内容） |
| `003-对照C-8184` | `57a71128b89a2986cc206aad1cfa38bad3ea33672a0d9762e9a6372a14ff5eeb` | 945 | 15/15 Pass | baseline |
| `004-D-baseline` | `d42787431adad3e0590c8ef60099935881ae81725725a1d918012764057aabc8` | 1002 | 15/15；1.000× 参考 | baseline 冻结 |
| `R015-V001` | `46d2d0a2296fd6d49ace39a28cb808fe2662b37ddd2774c89164880fe6ea98f1` | 1525 | Runtime Error | failed |
| `R015-V002` | `b308f8c6a8af61fbb386972a4828459543073c6c47c66a1c2485adee6eba1bec` | 1473 | 15/15 ≈1.018× | pure 弱 |
| `R028-V001` | `cdff66787f3e40cb24bc9ac2e5dfa78b84d594ea1478d3a5c69c6c59db7ed3da` | 1370 | 15/15 ≈1.002× Reject | pure |
| `R002-V001` | `5f7452c32e6aada17ccde884124d6f4f7bf02059e3d7b28e5c52a9461649f69b` | 1244 | 15/15 ≈1.030× | pure→MIX |
| `R014-V001` | `1c23df73b47c0b3ad9170e17e2e4842945b336185d196f53c12c1553a98aff08` | 1380 | 15/15 | pure |
| `R014-V002` | `e6192b3a8a22de609f84ce15aed99d836346db4dc1e91d6335c4f215bb6e761b` | 1404 | 15/15 | pure→MIX |
| `R019-V001` | `a99f68d0b3e14577934332b011757423b38cbcc4f55747c8411532b2c65a99c1` | 1042 | 15/15 弱正 | pure→MIX |
| `R013-V001` | `ccce97ae33e3b1d234d086e103ef1cc2afd994601a39f345569eecfeeec58cb0` | 1996 | 15/15 无增益 Reject | pure（**≠ FULL-R013**） |
| `MIX-R014-R002-V001` | `da6208a117fbaed886480c07be0e2b30587a46c79492fd4b6662650093bf7d62` | 1449 | 15/15 | integrated |
| `MIX-R014-R002-R019-V001` | `1cfa97a2f1d260640cab0d94038ac610bcaad78c394aa72ca96fce5d27494eb7` | 1414 | 15/15 | integrated 父 |
| `R029-V003…_沙箱下载件` | `aebe961da39feaae650f1d2806b96f98ea30b1659788b8a7e4e4bc63a54885b1` | 837 | 15/15 **28.68** | full（载荷身份未裁决） |
| `R029-V003…_对话全文` | `b1470ef5736743f0028e5c6ee0d684a18b112b9dd0b169692b9eb2cb5f982246` | 2034 | 同声称 28.68 | full（重建，≠ 沙箱） |
| `R029-V004-WIDE-LOWP-CACHED` | `02bd5dc8fb727d4e3fe5a2e969df3bcab218232b1cbdba10dd49b819a18030b8` | 2108 | 15/15 **29.01** | full（R031 前最佳） |
| `候选-R006-V001-PARTIALS` | `28c282daeab3778b87fb8dbe37496e082b553d1f5c2e15e8d0a63b63f6423602` | 1403 | memory: **28.80**；目录结果.md 仍写“未提交” | pending/full（文档过期） |
| `候选-R020-V001-RSQRT` | `070524319cd8cc39a8e9219a922d36311e191bdd04c5f16f4445b0161d4c521e` | 1304 | WA Case2–15 Reject | failed |

### 源码/ 工程

| 路径 | SHA256 | 行数 | 角色 |
| --- | --- | --- | --- |
| `源码/op_kernel/add_rms_norm_bias.cpp` | `4a76b368abb1777a256d3d4f528eb37d4f3bc83ff1e3c54e77ffae8764136552` | 301 | baseline 骨架（**不匹配任何 V00N SHA**） |
| `源码/op_host/add_rms_norm_bias.cpp` | `93c36231c8ea0e9b39cb467222d63c5ff687b302ea277a5846dcfc7f911b09f6` | 131 | baseline |
| `源码/op_host/add_rms_norm_bias_tiling.h` | `5993602c30dccca616c466a11619568d4947d922ad5a273d18611c7958174d3d` | 24 | baseline |
| `源码/参考/可运行候选/kernel.asc` | `35525ce93b068ef928d1e9a60e4f2c6fdec45fbc7c5511b73b509186dfd0635e` | 2947 | baseline 参考（Ref） |

### 临时/

| 路径 | SHA256 | 关系 |
| --- | --- | --- |
| `临时/服务器证据/V005/server3-kernel-20260917.txt` | `afd5b8ee…bd9c` | **≡ H001/V005/kernel.txt**（唯一重复组） |

### 重复 SHA 组

| SHA256 | 路径 |
| --- | --- |
| `afd5b8eef071e2e849064b1913290b485e0689cf9da40987fb1ffc67dc36bd9c` | H001/V005/kernel.txt · 临时/服务器证据/V005/server3-kernel-20260917.txt |

其余约 26 个 kernel 文件 SHA 全部唯一。

### 同编号混淆风险（摘要）

1. 外部 `R013-V001` ≠ `FULL-R013`；外部 `R014-V001/V002` ≠ `FULL-R014`；外部 `R016-V001`(28.72) ≠ `FULL-R016`(17.14)；`候选-R006-PARTIALS`(28.80) ≠ `FULL-R006`(1/15)。
2. 仓库 `R029-官方Tiling五模式` ≠ 外部 `R029 wide cached-y`。
3. H002 与外部同名 MIX 目录 kernel/结果双不同。
4. 扩展名 `.asc`/`.txt` 不可作版本信号。

---

# 5. Online Result Inventory

## 5.1 与上游 9 条对照

| # | 版本 | 本地正确性 | 本地官方分 | 本地提交时间 | 15 点明细 | SHA 绑定 | git 绑定 | 结果路径 | 对照 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | B0-PURE-V001 | 15/15 | 17.52 | 2026/09/17 19:52:39 | 有 | 生成期 STATIC 有；RESULT 无 source_sha256 | 否 | `对话全文.md` L59812/L60094–60105 | **完全一致** |
| 2 | PURE-R012-V001-32B-ROWGROUP | 15/15 | 21.37 | 20:39:00 | 有 | 同上 | 否 | 同上 L60383/L60653–60668 | **完全一致** |
| 3 | PURE-R009-V001-ALIGNED-DATACOPY | 15/15 | 17.64 | 20:47:53 | 有 | 同上 | 否 | 同上 L60979/L61185–61200 | **完全一致** |
| 4 | PURE-R010-V001-MANUAL-TAIL | 15/15 | 17.37 | 21:33:52 | 有 | 同上 | 否 | 同上 L61678/L61725–61739 | **完全一致** |
| 5 | R031-V001-FULL-MULTIMODE-REWRITE | 15/15 | 38.36 | 17:26:33 | 有 | **源码全文 SHA `e22188ef…`**（文档记录） | 否 | 同上 L56336/L56681–56697 | **完全一致** |
| 6 | FULL-R006-V001-REDUCTION-ARCH | **1/15** | 本地 `not provided` | 无 | 有 15 点表 | 生成期 STATIC | 否 | 同上 L64781–64815；memory L125 | **完全一致**（正确性/冻结） |
| 7 | FULL-R014-V001 | 本地无 | 本地无（上游 20.09） | 本地无 | 无 | 仅有待提交源码 | 否 | 仅源码 L66212–67349；文末待提交 | **本地缺失** |
| 8 | FULL-R016 / COMPILEFIX-A | 本地无 | 本地无（上游 17.14） | 本地无 | 无 | 无 | 否 | 仅计划名 | **本地缺失** |
| 9 | FULL-R013-V001 | 本地无 | 无 | 无 | 无 | 无 | 否 | 仅计划名 | **本地缺失** |

全库确认：`20.09` / `17.14` / `COMPILEFIX` / `00:24:36` / `02:20:54` **零命中**。  
（`临时/…csv` 中 `38.36` 为 `2655738.368` 子串误报。）

## 5.2 本地额外官方分（不在上游 9 条）

| 版本 | 正确性 | 官方分 | 时间 | SHA 绑定 | 位置 |
| --- | --- | --- | --- | --- | --- |
| R029-V003-WIDE-BF16-CACHED | 15/15 | 28.68 | 09-17 15:13:49 | 双 SHA 裁决未定 | `提交/外部轨道…/R029-V003-*/结果.md`；对话 |
| R029-V004-WIDE-LOWP-CACHED | 15/15 | 29.01 | 15:27:42 | `02bd5dc8…` | `…/R029-V004…/结果.md` |
| R006-V001-PARTIALS | 15/15 | 28.80 | 15:56:34 | 文件 SHA 未闭环 | 对话 L51147 |
| R016-V001（旧广度） | 15/15 | 28.72 | 16:07:28 | 无 | 对话 L54689 |
| R030-V001 | 15/15 | 28.62 | 16:12:45 | 无 | 对话 L54720 |
| R012-on-R031 | 15/15 | 36.69 | 17:50:29 | 无 | 对话 L57164 |
| R009-FASTPATH@R031 | 15/15 | 38.36 | 18:08:25 | 无 | 对话 L57811 |
| R010-SPLIT@R031 | 15/15 | 38.15 | 19:40:18 | 无 | 对话 L58556 |

另有大量 15/15 **无官方分** 记录（外部早期 001–004、R015-V002、R002、R028、R014、R013-V001、MIX 系…），以及 H001 V001/V002 CE、旁路 311559/311894/316974、R015-V001 RE、R020 WA。V005 无官方分（仅 Hypothesis ≈27.6–29.0）。

## 5.3 绑定总评

| 维度 | 状态 |
| --- | --- |
| 15 case 明细 | 有官方分的记录 **均有** |
| source_sha256 ↔ 分数 | RESULT 层普遍缺失；R031/R029 有可追溯 SHA |
| git_commit ↔ 分数 | **全部未绑定** |
| submission_id | 仅 CE/旁路有；**所有 Pass+官方分无 id** |

## 5.4 时间线

```text
本地对话归档结束: 2026-09-17 22:25  → FULL-R014 源码就绪、待提交
上游 FULL-R014:    2026-09-18 00:24:36 / 15/15 / 20.09  冻结
上游 FULL-R016:    2026-09-18 02:20:54 / COMPILEFIX-A 15/15 / 17.14  冻结
上游 FULL-R013:    候选在、待首次有效提交
⇒ 本地 #7–#9 = 缺失/更旧快照，不是冲突
```

---

# 6. Documentation Inventory

## 6.1 缺失的交接类文件

| 模式 | 结果 |
| --- | --- |
| `CURRENT_STATE*` / `AGENT_HANDOFF*` / `ROUTE_TREE*` / 顶层 `RESULTS*` | **仓库内不存在**；仅出现在 `对话全文.md` 内的沙箱路径文本 |

功能等价物：`调研/结果.md`（路线树）、`提交/版本实验记录.md`、`提交/外部轨道…/结果汇总.md`、`管理/路线状态/*.json`。

## 6.2 当前仍有效（但范围有限）

| 路径 | 角色 | 关键主张 |
| --- | --- | --- |
| `AGENTS.md` | 工作约定 | 主线 `H001/V005`；单路线深度优先；不知 38.36 |
| `文档/统一探索收敛与线上提交流程.md` | 自称唯一流程事实源 | 深度优先 R001–R029；§11 仓库无有效分 |
| `调研/结果.md` | 本地路线树 | R001–R029 + H001/H002；无官方分 |
| `提交/版本实验记录.md` | 版本索引 | V001/V002 CE；V005 服务器已验、平台未提交 |
| `提交/README.md` | 提交区入口 | 候选 V005；外部最佳写 28.68/29.01（**过期**） |
| `提交/外部轨道…/{README,MANIFEST,结果汇总}.md` | 旧 20 版同步 | 止于 29.01；无 R030/R031/FULL |
| `调研/归档/…/审查报告.md` | 旧审计 | 29.01 无 submission_id → 不能写 Champion |
| `.workbuddy/memory/{MEMORY.md,2026-09-17.md}` | Agent 记忆 | 同一文件内前后 NEXT 自相矛盾；晚段含 38.36/FULL |
| `调研/归档/…/原始记录/对话全文.md` | **最深最新外部证据** | 67,375 行；R031/B0/PURE/FULL 与全部榜单表 |
| `脚本/README.md`、`调研/工具/README.md` | 工具入口说明 | 分工已声明 |

## 6.3 历史 / 已被推翻

| 路径 | 原因 |
| --- | --- |
| `调研/路线最高分.md`（09-15） | 仍写“无有效平台分”——被 28.68+ 与 38.36 推翻 |
| `文档/README.md`（状态 09-11） | 旧目录树、旧环境描述 |
| `文档/competition-rules.md` 等 | 事实类仍可用，不承载 NEXT |
| `调研/归档/汇总1–3` | R 编号跨轮不稳定（总结.md L14） |
| `管理/工作目录盘点.md` | 服务器快照 |
| `scripts/`（英文）契约 | 已删，被中文 `脚本/` 取代 |

## 6.4 冲突（不可当单一真相）

| 主张 A | 主张 B |
| --- | --- |
| 仓库：**无有效分 / V005 无分** | 对话：外部 28.68–38.36；上游另有 20.09/17.14 |
| 仓库候选 **H001/V005** | 外部包 **R029-V004 → R031**（未进 `提交/` 包） |
| NEXT=总结 §16.1 `R001→R017→…` | workbuddy 中段 `R020+R006→R030` | 晚段 **FULL-\* 广度 + B0-PURE** |
| 仓库 R029=Tiling 五模式 | 外部 R029=wide cached-y 计分线 |
| AGENTS：一次一条路线 | 外部：广度每支线先 V001 |
| config 磁盘：98% 使用率 | 结果.md 用户裁定：仅可用 <20G |

## 6.5 会误导新 Codex 的文档

1. `调研/路线最高分.md` — 声称无分  
2. 根 `README.md` — 链到上述过期分数文件  
3. `管理/路线状态/R001–R029.json` — 全 null，`next_action` 假设尚无分  
4. `调研/总结.md` §16.1 — 深度优先 NEXT，与 FULL 广度冲突  
5. `AGENTS.md` — 单路线 + V005 主线 + 无冠军意识  
6. `提交/README` / `结果汇总` — 止于 29.01  
7. `文档/统一探索…` §11 — “无可确认 15/15 官方有效结果”  
8. 任何寻找 `CURRENT_STATE`/`AGENT_HANDOFF` 的 Agent — 找不到  
9. 断链：`管理/README.md:3` → `../scripts/README.md`  
10. `候选-R006/R020/结果.md` 仍写“未提交”，与 memory 28.80/Reject 冲突  

---

# 7. CANNJudge Tooling State

静态只读审计；**本轮未执行 login/submit**。

## 7.1–7.8 `脚本/cannjudge-submit.mjs` 能力矩阵

| # | 问题 | 答案 | 证据 |
| --- | --- | --- | --- |
| 1 | 真实支持哪些功能 | Playwright CLI：`login` + `submit`；源身份 SHA/行数、`--dry-run`、`--yes` 确认、解析 problemId、单次 POST 提交、轮询评测、Pass 后查 ranking 官方分、本地公式分、人类表格 + `--json`/`--out` | `:445-457` 等；`node --check` 通过 |
| 2 | login 是否可用 | **交互式手动登录可用（代码层）**；非自动凭据登录。开有头浏览器→等 Enter→校验 `localStorage cannjudge_user`。**本次未实测平台** | `:341-354` |
| 3 | submit 是否可用 | **代码路径完整**；需 session + `--yes`；单 POST 无重试。HTTP/限流/载荷 **未实测** | `:409-431` |
| 4 | 能否等待评测完成 | **能**；默认 interval 2s、max 30min，轮询至终态 | `:19-20`, `:374-397` |
| 5 | 能否返回 submission id | **能**；读 `submissionId` 并打印/写入 JSON | `:430-432`, `:210` |
| 6 | 能否返回 Official Score | **有条件能**；Pass 后翻 ranking 最多 10 页×100，匹配 id，重试≤5；非 Pass/未上榜 → `null` | `:356-372`, `:386-391` |
| 7 | 能否返回完整 15 Case | **返回 `submission.result` 全部行**；不硬编码 15；README 称 15 点，脚本不断言 `length===15`；平台是否总回 15 行 **未实测** | `:203-220`, `:237-249` |
| 8 | 能否输出结构化 JSON | **能**：`--json` / `--out`；schema 含 submissionId、status、passCount、testcaseCount、officialScore、calculatedScore、source、testcases[]、result | `:209-222`, `:434-438` |

## 7.9 npm scripts 指向

```json
"cannjudge:login": "node 脚本/cannjudge-submit.mjs login",
"cannjudge:submit": "node 脚本/cannjudge-submit.mjs submit"
```

（`package.json:8-9`）↔ 文件存在、README 示例一致。**`package.json`/`package-lock.json`/`脚本/` 均 untracked。** lock 仅 pin `@playwright/cli@0.1.20` + `playwright@1.64.0-alpha-2026-09-14`；`node_modules/` 已存在且 gitignored。

## 7.10 旧 browser/bookmarklet/scripts 残留

| 残留 | 状态 |
| --- | --- |
| `调研/工具/cannjudge-browser-submit.js` | 工作区 **已删**（`D`）；HEAD 仍有 |
| `scripts/README.md` | 工作区 **已删**；HEAD 仍有（从未实现的队列契约） |
| `管理/README.md:3` → `../scripts/README.md` | **断链仍在** |
| `文档/统一探索…` 中 `submit_enqueue/worker/score_record` | 文档规划，**无实现文件** |
| 可执行 bookmarklet | **未找到** |
| Chromium profile 内 “Bookmarks” | profile 内部文件，非项目脚本 |

## 7.11 `cannjudge.py` 当前职责

只读 GET CLI；docstring 禁止 POST/凭据；刻意不设 Cookie/Authorization：

| 子命令 | 行为 |
| --- | --- |
| `preflight <path>` | 本地：行数/字节/SHA-256 + 模板检查（run_kernel 等） |
| `poll <id>` | GET `/api/submissions/{id}` 至终态；可选 ranking 官方分；`--json/--out/--once` |
| `problem` | GET problem + 可选 ranking |

相对 HEAD 修改：默认 poll 间隔 **15s → 2s**。`py_compile` OK。**无 submit 路径。**

分工：`cannjudge.py` = 匿名 preflight/poll；`cannjudge-submit.mjs` = 鉴权一键提交。

## 7.12 是否存在重复提交脚本

| 路径 | 角色 |
| --- | --- |
| `脚本/cannjudge-submit.mjs` | **唯一** login+submit 入口（未跟踪） |
| `调研/工具/cannjudge.py` | GET-only，非 submit |
| `调研/工具/cannjudge-browser-submit.js` | 已删（HEAD 可恢复） |
| `scripts/README.md` | 已删；非可执行 |
| `.agents/…/workflow.submit_server.py` | 无关（华为 YAML 收集） |

**结论：工作区无第二条可执行提交路径。**

---

# 8. Duplicates / Conflicts

## 8.1 同 SHA 多文件

仅 1 组：`afd5b8ee…` ≡ H001/V005/kernel.txt · 临时/服务器证据/V005/server3-kernel-20260917.txt（有意镜像）。

## 8.2 同名不同 SHA

| 逻辑名 | A | B |
| --- | --- | --- |
| `MIX-R014-R002-V001` | H002 Parent `00df77a2…` asc | 外部 `da6208a1…` txt |
| `MIX-R014-R002-R019-V001` | H002 `5d0fe4c1…` asc + 结果 `69981a6e…` | 外部 `1cfa97a2…` txt + 结果 `1d0ed7c6…` |
| `R029-V003-WIDE-BF16-CACHED` | 沙箱 `aebe961d…` 837L | 对话 `b1470ef5…` 2034L（均写 28.68） |
| `kernel.txt` ×22 / `kernel.asc` ×5 / `结果.md` ×25 | 必须靠父目录区分 | |

## 8.3 同 route 多套实现

| ID | 单方案壳 | 外部轨道实现 | FULL/PURE 代 | 状态 JSON |
| --- | --- | --- | --- | --- |
| R006 | 空目录 | PARTIALS 28.80 | FULL-R006 1/15 | R006.json null |
| R013 | 空 | R013-V001 Reject | FULL-R013 无本地源 | R013.json null |
| R014 | 空 | V001/V002 + 两套 MIX | FULL-R014 仅对话源 | R014.json null |
| R016 | 空 | 无本地 kernel（对话 28.72） | FULL-R016 无 | R016.json null |
| R029 | README=Tiling | V003/V004 计分 | — | R029.json null |

## 8.4 结果 ↔ 源码绑定

- **可绑定**：H001/V005（全 SHA + 服务器快照）；外部 20 版经 MANIFEST 前缀（无 submission_id）。
- **无法绑定**：所有对话时代分数（17.52…38.36…）；R029-V003 双载荷；H002 Parent 无结果；`源码/op_kernel` SHA 不匹配任何 V00N；V003 结果早于 kernel 修改 2 天。
- **有源无结果**：H002 Parent、源码/参考、FULL-R014 源（本地）、单方案空壳。
- **有结果无独立源**：B0/PURE/R031/FULL-*（仅对话嵌入）。

## 8.5 分数复用

| 分数 | 引用面 |
| --- | --- |
| 28.68 | ≥11 个内容路径；挂 **两个不同 SHA** |
| 29.01 | ≥10 路径；绑 `02bd5dc8…` |
| 17.52/21.37/17.64/17.37/38.36/28.80/28.72 | 实质仅 `对话全文.md`（+ memory 摘要） |
| 20.09 / 17.14 | **本地不存在** |

## 8.6 ZIP

全仓 `*.zip/*.tar*/*.7z` = **0**。对话内沙箱 zip 路径不是磁盘文件。

---

# 9. Sensitive / Generated / Temporary State

## 9.1 凭据风险（仅路径 + 类型，未读内容）

| 路径 | 类型 |
| --- | --- |
| `管理/提交队列/runtime/cannjudge-browser-profile/Default/Cookies`（+journal） | 浏览器 Cookie |
| `…/Default/Login Data`、`Login Data For Account`（+journals） | 保存的登录库 |
| `…/Default/Session Storage/`、`Sessions/`、`EdgeSessions/` | 会话态 |
| `…/Default/Trust Tokens`、`Vpn Tokens`（+journals） | Token 库 |
| `…/Local State` | 可能含加密密钥材料 |
| 整个 `…/cannjudge-browser-profile/` | 约 360–361M，566 文件 |

缓解：`.gitignore` 含 `管理/提交队列/runtime/`；`git ls-files` 对 profile = 0。  
项目内容区：**无** `.env*`；源码脚本无硬编码密钥；`token-config.md` 为 skill 文档。

## 9.2 生成/依赖/噪声

| 路径 | 类型 | 备注 |
| --- | --- | --- |
| `node_modules/` | 18M | gitignored |
| `.cannbot/` `.agents/` `.codex/` | ~184M | gitignored 依赖/skill |
| `.DS_Store` ×10 | 噪声 | 分布各层 |
| `缓存/buckmaster_statement.pdf` | 无关缓存 | 56K |
| `源码/` build 产物 | **无** | 干净 |
| `__pycache__` / `PROF_*` / `input|output` | **未发现** | |

## 9.3 `临时/` 分类

```text
临时/
├── 服务器证据/V005/     ← 重要（20 文件：kernel 快照 + 5 脚本 + 14 日志）
│                           V005 编译/精度/BF16/D边界/性能校准 vs 317643
└── R001-V003-* / R001-V005-*  ← 09-15 任务草稿（7 已跟踪文件）
                                  其中 CPU 矩阵被 路线最高分.md 引用
```

- **重要服务器证据**：`临时/服务器证据/V005/**`（**当前未跟踪**）  
- **草稿但被文档引用**：`临时/R001-V003-Agent02-CPU矩阵.md` 等  
- **可清理候选（本轮未清理）**：仅在确认无引用后的纯草稿  
- 不存在 `脚本/服务器证据/`

## 9.4 自动生成

`gen_data.py`/`gen_probe.py`/`重建脚本/*.py` 为证据工具链；未发现 CANNBot `operators/**/state.json` 等运行态文件。

---

# 10. Current Local vs Online Gap

## A–E. 四条 FULL 路线

| 路线 | 本地有源？ | 本地有结果？ | 位置 | git commit？ | 线上结果本地？ | 可唯一绑定 SHA？ |
| --- | --- | --- | --- | --- | --- | --- |
| **FULL-R006** | 生成脚本/全文嵌在对话 | **有：1/15 冻结** | `对话全文.md` L62717–64815；memory L125 | **否**（对话 +25 行未提交） | 有（1/15，无官方分） | 仅对话考古级；无 platform id |
| **FULL-R014** | **有完整 kernel.txt 粘贴**（L66212–67349） | **无**（停在“待提交”） | 同上 L64818–67375 | **否** | **无**（上游 20.09 缺失） | 源可从对话重哈希；分数本地不存在 |
| **FULL-R016** | **无** | **无** | 仅计划名 | **否** | **无**（17.14 缺失） | 无法绑定 |
| **FULL-R013** | **无**（无候选文件） | **无** | 仅计划名 | **否** | **无** | 无法绑定 |

注意：`提交/外部轨道/…/R013-V001`、`R014-V001/V002` 是 **旧一代**，不是 FULL 代。

## F. 本地是否比上游走得更远？

- **对上游在线 FULL 轨道：否，本地落后。** 缺 R014 分数、R016 全流程、R013 候选。  
- **对本地 git HEAD：是。** B0→PURE→R031→FULL-R006 结果→FULL-R014 源，全部只在工作区未提交内容里。  
- **上游清单未覆盖的本地增量**：  
  - H001/V005：40-case 三 dtype NPU、16 探针、msprof 10.580µs  
  - `V005-vs-317643` 性能校准 @2026-09-18 01:09  
  - 旧 20 版 SHA 绑定同步 + 审查报告（curation）

## G. 具体多出的实验

相对上游状态列表：V005 服务器证据链、vs-317643 校准、旧 20 版 MANIFEST 审计。  
**没有** 本地独有的 FULL-R014/016/013 新结果。

## H. 真实 NEXT route

按代码+结果（非文档）双轨：

1. **在线 FULL 轨道**：本地卡在“提交 FULL-R014”；上游已完成 R014/R016 → **真实下一步 = 先同步上游 R014(20.09)/R016(17.14)/R013 记录，再按上游顺序做 FULL-R013 首次有效提交**；否则会重复提交已冻结路线。  
2. **仓库 H001 轨道**：唯一 NPU 验证候选 = **H001/V005**；单点方向 = V005 + 宽行 cached-y（memory L108–109）。  
3. 若强制一行：**先 re-sync 上游 → FULL-R013；并行仓库线 = V005 宽行 cached-y**。

## I. 本地文档 NEXT vs 真实代码/结果

| 文档 | 声称 | 现实 |
| --- | --- | --- |
| `提交/README` L38 | 外部最佳 28.68/29.01 | 已有 38.36；上游另有 20.09/17.14 |
| `路线最高分.md` | 无有效分 | 对话有完整榜单表 |
| `结果.md`/`总结`/`版本实验记录` | 仅仓库轨 | 对外部轨沉默 |
| `管理/路线状态/*` | score null | 不知 FULL 时代 |
| workbuddy L101 NEXT | 判据→R020+R006→R030 | 被同文件 L123–125 自我推翻 |
| 对话末行 L67375 | “现在提交 FULL-R014” | 与本地代码一致，**相对上游过期** |

**结论：本地正式文档 NEXT 与真实在线状态不一致；对话末行与本地磁盘一致但相对上游过期。**

## J. 最可信 canonical source of truth

**首选（分数+源考古）**：`调研/归档/外部对话-ChatGPT编译并修复/原始记录/对话全文.md`（工作区 67,375 行）  
- 唯一含 09-17 15:13→22:25 逐条榜单表（15 case 用时、时间戳）+ FULL-R006 失败表 + FULL-R014 全文源  
- **必须标注**：未提交；截止 09-17 ~22:2x；无 submission_id  

**并列（旧 20 版 SHA 绑定）**：`提交/外部轨道-ChatGPT编译并修复/MANIFEST.md`  

**不可单独作为真相**：`路线最高分.md`、`版本实验记录.md`、`结果汇总.md`、`管理/路线状态/*`、workbuddy 摘要（有损且前后矛盾）。

---

# 11. Problems Requiring Cleanup

本轮只报告，不执行整理。

## P0 — 会导致实验结果错误绑定/丢失

1. **未提交的 `对话全文.md` +17586 行与 memory +25 行**：B0→FULL 时代唯一全文；checkout/stash/误还原即丢失。  
2. **`临时/服务器证据/V005/**` 20 文件未跟踪**：V005 唯一 NPU 证据链。  
3. **所有官方分无 git/SHA/submission_id 绑定**；R029-V003 双载荷未裁决 → 28.68 可能绑错字节。  
4. **H001 V003 身份断裂 + 未提交 tile 修改**：结果.md 描述的内核与磁盘文件不一致；原始 R001 V003 仅在 git 历史。  
5. **上游 FULL-R014/016/013 记录本地缺失**：若不先同步，可能把已冻结路线当未做重提。

## P1 — 会误导 Codex 路线判断

1. `调研/路线最高分.md` + 根 README 分数链接过期。  
2. `管理/路线状态/*.json` 全 null、next_action 过时。  
3. `调研/总结.md` §16.1 深度优先 NEXT vs 外部 FULL 广度。  
4. `AGENTS.md` 单路线 + V005 无冠军意识。  
5. 无 `CURRENT_STATE`/`AGENT_HANDOFF` 文件名。  
6. 同号异义：R029、R016(28.72 vs 17.14)、R013/R014/R006 的 OLD vs FULL。  
7. `文档/统一探索…` §11 “无有效 15/15 官方分”。  
8. `候选-R006/R020/结果.md` “未提交” vs 已有 28.80/Reject。  
9. `管理/README` 断链 `scripts/`。  
10. workbuddy 同一文件前后 NEXT 矛盾。

## P2 — 目录/命名/重复

1. `提交/` vs `源码/` 双处 kernel；`源码/op_kernel` 不绑定 V00N。  
2. H002 vs 外部同名 MIX 双实现。  
3. `实验/` 缺失，正式证据在 `临时/`。  
4. `单方案/R001–R026` 空壳占 26 目录。  
5. `脚本/` 与 `调研/工具` 靠 README 分工，易混。  
6. `.asc`/`.txt` 扩展混用。  
7. `缓存/buckmaster_statement.pdf` 疑似无关。  
8. 磁盘暂停规则 98% vs 20G 双标准。

## P3 — 历史整洁

1. 10× `.DS_Store`。  
2. 18M `node_modules` + 178M `.cannbot`（已 ignore）。  
3. 360M 浏览器 profile（已 ignore，勿提交勿外传）。  
4. `临时/R001-*` 草稿（CPU 矩阵仍被引用，清理前核对）。  
5. 删除未暂存的旧 submit 脚本与 `scripts/README`（历史可恢复）。  
6. 无 ZIP 可归并问题。

---

# 12. Raw Facts Appendix

## 12.1 关键 git 输出摘要

```text
branch: main
HEAD: dfa0af05ac63c0c1dc7d9c50cf16a58ab7aa5dd6
  Validate V005 on server 3 against online-scored external versions
  2026-09-17T17:20:05+08:00
upstream: origin/main @ 0b59c26 (https://github.com/sxyq/CANN.git)
ahead 2 / behind 0; stash: none; worktrees: 1; total commits: 15
branches: * main ; remotes/origin/main

status highlights:
  M .gitignore
  M .workbuddy/memory/2026-09-17.md
  M 提交/混合方案/H001-正确性优先/V003/kernel.txt
  M 调研/工具/README.md
  M 调研/工具/cannjudge.py
  M 调研/归档/外部对话-ChatGPT编译并修复/原始记录/对话全文.md   (+17586)
  D scripts/README.md
  D 调研/工具/cannjudge-browser-submit.js
  ?? package.json
  ?? package-lock.json
  ?? 脚本/
  ?? 临时/服务器证据/

ls-files: 273 tracked
  调研125 提交76 管理39 源码9 文档9 临时7 缓存2 workbuddy2 scripts1 根3
```

## 12.2 重要文件 hash 表（核心）

| 文件 | SHA256 |
| --- | --- |
| H001/V001/kernel.asc | `23b2f3d3d5cc50df828564229b6db80275a0a5df4d3b131429024b52b25291fa` |
| H001/V002/kernel.asc | `2aa9f65483f7ca8c772aaf252c1131bea77f25c487f84de2a9b721e5517a970b` |
| H001/V003/kernel.txt (WT) | `6c54b4e70d8397fdd3d26acf6cd8e6a57c78861b628cc9ea6a2b2e3d0afa247f` |
| H001/V003/kernel.txt (HEAD) | `fb9f8c83bc23ffe3e99af56dfbab6547ca9c38f49675a5c1572de4b958c52e8a` |
| H001/V005/kernel.txt | `afd5b8eef071e2e849064b1913290b485e0689cf9da40987fb1ffc67dc36bd9c` |
| 临时/…/server3-kernel-20260917.txt | 同上（≡ V005） |
| H002 MIX-R014-R002-R019-V001/kernel.asc | `5d0fe4c1aa2f0c7d62d114eb653db7ec2c6cb0e785a4968a44091bb2e83b9634` |
| H002 MIX-…-Parent/kernel.asc | `00df77a28ec051b073be09d6c91a1d7e288b8f1e940ffbe1b6a7f3589fd0a7b6` |
| 外部 002-对照A-7680 | `55fd90d7c0c16580302a96eb0ea3dfcc011ead3a6e3d5aa5f35a2c1f71752df9` |
| 外部 004-D-baseline | `d42787431adad3e0590c8ef60099935881ae81725725a1d918012764057aabc8` |
| 外部 R029-V004 | `02bd5dc8fb727d4e3fe5a2e969df3bcab218232b1cbdba10dd49b819a18030b8` |
| 外部 R029-V003 沙箱 | `aebe961da39feaae650f1d2806b96f98ea30b1659788b8a7e4e4bc63a54885b1` |
| 外部 R029-V003 对话 | `b1470ef5736743f0028e5c6ee0d684a18b112b9dd0b169692b9eb2cb5f982246` |
| 外部 R006-PARTIALS | `28c282daeab3778b87fb8dbe37496e082b553d1f5c2e15e8d0a63b63f6423602` |
| 外部 R013-V001 | `ccce97ae33e3b1d234d086e103ef1cc2afd994601a39f345569eecfeeec58cb0` |
| 源码/op_kernel/add_rms_norm_bias.cpp | `4a76b368abb1777a256d3d4f528eb37d4f3bc83ff1e3c54e77ffae8764136552` |
| 源码/参考/可运行候选/kernel.asc | `35525ce93b068ef928d1e9a60e4f2c6fdec45fbc7c5511b73b509186dfd0635e` |
| git 原始 V003/kernel.asc（历史） | `92d10c0d8f05ada8febed5b653b5c9bc9b2104464ed8fc7971d933d2f58cab51` |
| R031（文档记录，磁盘无） | `e22188efbb916803b34a427cb353a29fc83494a7b22dec249981ae1acd0155ba` |

完整 20+ 外部轨道 SHA 见第 4 节表。

## 12.3 关键结果文件路径

```text
调研/归档/外部对话-ChatGPT编译并修复/原始记录/对话全文.md   # 主结果源（未提交）
.workbuddy/memory/2026-09-17.md                            # 摘要（未提交尾段）
提交/外部轨道-ChatGPT编译并修复/结果汇总.md
提交/外部轨道-ChatGPT编译并修复/MANIFEST.md
提交/外部轨道-ChatGPT编译并修复/源码/*/结果.md              # 20 个
提交/混合方案/H001-正确性优先/V001/结果.md
提交/混合方案/H001-正确性优先/V002/结果.md
提交/混合方案/H001-正确性优先/V003/结果.md
提交/混合方案/H001-正确性优先/V005/结果.md                  # 含全 SHA 绑定
提交/版本实验记录.md
调研/路线最高分.md                                         # 过期
调研/结果.md
管理/路线状态/R001.json … R029.json                        # 全 null
临时/服务器证据/V005/日志/*                                 # V005 NPU 证据（未跟踪）
临时/服务器证据/V005/日志/V005-vs-317643-性能校准-20260918.md
```

## 12.4 关键文档路径

```text
AGENTS.md
README.md
文档/统一探索收敛与线上提交流程.md
文档/README.md                                              # 状态停在 09-11
文档/problem-add-rms-norm-bias.md
文档/submission-checklist.md
调研/总结.md                                                # §16.1 NEXT
调研/归档/外部对话-ChatGPT编译并修复/审查报告.md
提交/README.md
提交/混合方案/README.md
提交/单方案/README.md / 路线索引.md
管理/README.md                                              # 断链 scripts/
管理/config.json
管理/工作目录盘点.md
脚本/README.md
调研/工具/README.md
.workbuddy/memory/MEMORY.md
```

## 12.5 ZIP 路径

**无。** 全仓 0 个 zip/tar/7z。

## 12.6 敏感路径（不展开内容）

```text
管理/提交队列/runtime/cannjudge-browser-profile/Default/Cookies
管理/提交队列/runtime/cannjudge-browser-profile/Default/Login Data
管理/提交队列/runtime/cannjudge-browser-profile/Default/Session Storage/
管理/提交队列/runtime/cannjudge-browser-profile/Default/Trust Tokens
管理/提交队列/runtime/cannjudge-browser-profile/Default/Vpn Tokens
管理/提交队列/runtime/cannjudge-browser-profile/Local State
```

---

## 审计元数据

- 子代理：explore-1 Git · explore-2 目录 · explore-3 Kernel · explore-4 线上结果 · explore-5 文档 · explore-6 CANNJudge 工具链 · explore-7/9 重复冲突 · explore-8 本地-上游差距（共 8+1）
- 本轮唯一写入：`临时/LOCAL_FULL_STATE_REPORT.md`（本文件）
- 未修改其它任何文件；未发起提交/推送/在线评测
