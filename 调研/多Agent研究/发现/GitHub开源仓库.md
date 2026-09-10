# F2: GitHub 开源仓库

## 研究范围

本轮读取了与 OpenAI 数学结果、形式化证明、数值复算和研究证据管理有关的公开仓库 README、关键源码或配置。以下 `verified` 只表示已读到对应内容，不表示本机已经运行外部仓库；外部仓库均未在本机执行。

## 仓库核对

| 仓库 | 分支 / commit | 实际读取内容 | Issue / Release | 判断 |
| --- | --- | --- | --- | --- |
| [openai/NavierStokesAndEuler](https://github.com/openai/NavierStokesAndEuler) | `main` / 未确认 | README、`lakefile.toml`、Lean 4 工具链、Navier-Stokes/Euler 源码、Comparator 入口 | 0 / 无 Release | `verified`；OpenAI 形式化主仓库，有 `lake build` 路径，本机未构建 |
| [plby/Erdos90](https://github.com/plby/Erdos90) | `main` / `2062b0e6c9770c81e397bfe41148e90b92ca0567` | README、`Submission.lean`、`Challenge.lean`、`Solution.lean`、`WorkspaceTest.lean` | 0 / 无 Release | `verified`；有 Lake 构建入口；Challenge 中的 `sorry` 是题目占位，不能当成完整证明 |
| [fbundle/erdos90](https://github.com/fbundle/erdos90) | `master` / `7197bb6c2d2ef0180bde588b3c350a404278d7c4` | README、`Main.lean`、`Defs.lean`、`Axioms.lean`、Lake 和 Mathlib 配置 | 0 / 无 Release | `partial`；仍含 `brd_tower_data` 公理，README 也提示尚未完成手工核对 |
| [kim-em/erdos-unit-distance](https://github.com/kim-em/erdos-unit-distance) | `master` / 未确认 | `Main.lean`、`GeometricCore.lean`、`PointCount.lean`、Mathlib 与附加依赖 | 0 / 无 Release | `verified`；有 `lake exe cache get && lake build` 路径；旧工具链记录与当前 Lean 版本不一致 |
| [kim-em/erdos-unit-distance-comparator](https://github.com/kim-em/erdos-unit-distance-comparator) | `master` / 未确认 | `Challenge.lean`、`Solution.lean`、`verify.sh`、`comparator.json` | 0 / 无 Release | `verified`；有独立 comparator 路径；`enable_nanoda: false`，不能等同于双引擎复核 |
| [Flamehaven-Labs/openai-erdos-eq22-reproduction](https://github.com/Flamehaven-Labs/openai-erdos-eq22-reproduction) | `main` / `000d3101d2e232c7595e763108af60d8ebf2fca6` | README、`pyproject.toml`、验证脚本、数值复算代码、测试和 CI | 0 / Release badge `v0.2.5` | `verified`；README 记录 60 个单测、21 个验证项和 200-bit `mpmath`；明确不等于新证明 |
| [mobiusresearch/navier-stokes-theorem-1-1](https://github.com/mobiusresearch/navier-stokes-theorem-1-1) | `main` / 未确认 | README、`verification/verify.py`、`reproduce.sh`、正例/弱化例/未证明例、Lean 源码和日志 | 0 / 无 Release | `verified`；复核材料完整，但脚本主要核对已有记录，不重新运行全部内核 |
| [swarm-ai-research/navier-stokes-lean-check](https://github.com/swarm-ai-research/navier-stokes-lean-check) | `main` / 未确认 | README、`repro.sh`、语句差异、构建摘要和 Clay 条件说明 | 0 / 无 Release | `verified`；有语句一致性和 Lean 构建两步路径，完整构建需要较大磁盘空间 |
| [AniketWathore/Ramanujan](https://github.com/AniketWathore/Ramanujan) | `main` / 未确认 | README、`agent/package.json`、`agent-loop.ts`、Agent/数学工具/TUI 测试树 | 0 / 无 Release | `partial`；有多模型并行、文献来源和计算检查设计，端到端数学结果未确认 |
| [alexyyyander/proofweave](https://github.com/alexyyyander/proofweave) | `main` / 未确认 | README、MCP attempt API、隔离 Lean 执行器、protocol/runner/verification 测试和 CI | 3 / 无 Release | `verified`；研究记录、隔离重放、正负 fixture 和公开演示较完整，本身不证明具体定理 |
| [47thtechcorner/RayCodes_OpenAI_Navier_Stokes](https://github.com/47thtechcorner/RayCodes_OpenAI_Navier_Stokes) | `main` / 未确认 | README 和说明性内容 | 0 / 无 Release | `partial`；解释性线索，没有源码、依赖清单或测试 |
| [az9713/openai-navier-stokes-results](https://github.com/az9713/openai-navier-stokes-results) | `main` / 未确认 | README、`package.json`、`app/page.tsx`、`guide.mdx`、Pages workflow | 1 / 无 Release | `partial`；交互式说明站点，不是 PDE 求解器，也不提供 Lean 证明 |

## 证据排序

- 较强：`mobiusresearch`、`Flamehaven-Labs`、`swarm-ai-research`、`kim-em/erdos-unit-distance-comparator`。
- 中等：`openai/NavierStokesAndEuler`、`kim-em/erdos-unit-distance`、`plby/Erdos90`。
- 工程流程参考：`proofweave`、`Ramanujan`、`az9713`。
- 解释性线索：`47thtechcorner`。
- 需谨慎使用：`fbundle/erdos90`，因源码仍含公理，完成状态需继续人工核对。

## 对本题的可迁移做法

1. 每个研究对象记录固定 URL、分支、commit、工具链和依赖版本。
2. 分开记录定理/题面语句、源码构建、内核接受、独立复核和数值复算五类结果。
3. 为每个主张绑定一个入口文件和最短运行命令。
4. 同时保留正例、弱化例和故意未完成例，验证工具能否识别错误类型。
5. 将 README 自述、源码事实、CI 结果、公开日志和人工判断分别标注。
6. 明确复现范围，例如只复算某个公式或某个引理时，不把结果扩写成整篇证明。
7. 研究记录、输入来源、运行结果、工具链和人工判断分开存放，避免只依赖聊天记录传递上下文。

## 不可直接迁移的部分

- 外部仓库主要依赖 Lean、Lake、Mathlib、nanoda 或 Comparator，本项目主体是 Ascend C Kernel 和 CANN 工具链。
- 数学内核接受只说明给定形式化语句在对应依赖下成立，不能推出 NPU 指令、DMA 对齐、UB 容量或性能结论。
- 多 Agent 数量和 token 数不能代替可复现输入、编译日志、误差记录和独立运行结果。

## 限制

本轮通过 GitHub 公共 API、仓库页面和源码树读取上述材料；匿名 API 额度在中途受限，部分默认 commit、Issue 详情和 Release 详情未能继续获取。以上所有外部仓库都未在本机运行。

## 来源

仓库链接已直接列在表格中。更完整的访问日期、版本、用途和证据状态见 `../../sources.md`；OpenAI Agent 研究的综合结论见 `../../openai-agent-research.md`。
