RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V042
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
BASE_COMMIT=e2fb6efc
ROLE=Route Agent; sole owner for this Route/worktree
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; V026 source/declaration; V027-V040 declarations and relevant source diffs; V041 result and evidence
PLANNING_SCOPE=Continue one-factor probes only inside the existing row-scale placement/order axis; use V026 as Direct Parent unless a Local Best is established
V041_STATE=Committed as e2fb6efc; Parent gate invalid/unresolved at [128,16384] and [128,12288]; Local not run
V042_CHANGE=FP32 [40,2050] ProcessNarrowMidOverlap; place row-scale Muls before the parameter-ready WaitFlag only
V042_PARENT=V026; source SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
V042_DUPLICATE_AUDIT=V029 changes FP32 scale/gamma order but retains the parameter-ready wait before scaling; V042 tests placement across that asynchronous wait, not the already-tested consumer order alone
V042_DISPATCH=width 2050 is >2048, <=4096, and not divisible by the FP32 row-stride 8; 40 rows on 40 vector cores yields one row per core and selects ProcessNarrowMidOverlap with nonresident parameters
EXECUTION_CONTEXT=hwnput3 current local server3 context; Ascend910B3/dav-2201; CANN 8.5.0.alpha002; NPU 0
SHARED_RECORDS=NOT_MODIFIED
OTHER_WORKTREE=NOT_ACCESSED
ONLINE=NOT_RUN
PUSH=NOT_AUTHORIZED_OR_RUN
