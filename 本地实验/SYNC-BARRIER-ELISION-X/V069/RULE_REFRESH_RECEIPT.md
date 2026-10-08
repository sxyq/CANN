# RULE_REFRESH_RECEIPT

- Route: `SYNC-BARRIER-ELISION-X`
- Revision: `V069`
- Worktree: `/home/data4t2/lelinfeng/cann-route-w4-3-sync-barrier-elision`
- Branch: `exp/w4-compile-sweep/r-w4-3-sync-barrier-elision-x`
- Receipt issued: `2026-10-08T17:33:49Z` (same active Route context)
- The receipt was issued before the V069 edit. This file was materialized alongside the declared edit and before Compile; exact file-write timestamp was not captured.
- Rule refresh status: complete before the V069 edit; this file records that receipt without repeating the audit.

The completed refresh covered the current repository entry instructions; experiment and execution rules; local-performance, server-experiment, online-submission, and Git rules; project route/record conventions; and the `cann-mainline` and canonical Route-executor skills. The applicable constraints for this revision are:

1. Keep one Route, worktree, branch, and owner; preserve all earlier revision evidence.
2. Start V069 from the exact Local Best `R31B-V011`, not noisy V068. Parent and unedited Candidate SHA256 are both `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
3. Make one synchronization-axis performance change only. Compile immediately after the Candidate edit; run Correctness only after Compile PASS; run numeric, interleaved Parent/Candidate Local only after Correctness PASS and an eligible device/resource snapshot.
4. Preserve raw samples and load/jitter context; noisy results still receive numeric scores and are not promoted or treated as Official results. Release any assigned device after capture.
5. Keep evidence route-local and use a scoped commit. Do not write shared records, submit Online, push, disturb other processes, or alter history.
6. Do not close or replace the Route because of elapsed time, heartbeat, runtime, or network delay.

## State at receipt

- Previous committed revision: V068, `27af2ad2`.
- V068 verdict: rejected/noisy; it is not the V069 parent.
- Local Best remains exact `R31B-V011`.
- V069 scaffold Candidate was identical to Parent before the declared edit; no V069 Compile, Correctness, or Local had run.
