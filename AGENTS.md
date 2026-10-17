# CANN AddRmsNormBias

本目录是 CANN AddRmsNormBias 的源码与归档仓库。比赛已结束；W2、W3、W4 与 N01–N10 的源码、结果、提交和失败材料只作历史追溯，不存在当前活动优化路线。

## 比赛结束状态更新（2026-10-17）

比赛已结束，N01-N10 路线进入只读归档状态，不再有活动优化路线。此前的路线执行约束保留为历史规则，不构成当前继续开新 Revision 的指令。基础提交源码为 `源码/R31B-V011/submission.asc`（Official 45.16）；完整路线源码、结果、临时运行材料和 Git refs 见 `归档/比赛结束-20261017/`。当前目录中 `worktrees/`、`server_runs/`、W2/W4 临时包等已迁入该归档，所有 Git 分支和登记中的工作树均保留。

## 路线状态（比赛结束后只读归档）

N01-N10 均已结束，不存在当前活动优化路线。路线源码与实验材料只用于回顾和追溯；比赛结束后不再启动 Candidate 或设备实验。

- N01-N05：Register live range、code footprint/I-cache、Vector ILP、GM/L2 streaming、scalar address generation。
- N06-N10：UB bank/buffer placement、parameter tile reuse、tail GM transaction、RMS reduction topology、row/Core balance。

路线概览见 `技术路线/技术路线总表.md`；比赛结束总归档见 `归档/比赛结束-20261017/README.md`。不同路线保留独立变体，不合并成一个未经验证的 Kernel。

## 历史五路线执行边界（已结束，仅供追溯）

以下约束描述比赛期间的执行方式，不授权或要求在归档状态下继续实验。

- 五条路线均以 `R31B V011 / Official 45.16` 为性能对照；主要优化机制必须互不重叠。
- 固定关系为 `1 Route = 1 Agent = 1 Context = 1 Branch = 1 Local Worktree`。Route Agent 只能读取、修改和提交自己的本地工作树，以及明确授权的公共规则和基线资料；禁止读取其他 Route 的 worktree、分支源码和未公开实验。跨路线比较由 Main 做只读审计。
- 完整源码相似度和有效修改相似度目标均不超过 80%，必须报告真实数据，不得用改名、格式变化、注释或无意义重写制造差异。
- Main 负责协调、资源与重复研究审计；Route Agent 负责真实 Compile、Correctness、Local、结果保存和本地版本提交。普通 Revision 沿用当前 Route 的上下文、分支和 worktree，不等待 Main 重新批准；任务恢复或规则变化时按现行规则读取入口。
- 不恢复已取消的额外 Official 提交条件，不新增调度器、审批系统、Dashboard、常驻隔离工程或重复执行链。服务器安全、Git 数据保护和真实实验步骤继续生效。
- 当前工具若无法提供系统级目录权限、Git 对象隔离或逐 Agent 访问日志，必须报告实际限制；worktree 目录分开和 Agent 自述不能被写成强隔离证明。

## 本地工作约定

- Main、Route、Support、Record 和 Online 这些名称用于说明工作内容，不形成额外的编辑或执行限制。
- 任意本地任务都可以在用户指定范围内直接读取、修改、测试和记录已有文件；优先沿用已有入口，不创建重复脚本、重复账本或平行执行链。
- 实验记录保留真实命令、返回码、设备、负载、原始输出、结果解释和源码提交；成功、失败、未执行和负结果都按实际情况保存。
- Local 测量、Compile、Correctness、Profile 和 Online 结果分别记录，不互相替代，也不因缺少某项记录改变源码处理范围。
- 线上提交只使用现有工具和平台实际要求；平台返回的错误按原样保存，不伪造结果。

## 资源与数据保护

- 服务器访问使用已配置的 `cann-server3` 入口，凭据不写入源码、日志或仓库。
- 只在资源不足、真实运行错误、可能影响其他用户任务或服务器不可连接时停止当前设备/服务器命令，并记录原因；安全的本地源码、版本和结果处理继续。负载、其他进程和设备占用作为上下文记录，不是源码工作门槛。
- 不停止、杀掉、重启、迁移或修改其他用户的进程、服务、容器、数据和系统配置。
- 保留源码、凭据、SQLite、Keychain、LaunchAgent、运行缓存、实验结果、失败材料、分支、工作树和 Git 历史。
- 不使用 `reset --hard`、`clean`、force push 或历史改写。

## Git 与远端行为

- 使用精确路径分批创建本地提交；一个提交只放同一职责的实际修改，不纳入用户已有的无关改动。
- 默认只操作本地 Git，不推送、不创建远端对象、不添加远端审查、自动构建或自动发布配置。
- 用户明确要求远端操作时，仍沿用已有对象和入口，不创建同类替代实现。

## 资料入口
- Main：`.agents/skills/cann-main-orchestrator/SKILL.md`
- Route：`.agents/skills/cann-route-executor/SKILL.md`
- Support：`.agents/skills/cann-support-research/SKILL.md`
- Record：`.agents/skills/cann-record-owner/SKILL.md`
- Online：`.agents/skills/cann-online-owner/SKILL.md`
- 兼容路由：`.agents/skills/cann-mainline/SKILL.md`
- 资源安全：`项目规则/服务器实验规范.md`
- 本地测量：`项目规则/本地性能测试规范.md`
- Git 保存：`项目规则/Git工作流程.md`
# 历史 C2C CONTROL — N06–N10 全新极简工作树与严格子代理隔离（比赛结束，不再执行）

**PRIORITY = P0**

**SCOPE = SECOND_WAVE_ONLY**

**NEW_ROUTES = N06,N07,N08,N09,N10**

**MAX_ACTIVE_AGENTS = 5**

**MODE = MINIMAL_WORKTREE / SOURCE_ONLY / NO_RESEARCH_DOCUMENTS**

此指令规定 N06–N10 的工作树和源码访问范围；任何批次收口、审批、完整性检查或评分状态都不是开始或继续工作的前置条件。

N01–N05 与 N06–N10 可并行推进，不得为启动新路线提前停止正在运行的合法实验。实际 Agent 槽位不足时按平台容量轮换，不等待全部路线或上一批收口。

第二批必须使用全新、极简的本地工作树，不复制任何旧路线的工作树结构或研究材料。

## 1. 工作树与并行执行

N06–N10 可立即建立或沿用现有专属工作树并开始工作，不等待 N01–N05 收口、设备命令结束或 Agent 槽位全部释放。

平台并发容量只决定同时运行数量，不构成审批或阶段门禁；任一路线完成一个版本后可独立继续，不等待其他路线。

不删除 N01–N05 原有工作树。

不将第一批 Candidate 合并到第二批。

不得让旧 Agent 和新 Agent 同时写入同一目录。

## 2. 五条新路线

N06 — UB-BANK-PLACEMENT-X

N07 — PARAMETER-TILE-REUSE-X

N08 — TAIL-GM-TRANSACTION-X

N09 — RMS-REDUCTION-TOPOLOGY-X

N10 — ROW-CORE-BALANCE-X

这些是第二批预定研究方向。

轻量机制去重可以与路线执行并行，不是创建工作树或开始源码工作的前置条件。发现机制重叠时，在当前路线边界内调整或细化假设并记录，不等待 Main 审批。

**新 Route Agent 不参与跨路线历史扫描。**

研究题目和技术边界一旦确定，直接写入该 Agent 的初始任务消息。

不把历史研究资料交给子代理。

## 3. 全新本地最简 Worktree

每条路线创建一个全新本地 Git Worktree 和 Branch。

全部从同一正式干净基线建立：

`R31B V011 / Official 45.16`

准确使用当前仓库中已核实的公共源码提交，而不是使用旧 N 系列 Candidate。

建议目录：

```text
worktrees/
  n06-ub-bank-placement/
  n07-parameter-tile-reuse/
  n08-tail-gm-transaction/
  n09-rms-reduction-topology/
  n10-row-core-balance/
```

每个目录均为独立 Git Worktree。

使用 Git Sparse Checkout 或等效的最小检出方式，只保留实际必需的文件。

绝不把完整研究仓库复制五份。

## 4. 每个 Worktree 允许保留的内容

目标结构：

```text
n06-ub-bank-placement/
├── kernel/
│   └── submission.asc
├── build/
│   ├── CMakeLists.txt
│   └── [现有构建必须使用的最小适配文件]
├── tools/
│   ├── run.sh
│   └── [已有 Correctness/Local 所需执行文件]
├── config/
│   └── [实际编译环境必需配置]
└── results/
```

N07–N10 使用相同的最小结构。

上述结构是目标示意，不要求重新发明文件命名；优先沿用仓库实际已验证成功的入口。

依赖核查在实际使用时进行，不作为开始源码工作的前置条件：

- Candidate 能实际进入 CANN 编译。
- 构建入口可用时运行真实 Build；不可用时记录阻碍并继续可做的源码工作。
- Correctness 和 Canonical Local 入口可用且适合当前变更时运行；未运行或入口缺失只记录状态，不阻止下一次修改。
- 能保存 Source/Artifact SHA。
- 能保存原始测量。
- 能计算现有合法 Local Score。

`results/` 最初可以为空。

不得为了减少文件数而删除编译必须的头文件、适配器或运行时依赖。

## 5. 严禁向新 Agent 提供研究文档

新工作树不允许包含：

- README.md
- 历史 HANDOFF
- 历史 RESULTS.md
- 研究计划书
- 旧路线设计文档
- 旧路线实验报告
- 旧版本 Candidate
- 旧 Git Diff
- 其他 Agent 的代码
- 其他 Agent 的工作记录
- W2/W3/W4 大量历史实验资产
- 历史归档目录
- CodeGraph 索引
- 历史聊天导出
- 无关 Markdown
- 大量失败日志
- 其他路线的评分账本

不要把 `AGENTS.md`、Skill 文件或研究知识包复制给新 Agent 作为工作树材料。

Agent 必须遵守的必要规则，通过启动消息一次性下发。

自动生成的机器可读实验结果、原始日志与 SHA 文件可以保留在自己的 `results/` 中；禁止额外生成研究报告代替实际实验。

## 6. 子代理只能访问自己的工作树

每个 Agent 的唯一项目源码访问根目录就是自己的 Worktree。

N06 Agent：

只能访问 `n06-ub-bank-placement`。

N07 Agent：

只能访问 `n07-parameter-tile-reuse`。

N08 Agent：

只能访问 `n08-tail-gm-transaction`。

N09 Agent：

只能访问 `n09-rms-reduction-topology`。

N10 Agent：

只能访问 `n10-row-core-balance`。

禁止：

- 读取主仓库源码目录。
- 读取其他 Route Worktree。
- 读取旧 N01–N05 Worktree。
- 读取其他路线私有 Git 对象。
- 用 git show/grep 绕过限制。
- 通过绝对路径访问其他项目内容。
- 通过符号链接访问外部研究源码。
- 扫描整个仓库历史。
- 修改其他 Route 的 Branch。
- 修改 main。
- 运行跨路线搜索脚本。
- 复制其他路线的 Kernel Candidate。

公共基线源码与合法构建依赖，必须在启动前由有权限的初始化负责人放入自己的最小工作树。

Agent 启动后不需要访问主仓库查找缺失文件。

如平台提供可强制的 Agent 文件系统根目录或沙箱限制，必须实际配置，而不是仅在提示词中声明禁止。

Git Worktree 本身共享 Git 对象库，只能提供软隔离；如没有强制权限机制，必须如实报告，不能宣称实现了 OS 级强隔离。

不能为满足隔离要求而忽视实际执行能力。

## 7. 本地自动编译脚本

每条新路线必须具备从工作树内部发起真实编译的能力。

唯一自动执行入口应尽量保持简单：

```bash
./tools/run.sh build
./tools/run.sh correctness
./tools/run.sh local
```

实际命令以仓库中已经验证的脚本为准。

允许把现有脚本作最小化包装，但禁止新建第二套编译系统、Runner 或评分器。

`build`：

- 校验当前 Candidate 身份。
- 调用已有 CANN 构建链。
- 保存编译输出和 Artifact SHA。

`correctness`：

- 使用当前编译产物。
- 调用已有测试入口。
- 保存真实通过/失败结果。

`local`：

- 使用已有 Canonical 评分协议。
- 完成全部约定测试项。
- 保存 Parent/Candidate 原始数据。
- 调用现有评分程序。
- 输出真实 Local Score 与质量状态。

不允许将单 Shape 局部测量冒充完整分数。

## 8. 本地开发、远端实际编译

若当前 macOS 本地没有可用 Ascend C 工具链：

保持本地编辑和 Git 提交。

由本工作树内的自动脚本通过：

`ssh cann-server3`

执行真实远端临时编译及设备测试。

上传的必须是本 Route 所需最小源码和构建输入。

远端每个 Route 使用自己独立的临时实验目录。

禁止远端读取、修改其他 Route 的私有源码和实验目录。

服务器上可调用已安装的 CANN SDK、编译器及系统库。

不允许为了完成构建而复制完整主仓库或别人的工作树。

保持服务器安全，不重置或杀死其他用户进程。

## 9. Agent 启动指令

每个新 Agent 的初始任务只包含：

- Route ID。
- Worktree 绝对路径。
- Branch。
- 公共 Parent SHA。
- 当前技术研究机制。
- 唯一允许的代码修改范围。
- 禁止修改的其他机制。
- 唯一 Build/Correctness/Local 入口。
- 设备安全约束。
- Git 精确提交约束。
- 版本生命周期。
- 连续执行要求。

不得向新 Agent 发送完整项目历史总结。

不得发送其他路线源码、评分细节或实验失败历史。

不得让 Agent 再从主仓库寻找文档。

启动后立即进入当前 Kernel 源码，识别一个单点优化机会。

## 10. 每版执行与非阻塞验证

```text
最小 Worktree
      ↓
读取本路线源码
      ↓
确定一个性能假设
      ↓
只修改一处 Kernel 代码
      ↓
Git Commit 固定源码
      ↓
本地脚本触发真实 CANN Build
      ↓
按变更影响运行可用的 Correctness / Canonical Local / Profile
      ↓
保存已执行项目的 Raw / SHA / Score 和未执行原因
      ↓
有新结果时提交结果
      ↓
继续下一次单点修改，不等待其他验证或路线
```

编译、Correctness 或测量失败时保存证据；可在当前 Revision 修复，也可基于失败信息开始下一项源码工作。失败、缺失结果和未执行项目均不阻止后续 Revision。

不得升级版本号掩盖失败。

不得因为单次失败退出整夜任务。

暂时没有明确优化假设时，继续从当前 Kernel 和已获得的编译或测量事实中寻找可验证方向；不得仅因“无新假设”宣布路线收口或停止。区分探索性改动与已验证的性能结论，不虚构收益。

当前命令和结果保存完成后即可独立开始下一轮，不设置收口或等待窗口。

## 11. 新路线与旧路线的数据隔离

N06–N10 不继承：

- N01–N05 Candidate。
- N01–N05 Local Score。
- N01–N05 的未晋升 Best。
- 旧路线的 Runner 绑定。
- 旧路线的 Source SHA。
- 旧路线 HANDOFF。
- 旧路线实验目录。

允许复用的只有已经授权公开且确实必要的：

- 正式 Champion 源码。
- 最小构建模板。
- 现有 Correctness 入口。
- 已发布 Canonical Local 测试与 Scorer。
- 必要运行时配置。

这些文件在需要时纳入该路线自己的最小环境；依赖缺失不阻止其他可进行的源码工作。

## 12. 过夜持续执行

第二批启动后，五个 Agent 各自独立连续执行。

不得等待五条路线同时完成一个版本。

不得每次重新请求 Main 审批。

不得因为 Git Push、非安全 Owner Ack 或状态报告阻止合法本地实验。

如果某个 Agent 中断：

从其工作树内的真实源码提交、结果状态和最后一个实际完成的命令恢复。

不依赖已删除的旧路线文档。

不要创建新的常驻调度器。

不要虚报已经在后台持续运行。

## 13. 非阻塞状态核验

以下项目用于开始后的状态审计，不是工作树创建、Agent 启动或源码修改的前置条件：

- N06–N10 五个新 Worktree 已存在。
- 每个 Worktree 来自相同正式基线。
- 每个目录都采用最小检出。
- 不含历史研究文档。
- 不含其他路线 Candidate。
- 只有本路线需要的源码和编译依赖。
- 每个目录都能通过自身入口触发真实 CANN Build。
- Correctness 和 Local 入口已经绑定。
- Agent 工具的实际访问范围已经确认。
- 未声明无法保证的强隔离。
- 五个 Agent 可以分别开始独立修改。
- Main-1、第一批 N01–N05、W4 归档均未被修改。
- 总并发 Agent 不超过五个。

如果最小环境缺少某项依赖，在实际需要该能力时补入对应必需文件或记录该项未执行；不因该缺项阻断其他工作，也不复制整个历史仓库。

## 最终命令

**立即推进 N06–N10；不等待 N01–N05 收口、审批、验证门槛或路线间同步。**

每棵只包含必要 Kernel、编译脚本、测试入口和运行配置。

不提供任何历史研究文档。

各 Agent 只在自己的工作树内读取、修改和提交。

从本地入口发起真实 CANN 编译，并在入口可用时运行正确性与 Local 评分；这些结果用于记录，不作为继续源码工作的门槛。

保留原有唯一评分规则，不创建第二套系统。

让五个新 Agent 在严格隔离的技术路线中连续执行单点优化，持续推进过夜实验。
