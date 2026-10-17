# CANN AddRmsNormBias

## 当前状态与范围

比赛已结束；W2、W3、W4、N01–N10 及其他技术路线均为历史归档，没有活动优化路线。此前连续优化、工作树创建和 Agent 派发指令仅供历史追溯，不构成当前启动实验的授权。

当前任务覆盖整个 cann 目录：分类保存基础源码、全版本源码与结果，剔除临时构建产物，维护文档，并按用户明确要求提交、推送到已有远端。默认不启动 NPU、重跑历史 Sweep 或提交 Judge。

## 源码与证据入口

- 基础实现：`源码/R31B-V011/submission.asc`，Official 45.16、15/15。
- 全版本源码：`归档/比赛结束-20261017/路线源码/对象/`，按 SHA256 保存独立对象。
- 路径映射：`索引/源码清单.tsv`；Git Blob 与提交版本见 `索引/Git源码对象.tsv`、`索引/Git源码版本.tsv`（均相对比赛结束归档）。
- 实验记录：`归档/比赛结束-20261017/实验记录/`；研究、旧调度和看板分别保存在 `研究资料/`、`历史调度/`。
- 路线与分数：`技术路线/`；原始结果、失败和 POOR/FRAGILE 结论保留，不伪造可靠 Best。

分支历史已经归档合入 main，独立路线 Kernel 不机械融合。原分支、登记中的 Git 工作树和 Git 历史保留；仓内原工作树已移到仓外恢复目录，详见比赛结束归档说明。

## 整理与数据保护

- 在用户指定范围内沿用已有源码、工具和记录；不新建 Runner、Scorer、Dashboard、调度器或平行账本。
- Compile、Correctness、Local、Profile 与 Official 分开记录；未执行或资料缺失使用 NONE/UNKNOWN。
- 源码正文、算术语义与原始实验数字不因归档而改写；历史报告和日志的旧路径保留为来源事实。
- 临时编译缓存、对象文件、Runner 二进制、生成输入输出和重复快照移出仓库时，保存可恢复位置与校验清单；不永久删除用户数据。
- 凭据、SQLite/Keychain、LaunchAgent、运行缓存、实验原件和其他用户数据不纳入清理；不停止、重启、迁移或修改其他用户任务。

## Git 与远端

使用精确路径按职责分批提交，不使用 `git add .`、`git add -A`、`reset --hard`、`git clean`、force push 或历史改写。保留分支与工作树，不把分类合并误写成已经验证的组合 Kernel。

默认只操作本地 Git；本次用户已明确要求提交并推送，沿用 origin 的 main，不新建 PR、发布配置或替代远端。

## 资料入口

- Main：`.agents/skills/cann-main-orchestrator/SKILL.md`
- Record：`.agents/skills/cann-record-owner/SKILL.md`
- 兼容路由：`.agents/skills/cann-mainline/SKILL.md`
- Route / Support / Online 技能留作历史入口，只有新的明确任务才使用。
- 归档导航：`归档/比赛结束-20261017/README.md`
- 数据安全与 Git：`项目规则/服务器实验规范.md`、`项目规则/Git工作流程.md`
- 比赛期间测量与线上协议：`项目规则/本地性能测试规范.md`、`项目规则/线上提交规范.md`

服务器 3 当前主机名为 hwnput3；归档工作不需要设备资源门槛，不执行其他用户的设备命令。
