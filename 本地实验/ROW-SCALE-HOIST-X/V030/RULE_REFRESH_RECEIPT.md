RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V030
TIMESTAMP_UTC=2026-10-07T04:18:28Z
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
BRANCH=exp/row-scale-hoist-x-w4
ROLE=Route Agent
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; 技术路线/技术路线总表.md; 技术路线/技术路线图.md; 技术路线/路线成绩表.tsv (ROW-SCALE-HOIST-X row); 技术路线/全版本记录.tsv (ROW-SCALE-HOIST-X entries)
SKILL_SOURCE=Local canonical route-executor skill exists; no origin/main fallback needed
CURRENT_FACTS=V029 RESULT.md and commit a6d0bb28 exist; V029 is noisy/negative, score 96.979171875, +3.114924645% slower; CURRENT_LOCAL_BEST=V026
V030_CHANGE=Move the FP16 full-row path row-scale multiply to FP32 before output conversion
COMPILE=NOT_RUN; both server3 attempts stopped at SSH DNS resolution before CMake
CORRECTNESS=NOT_RUN
LOCAL=NOT_RUN
ONLINE=FORBIDDEN
SHARED_RECORDS=NOT_MODIFIED
NEXT=Retry server3 Compile after cann-server3 name resolution is restored
