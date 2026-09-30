# MAIN-1 Wave-2 Agent State Reconciliation

- Date: 2026-09-30
- Main branch: `main1/champion-exploit`
- Canonical `main` at review: `ed860e392d7604694ac6664da60aff1fc1f4c04f`; clean and equal to `origin/main`.
- Main-1 head at review start: `ef7996c7f5473e7581888549c9744623efd26295`; equal to `origin/main1/champion-exploit`.
- Official Champion: `R31B V011 / 45.16`.

## Launch lineage

The existing campaign record reports ten runtime launch attempts returning 429 before the current five-Agent roster was registered. The committed record does not associate those attempts with returned Agent IDs or individual routes, so no per-route assignment is inferred here.

Commit `17b7ea28` recorded the first five IDs below as `STARTED`. At reconciliation, `wait_agent` returned `not_found` for each ID; `close_agent` also returned `not found`. The later campaign registration is in commit `9341b80e`. The EPI Agent registered there (`01a0f039-2319-7b80-a504-2c2af0d0f95a`) later exited on a concurrency limit and was replaced by `01a0f05a-3dd0-7fa1-80da-dd5b51ba0784`, as recorded in commit `ef7996c7`.

The five current Agents accepted a follow-up handoff-completion request. Each returned a completion report after committing its handoff. Main closed the completed Agent IDs; no second Agent was created for any route.

## Route state

| ROUTE | WORKTREE | BRANCH | INITIAL_AGENT_ID | INITIAL_AGENT_STATE | REPLACEMENT_AGENT_ID | REPLACEMENT_REASON | CURRENT_AGENT_ID | CURRENT_AGENT_STATE | TRACK_B_STARTED | HANDOFF_EXISTS | HANDOFF_COMMITTED |
|---|---|---|---|---|---|---|---|---|---|---|---|
| SYNC-TOPOLOGY-CHAMPION-X | `/Users/sunyiyang/.codex/worktrees/w2-m1-sync/cann` | `w2/m1/sync-topology` | `01a0f00c-ce63-7c33-b0ed-3b22136bb65e` | `STARTED` in bootstrap; later lookup `not_found` | `01a0f038-7eae-7e13-9130-2dd37ca68948` | Current roster entry after the recorded launch failures; per-route mapping of the earlier 429 attempts is unavailable | `01a0f038-7eae-7e13-9130-2dd37ca68948` | `completed; closed by Main` | YES | YES | YES, final `9071292b` |
| STORE-EPILOGUE-W2-X | `/Users/sunyiyang/.codex/worktrees/w2-m1-store/cann` | `w2/m1/store-epilogue` | `01a0f00d-a9b6-7a80-a620-c7ba276b4655` | `STARTED` in bootstrap; later lookup `not_found` | `01a0f039-1cbb-79d1-bb77-9f63f830b5f3` | Current roster entry after the recorded launch failures; per-route mapping of the earlier 429 attempts is unavailable | `01a0f039-1cbb-79d1-bb77-9f63f830b5f3` | `completed; closed by Main` | YES | YES | YES, final `415a2429` |
| EPI-ARITH-CHAMPION-W2-X | `/Users/sunyiyang/.codex/worktrees/w2-m1-epi/cann` | `w2/m1/epi-arith` | `01a0f00d-aa4b-7b30-8588-943dcb45b9a6` | `STARTED` in bootstrap; later lookup `not_found` | `01a0f039-2319-7b80-a504-2c2af0d0f95a` → `01a0f05a-3dd0-7fa1-80da-dd5b51ba0784` | First EPI roster Agent exited on the concurrency limit; the second took the same route/worktree/branch | `01a0f05a-3dd0-7fa1-80da-dd5b51ba0784` | `completed; closed by Main` | YES | YES | YES, final `0ce5441e` |
| SELECTIVE-FASTPATH-CHAMPION-X | `/Users/sunyiyang/.codex/worktrees/w2-m1-fastpath/cann` | `w2/m1/selective-fastpath` | `01a0f00d-aab7-7de3-8cc4-fe16db4e5d14` | `STARTED` in bootstrap; later lookup `not_found` | `01a0f039-295d-7900-8099-8097e2e68b78` | Current roster entry after the recorded launch failures; per-route mapping of the earlier 429 attempts is unavailable | `01a0f039-295d-7900-8099-8097e2e68b78` | `completed; closed by Main` | YES | YES | YES, final `4d8e073e` |
| SMALLMID-DATAFLOW-CHAMPION-X | `/Users/sunyiyang/.codex/worktrees/w2-m1-smallmid/cann` | `w2/m1/smallmid-dataflow` | `01a0f00d-ab3a-74c1-a181-f1b26936c6c3` | `STARTED` in bootstrap; later lookup `not_found` | `01a0f039-2fb0-7d52-aacb-f8e668ba426c` | Current roster entry after the recorded launch failures; per-route mapping of the earlier 429 attempts is unavailable | `01a0f039-2fb0-7d52-aacb-f8e668ba426c` | `completed; closed by Main` | YES | YES | YES, final `7f51d250` |

## Handoff references

| ROUTE | HANDOFF_PATH | CURRENT_COMMIT | REMOTE_SYNC | SUPPLEMENT_STATUS |
|---|---|---|---|---|
| SYNC-TOPOLOGY-CHAMPION-X | `研究/SYNC-TOPOLOGY-CHAMPION-X/TRACK-B-HANDOFF.md` | `9071292b` | YES | Complete; 3 hypotheses |
| STORE-EPILOGUE-W2-X | `研究/STORE-EPILOGUE-W2-X/TRACK-B-HYPOTHESES.md` | `415a2429` | YES | Complete; 3 hypotheses, one infeasible, one duplicate rejected |
| EPI-ARITH-CHAMPION-W2-X | `研究/EPI-ARITH-CHAMPION-W2-X/TRACK-B-HANDOFF.md` | `0ce5441e` | YES | Complete; one distinct hypothesis, pool exhausted |
| SELECTIVE-FASTPATH-CHAMPION-X | `研究/SELECTIVE-FASTPATH-CHAMPION-X/TRACK-B-HANDOFF.md` | `4d8e073e` | YES | Complete; 3 donor hypotheses |
| SMALLMID-DATAFLOW-CHAMPION-X | `研究/SMALLMID-DATAFLOW-CHAMPION-X/track-b-handoff.md` | `7f51d250` | YES | Complete; 3 distinct hypotheses, one duplicate rejected |

## Current limits

- `AGENT_STATE_RECONCILIATION=PASS`; all five current Agent IDs completed and were closed after their handoffs were verified.
- `M1_TRACK_B_HANDOFFS=5/5`; every route branch is clean and matches its remote branch.
- `MAIN_SELECTED=NONE` for all five routes pending Planning / Review.
- No Revision, Candidate Kernel, build, correctness run, performance run, server3 job, or Online submission was created by Main-1 in this reconciliation.
- Shared canonical ledgers were not modified.
- `NEW_ROUTES_OUTSIDE_APPROVED_5=0`.
