# RULE_REFRESH_RECEIPT - SYNC-BARRIER-ELISION-X V062

- RULE_REFRESH_UTC: `2026-10-08T12:24:49.374174278Z`.
- Owner/worktree/branch: same Route owner; `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`; `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`.
- V061: commit `876a2232`; `LOCAL_REJECTED_NOISY`; pooled paired-median score `+4.912068%`, median-latency ratio `+4.548211%`, 62 raw pairs preserved, device 3 released. It is not the Parent.
- Current Local Best and direct Parent: exact `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Workflow rules: current `AGENTS.md` instructions and the V061 full rule receipt were checked; `.agents/skills/cann-mainline/SKILL.md` and canonical `.agents/skills/cann-route-executor/SKILL.md` were read in this context. The V061 receipt records the current `项目规则/实验总则.md`, `执行约定.md`, `本地性能测试规范.md`, `服务器实验规范.md`, `Git工作流程.md`, and `线上提交规范.md`; no rule-file edits were made by V061.
- Route-only duplication audit: searched this Route's `REVISION-DECLARATION.md` and `RULE_REFRESH_RECEIPT.md` files for `outputReady`, `V_MTE3`, and store-wait descriptions. No declaration names deletion of the exact `WaitFlag<V_MTE3>(outputReady)` before this batched Store. V050 describes a distinct post-store `SyncMTE3ToV`; V061 deletes a distinct MTE3-to-V `outputRelease` wait after `Muls`. Status is `NO_MATCH_IN_SCOPED_DECLARATIONS`; no claim is made that this narrow search proves all historical source patches are duplicate-free.
- Single factor: delete only the `WaitFlag<V_MTE3>(outputReady)` immediately before `Store` in `ProcessSmallLowPrecisionContiguousBatched`. Candidate begins as an exact copy of R31B V011; no V061 Candidate change is inherited.
- Device policy: device 3 is not to be used before V062 Compile PASS. After PASS, take a fresh HBM/process snapshot and use only within the V062 assignment; preserve all existing processes.
- Prohibited actions: no shared ledger/dashboard writes, Official comparison, Online, push, reset, clean, rebase, history rewrite, or route closure/replacement.
- SLA: final V061 Local block log mtime/result timestamp `2026-10-08T12:09:53.904472160Z`. At this receipt time the gap is `14m55.470s`, already beyond the 180-second target. Record the actual V062 Candidate-edit timestamp and resulting total gap after Compile; do not backdate.

This receipt precedes the V062 Candidate edit. The immediate experimental action after that one edit is Compile.
