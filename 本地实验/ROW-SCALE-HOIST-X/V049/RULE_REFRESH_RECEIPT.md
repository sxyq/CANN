# V049 Rule Refresh Receipt

- Route: `ROW-SCALE-HOIST-X`
- Worktree: `/home/data4t2/lelinfeng/cann-row-scale-hoist-x-w4`
- Branch: `exp/row-scale-hoist-x-w4`
- Pre-edit HEAD: `772645493ec68ac5881dcc6633572fc4abed99d7` (V048 result commit)
- Rule sources refreshed: `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/线上提交规范.md`; `项目规则/Git工作流程.md`; `技术路线/技术路线总表.md`; `技术路线/技术路线图.md`; `技术路线/路线成绩表.tsv`; `技术路线/全版本记录.tsv`.
- Route-local evidence refreshed: exact V026 source and declaration; V021/V022/V029; V032-V048 declarations and available results, with focused duplicate comparison to V022, V042-V048.
- Formal route tables contain no `ROW-SCALE-HOIST-X` row; no shared-record changes will be made. No route research files were found under `研究/`.
- Current Local Best: V026. V048 remains preserved and committed; its descriptive numeric result remains `119.552054493`, `-16.3544278481%`, classified `MEASUREMENT_BLOCKED`.
- V049 hypothesis: in BF16 one-row `ProcessNarrowMidOverlap`, move the existing FP32 `valueLocal` row-scale Muls ahead of the parameter-ready wait. Direct parent is exact V026 SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Distinctness: no previous V026 sibling isolates BF16 one-row scale placement at the parameter-ready wait; V022 covers BF16 arithmetic order, V043 covers resident multi-row gamma operand placement, and V042/V048 cover FP32/FP16 one-row wait placement.
- Device assignment received for device 0 V049 Parent/Candidate Correctness and, if both pass, Local through numeric capture. Fresh snapshot at `2026-10-08T03:34:57Z` reported 62105 MB free HBM; gate passed. Pre-Local snapshot at `2026-10-08T03:42:08Z` reported 60722 MB free HBM; Local proceeded under the explicit >=100 MB gate regardless of AICore/process load. Device 0 was explicitly released after numeric capture at `2026-10-08T03:49:06Z`; see `device-release.log`.
- Candidate source SHA-256 after the single edit: `6c952259f62da6812385310366869152a746dc15a1635a386cd3ad04323e9890`; the compile copy matches. The only source diff is the BF16 one-row scale movement before `paramReady` and its resident-path guard; see `diff.patch`.
- Compile: PASS for `device` and `submission` at `/tmp/cann-row-scale-hoist-x-v049-compile.CIoRNS`.
- Execution complete: one Candidate change -> Compile PASS -> Parent/Candidate Correctness PASS -> Local raw capture -> numeric result (`MEASUREMENT_BLOCKED`) -> device release -> scoped result commit. Local Best remains V026. No shared-record or Online work.
