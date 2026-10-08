RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V043
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
BASE_COMMIT=cf271144
ROLE=Route Agent; sole owner for this Route/worktree
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; V026 source/declaration; V022, V035, V039, V041, V042 declarations and relevant source diffs/results; BF16 float-compute precision standard and benchmark-construction guidance
PLANNING_SCOPE=Continue one-factor probes only inside ROW-SCALE-HOIST-X's existing row-scale placement/order axis; use V026 as Direct Parent unless a Local Best is established
ROUTE_STATE=V041 committed correctness/path blocked with no Local; V042 committed with Compile/Correctness PASS and Local MEASUREMENT_BLOCKED; CURRENT_LOCAL_BEST=V026
V043_CHANGE=In the BF16 ProcessNarrowMidOverlap epilogue, apply the existing FP32 invRms Muls to the already-converted gamma scratch instead of to valueLocal
V043_PARENT=V026; source SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
V043_DISPATCH=BF16 [128,3072], 40 vector cores, 3-4 rows/core; width exceeds the low-precision contiguous cutoff and is below kTileElems, selecting ProcessNarrowMidOverlap with resident parameters
V043_DUPLICATE_AUDIT=V022 changes BF16 scale/gamma order in ProcessNarrowMidOverlap; V039 moves scale to gamma scratch only in the FP32 full-row consumer. V043 changes only the scale operand in the distinct BF16 narrow-mid consumer
SHARED_RECORDS=NOT_MODIFIED
OTHER_WORKTREE=NOT_ACCESSED
ONLINE=NOT_RUN
PUSH=NOT_AUTHORIZED_OR_RUN
