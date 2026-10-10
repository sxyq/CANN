# CANN AddRmsNormBias

本目录是 CANN AddRmsNormBias 的本地源码、实验记录和资料仓库。W2、W3、W4 的历史源码、结果、提交记录和失败材料继续保存在原位置；历史资料只用于查阅，不改变当前活动内容。

## 当前研究方向

当前活动方向为 N01-N05：

- `N01 REGISTER-LIVE-RANGE-X`：研究宽行 Vector 临时 FP32 值的寄存器生命周期。
- `N02 CODE-FOOTPRINT-ICACHE-X`：研究辅助函数布局和 inline/noinline 对热点代码的影响。
- `N03 VECTOR-ILP-DUALCHAIN-X`：研究独立 Vector 操作的指令级并行。
- `N04 GM-CACHE-STREAMING-X`：研究大工作集 GM/L2 访问策略。
- `N05 SCALAR-ADDRESS-GENERATION-X`：研究 `ProcessWideLowPrecision` 的 Scalar 地址生成。

各方向的源码、参数、算术、访存、同步和 Tile 语义以实际代码与实验结果为准。历史 W2/W3/W4 资料不自动成为当前研究内容。

## 本轮五路线执行边界

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
- 只在资源不足、真实运行错误、可能影响其他用户任务或服务器不可连接时停止当前操作；负载、其他进程和设备占用作为上下文记录。
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
