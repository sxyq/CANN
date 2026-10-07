RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V038
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
BASE_COMMIT=e41a97aa775462f6d5a728a7d2c30dc6af1c5075
ROLE=Route Agent; sole owner for this Route/worktree
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; 技术路线/技术路线总表.md; 技术路线/技术路线图.md; 技术路线/路线成绩表.tsv; 技术路线/全版本记录.tsv; ROW-SCALE-HOIST-X V026-V037 declarations and route-local results
PLANNING_SCOPE=Continue OFAT only within the existing row-scale placement/order axis; each revision uses V026 as Direct Parent unless a Local Best is established
V038_CHANGE=FP32 full-row output per-tile row-scale/gamma order only; scale remains per tile and before bias
V038_PARENT=V026; source SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
V038_DUPLICATE_AUDIT=No prior declaration changes this exact consumer's per-tile FP32 scale/gamma order; V037 changes scale extent only
EXECUTION_CONTEXT=hwnput3 current local server3 context; Ascend910B3/dav-2201; CANN 8.5.0.alpha002; NPU 0
CURRENT_LOCAL_BEST=V026
SHARED_RECORDS=NOT_MODIFIED
OTHER_WORKTREE=NOT_ACCESSED
ONLINE=NOT_RUN
PUSH=NOT_AUTHORIZED_OR_RUN
