RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V040
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
BASE_COMMIT=a372880ee4f3bd7022027afd9e4991c22d3f659d
ROLE=Route Agent; sole owner for this Route/worktree
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; 技术路线/技术路线总表.md; 技术路线/技术路线图.md; 技术路线/路线成绩表.tsv; 技术路线/全版本记录.tsv; ROW-SCALE-HOIST-X V026-V039 declarations and V038/V039 results
PLANNING_SCOPE=Continue one-factor probes only within the existing row-scale placement/order axis; use V026 as Direct Parent unless a Local Best is established
V038_V039_STATE=Committed; MEASUREMENT_BLOCKED; CURRENT_LOCAL_BEST remains V026
V040_CHANGE=FP32 [128,4096] ProcessSmallFp32FullTileBatched row-scale/gamma order only
V040_PARENT=V026; source SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
V040_DUPLICATE_AUDIT=V027/V028 target other small-FP32 consumers; V029 leaves this dedicated helper unchanged; V037-V039 target the separate FP32 full-row helper
EXECUTION_CONTEXT=hwnput3 current local server3 context; Ascend910B3/dav-2201; CANN 8.5.0.alpha002; NPU 0
SHARED_RECORDS=NOT_MODIFIED
OTHER_WORKTREE=NOT_ACCESSED
ONLINE=NOT_RUN
PUSH=NOT_AUTHORIZED_OR_RUN
