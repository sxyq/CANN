RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V037
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
ROLE=Route Agent; sole owner for this Route/worktree
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; 技术路线/技术路线总表.md; 技术路线/技术路线图.md; 技术路线/路线成绩表.tsv; 技术路线/全版本记录.tsv; V026 declaration/source; V027/V029 declarations and relevant source; V030/V032-V036 declarations and results
PLANNING_SCOPE_SOURCE=Latest user-approved controls: continue from V026; one unique row-scale placement/consumption factor; no Rsqrt/math redesign, reduction, dtype, or scheduling changes; no Online/shared-record/other-worktree access
V035_COMMIT=5e097215; preserved; Local MEASUREMENT_BLOCKED
V036_COMMITS=3af322cf evidence; 1f13f90d Local receipt reconciliation; preserved
V036_RECONCILED=Valid Local completed; score=95.2763391592; delta=+4.9578530016% candidate slower; Parent/Candidate medians=20.170001/21.170000 us; 96 samples per arm, none excluded; logical effective throughput=627.0921/597.4704 GB/s (not physical HBM); all 3 paired blocks slower; verdict MEASUREMENT_BLOCKED due high jitter/shared host+device activity, not runtime failure; V026 remains CURRENT_LOCAL_BEST
V037_DIRECT_PARENT=V026
V037_SINGLE_CHANGE=In ProcessFp32FullRowOutputPipelined, move the existing per-tile FP32 row-scale Muls from the two output tiles to one full-row Muls after invRms computation and before the output loop; preserve scale-before-gamma order and every other operation
V037_TARGET=FP32 [128,8192]; existing full-row dispatch requires width 8192 and multiple rows per core
V037_DUPLICATE_AUDIT=V029 leaves this FP32 full-row consumer at per-tile scale before gamma; V032 targets FP16 full-row and crosses its conversion boundary; V033/V034/V035 are FP16 gamma-order swaps; V036 hoists only a scalar half conversion. V037 changes only FP32 scale extent/placement, with no scale/gamma order change.
V037_EXCLUSIONS=No math/Rsqrt, reduction, dtype, scheduling, buffer, event, dispatch, Online, or shared-record changes
OFFICIAL_SCORE=NONE
ONLINE=NOT_RUN
OTHER_WORKTREE=NOT_ACCESSED
PUSH=NOT_AUTHORIZED_OR_RUN
