RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V036
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
ROLE=Route Agent; sole owner for this Route/worktree
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; 技术路线/技术路线总表.md; 技术路线/技术路线图.md; 技术路线/路线成绩表.tsv; 技术路线/全版本记录.tsv; V026 declaration/source; V032-V035 route-local declarations and results
PLANNING_SCOPE_SOURCE=Latest user-approved control in this owner context: use V026 as parent; row-scale placement/consumption only; no Rsqrt/math redesign, reduction, dtype, or scheduling changes; no Online/shared writes/other worktrees
V035_COMMIT=5e097215; full raw evidence preserved; verdict MEASUREMENT_BLOCKED; CURRENT_LOCAL_BEST remains V026
CURRENT_LOCAL_HOST=hwnput3; Ascend910B3; toolkit 8.5.0.alpha002; device 0 observed HBM 91%, AICore 32%, shared processes present; no process modified
V036_DIRECT_PARENT=V026
V036_SINGLE_CHANGE=In ProcessFp16FullRowOutputPipelined, materialize the existing per-row FP32 invRms as the same native-half scalar once before the two-tile output loop and reuse it for both existing post-gamma Muls; vector arithmetic, order, and precision are unchanged
V036_EXCLUSIONS=No scale/gamma order swap; no Rsqrt/math redesign; no reduction, dtype, scheduling, buffer, event, or dispatch change; no Online or shared-record write
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN
OTHER_WORKTREE=NOT_ACCESSED
PUSH=NOT_AUTHORIZED_OR_RUN
