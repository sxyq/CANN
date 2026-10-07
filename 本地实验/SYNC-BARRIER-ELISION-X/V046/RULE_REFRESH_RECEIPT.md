# RULE_REFRESH_RECEIPT — SYNC-BARRIER-ELISION-X V046

- Owner/context: the existing SYNC-BARRIER-ELISION-X Route owner; no replacement worktree or branch.
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`
- Read before the runner-label edit: this worktree's `AGENTS.md`; `.agents/skills/cann-mainline/SKILL.md`; canonical `.agents/skills/cann-route-executor/SKILL.md`; `项目规则/实验总则.md`; `项目规则/执行约定.md`; `项目规则/本地性能测试规范.md`; `项目规则/服务器实验规范.md`; `项目规则/Git工作流程.md`; `项目规则/线上提交规范.md`; and the applicable `ops-profiling` skill.
- Role boundary: edit and record only this Route's Candidate and V046 evidence. Do not write shared ledgers or dashboards, modify another Route, decide lifecycle, submit Online, push, SSH, reset, clean, rebase, or rewrite history.
- Experiment order: one Candidate change, Compile, Correctness, Local, Result, scoped commit. V046's Candidate change is the single removal of `SyncVToMTE2()` after per-row reductions in `ProcessSmallLowPrecisionContiguousBatched`; the later V046 runner edit changes only a stale measurement label and does not change Candidate source.
- V046 identity at receipt: Parent R31B-V011 source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; Candidate source SHA256 `dafe51fc1ea4582c3605222e282f3e68da556c8388ba3ca697fd00185e1f05f4`. Compile and seven FP16 bitwise correctness cases had passed; Local remained pending.
- Measurement: use device-event latency as primary, preserve every raw sample, throughput, wall latency, ordering, host/device load and jitter. Qualification and two interleaved 31-pair Local blocks use 60 warmups per invocation. A noisy result still receives a numeric score and is not promoted. Single-shape Local is not comparable to Official score.
- Device/process safety: Main assigned device 3 exclusively to V046 through measurement/result closeout. Do not wait for a low-load window or interfere with other processes. PID 2277907 is not to be signaled or restarted; the read-only check showed it absent.
- Stage at issuance: Candidate Compile PASS and Correctness PASS; fix the V046 qualification runner label, rebuild and verify identities, then Parent qualification.

This receipt was sent before the runner-label edit; this file records the same receipt with the completed V046 evidence.
