ROUTE=ROW-SCALE-HOIST-X
REVISION=V035
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
RULE_REFRESH=COMPLETE
READ=AGENTS.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; 技术路线/技术路线总表.md; 技术路线/技术路线图.md; 技术路线/路线成绩表.tsv; 技术路线/全版本记录.tsv; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; ops-profiling/SKILL.md; ascendc-env-check/SKILL.md
PLANNING_SCOPE=Current user control authorizes one row-scale placement/consumption-order factor from CURRENT_LOCAL_BEST=V026. No Rsqrt/math redesign, reduction, dtype, or scheduling changes. Preserve V032/V033 and all raw V034 evidence. Current local host only; no SSH, Online, or shared-record writes.
V034_AUDIT=Raw 96 samples per arm parsed and cross-checked against local-result.json; medians, CVs, maxima, and paired-window medians match. Parent stability failed; verdict remains MEASUREMENT_BLOCKED; V026 remains CURRENT_LOCAL_BEST.
NEXT_FACTOR=In ProcessNarrowMidOverlap FP16 epilogue, swap the existing native-half row-scale Muls and gamma Mul after FromFloat and before bias. No arithmetic precision or pipeline changes.
PARENT=V026; source SHA256=7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9
TARGET=FP16 [128,3072]; narrow-mid overlap dispatch, local rows > 1
