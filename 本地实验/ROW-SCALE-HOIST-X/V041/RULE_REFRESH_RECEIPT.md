RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V041
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
BASE_COMMIT=57934c3b
ROLE=Route Agent; sole owner for this Route/worktree
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; V026 source/declaration; V027-V037 declarations; V038-V040 declarations and result evidence
PLANNING_SCOPE=Continue one-factor probes only inside the existing row-scale placement/order axis; use V026 as Direct Parent unless a Local Best is established
V038_V039_STATE=Committed and preserved; not recreated
V040_STATE=Committed as 57934c3b; MEASUREMENT_BLOCKED; CURRENT_LOCAL_BEST remains V026
V041_CHANGE=FP32 [128,16384] ProcessWideFp32FullCacheRows row-scale/gamma order only
V041_PARENT=V026; source SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
V041_DUPLICATE_AUDIT=V027-V029 and V033-V035 target other consumers or precision/extent dimensions; V037-V039 target the non-wide FP32 full-row branch; V040 targets the small FP32 full-tile branch
V041_DISPATCH=width > 8192 enters widePath_; ProcessWideFp32 routes to ProcessWideFp32FullCacheRows
V041_RESULT=Parent and Candidate fail correctness at [128,16384]; nearby [128,12288] also fails on both while dispatching the same branch; Local not run
V041_NEXT=Commit as correctness/measurement blocked, then continue a sibling from V026
EXECUTION_CONTEXT=hwnput3 current local server3 context; Ascend910B3/dav-2201; CANN 8.5.0.alpha002; NPU 0
SHARED_RECORDS=NOT_MODIFIED
OTHER_WORKTREE=NOT_ACCESSED
ONLINE=NOT_RUN
PUSH=NOT_AUTHORIZED_OR_RUN
