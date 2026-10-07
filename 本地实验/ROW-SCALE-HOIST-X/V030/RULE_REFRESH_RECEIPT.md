RULE_REFRESH_RECEIPT
ROUTE=ROW-SCALE-HOIST-X
REVISION=V030
TIMESTAMP_UTC=2026-10-07T05:18:09Z
WORKTREE=/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4
HOST=hwnput3
BRANCH=exp/row-scale-hoist-x-w4
ROLE=Route Agent
READ=AGENTS.md; .agents/skills/cann-mainline/SKILL.md; .agents/skills/cann-route-executor/SKILL.md; 项目规则/实验总则.md; 项目规则/执行约定.md; 项目规则/本地性能测试规范.md; 项目规则/服务器实验规范.md; 项目规则/线上提交规范.md; 项目规则/Git工作流程.md; 技术路线/技术路线总表.md; 技术路线/技术路线图.md; 技术路线/路线成绩表.tsv; 技术路线/全版本记录.tsv; ascendc-env-check/SKILL.md
CANONICAL_ROUTE_SKILL=.agents/skills/cann-route-executor/SKILL.md
CURRENT_FACTS=V029 result is committed at a6d0bb28; V029 local score 96.979171875, +3.114924645% slower; CURRENT_LOCAL_BEST=V026
V030_CHANGE=FP16 full-row row-scale placement before output conversion; direct parent V026; candidate SHA256 c75d584e86b1bd58cc54066c59691d143c186352bac413f32a275221e2df3ef3
COMPILE=PASS locally on hwnput3 after sourcing /usr/local/Ascend/ascend-toolkit/set_env.sh; CANN=8.5.T8.0.B060 (version_dir=8.5.0.alpha002); device and submission targets built; prior SSH/DNS failures retained
CORRECTNESS=PASS on device0; parent and candidate matched_ratio=1.0; max_abs_error=0.001953125
LOCAL=NEEDS_ONE_MORE_LOCAL; device0 interleaved P-C-P-C-P-C; 96 raw samples per arm; descriptive pooled medians 17.34 us parent and 11.97 us candidate; candidate/parent delta -30.968858132%; HIGH_CONTENTION_AND_TIME_VARIANT; paired direction reversed in final block
CURRENT_LOCAL_BEST=V026
OFFICIAL_SCORE=NONE
ONLINE=FORBIDDEN_NOT_RUN
SHARED_RECORDS=NOT_MODIFIED
OTHER_WORKTREE=NOT_ACCESSED
EVIDENCE=compile-v030-local-20261007T045222Z.log; correctness-build-local-20261007T045717Z.log; correctness-run-local-20261007T045843Z.log; local-build-20261007T050045Z.log; local-paired-20261007T050208Z.log; compile-result-local-20261007T045222Z.json; correctness-result-local-20261007T045843Z.json; local-result.json
NEXT=Commit V030 result evidence; do not advance Local Best; use V026 as the next revision parent
