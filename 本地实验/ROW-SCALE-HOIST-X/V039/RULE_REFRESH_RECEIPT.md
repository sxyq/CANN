RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V039
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
BASE_COMMIT=02b9de46
ROLE=Route Agent; sole owner for this Route/worktree
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; 技术路线/技术路线总表.md; 技术路线/技术路线图.md; 技术路线/路线成绩表.tsv; 技术路线/全版本记录.tsv; ROW-SCALE-HOIST-X V026-V038 declarations and V038 result
PLANNING_SCOPE=Continue OFAT only within the existing row-scale placement/order axis; use V026 as Direct Parent unless a Local Best is established
V038_RESULT=MEASUREMENT_BLOCKED; descriptive score=101.3937282230; pooled delta=-1.3745704467% candidate faster; CURRENT_LOCAL_BEST remains V026
V039_CHANGE=FP32 full-row row-scale operand placement only; apply invRms to an FP32 scratch copy of each gamma tile before multiplying the value tile
V039_PARENT=V026; source SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
V039_DUPLICATE_AUDIT=Distinct from V037 scale extent and V038 value-tile operation order; cached gamma is not mutated
EXECUTION_CONTEXT=hwnput3 current local server3 context; Ascend910B3/dav-2201; CANN 8.5.0.alpha002; NPU 0
SHARED_RECORDS=NOT_MODIFIED
OTHER_WORKTREE=NOT_ACCESSED
ONLINE=NOT_RUN
PUSH=NOT_AUTHORIZED_OR_RUN
