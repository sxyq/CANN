RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V044
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
BASE_COMMIT=431397e0
ROLE=Route Agent; sole owner for this Route/worktree
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; ops-precision-standard float_compute.md and benchmark_construction.md; V026 source; V021, V035, V039, V042, V043 declarations/results and relevant diffs
PLANNING_SCOPE=Continue one-factor probes only inside ROW-SCALE-HOIST-X's existing row-scale placement/order axis; use V026 as Direct Parent while V026 remains Local Best
ROUTE_STATE=V041 committed correctness/path blocked with no Local; V042 committed with Compile/Correctness PASS and Local MEASUREMENT_BLOCKED; V043 committed with Compile PASS and Parent/Candidate shared correctness failure, no Local; CURRENT_LOCAL_BEST=V026
V044_CHANGE=In FP16 ProcessNarrowMidOverlap with localRows==1, apply existing half invRms Muls to the newly loaded gamma tile before the existing output/gamma Mul
V044_PARENT=V026; source SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
V044_DISPATCH=FP16 [40,3072], 40 vector cores, one row/core; row width is above 2048 and at most 4096, selecting ProcessNarrowMidOverlap; residentParams=false ensures each row's loaded gamma is not reused after scaling
V044_DUPLICATE_AUDIT=V021 changes conversion-boundary placement in this consumer; V035 swaps scale/Mul order on the output operand with 3-4 rows/core and resident params. V044 scales the freshly loaded gamma operand only in the one-row case. V039 and V043 target other dtype consumers.
FP16_GATE=atol=2^-9 (0.001953125); rtol=2^-9 (0.001953125); required matched ratio >=0.99; max error limit=max(0.1,32*2^-10)=0.1
SHARED_RECORDS=NOT_MODIFIED
OTHER_WORKTREE=NOT_ACCESSED
ONLINE=NOT_RUN
PUSH=NOT_AUTHORIZED_OR_RUN
