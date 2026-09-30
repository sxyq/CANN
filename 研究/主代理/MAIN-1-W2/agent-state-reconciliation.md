# MAIN-1 Wave-2 Agent State Reconciliation

DATE: 2026-09-30
BASELINE: origin/main `ed860e392d7604694ac6664da60aff1fc1f4c04f`
ROLE: MAIN-1 control-only record
SCOPE: state reconciliation and Track-B handoff audit; no kernel or shared-ledger changes

## Reconciliation rule

The five M1 branches were already present on GitHub when this reconciliation ran. Their
runtime Agent IDs were not recoverable from Git history or the current runtime, so they
are recorded as unknown rather than invented. Each branch contains one committed Track-B
handoff and no active runtime Agent is required for this closed research task.

## Route state

| Route | Worktree | Branch | Initial Agent ID | Initial State | Replacement Agent ID | Current Agent ID | Current State | Track-B | Handoff | Commit | Remote sync | Clean |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| SYNC-TOPOLOGY-CHAMPION-X | `/home/data4t2/lelinfeng/cann-w2-m1-sync` | `w2/m1/sync-topology` | UNKNOWN | PRE-EXISTING_REMOTE_HANDOFF; runtime not observable | NONE | NONE | HANDOFF_COMPLETE; no active runtime handle | YES | YES | `6a19f78c50cd663bb7e8de8f8836e442fe8f28be` | YES | YES |
| STORE-EPILOGUE-W2-X | `/home/data4t2/lelinfeng/cann-w2-m1-store` | `w2/m1/store-epilogue` | UNKNOWN | PRE-EXISTING_REMOTE_HANDOFF; runtime not observable | NONE | NONE | HANDOFF_COMPLETE; no active runtime handle | YES | YES | `80676bc73334fc0b7c3a912353fc02abdfec16b1` | YES | YES |
| EPI-ARITH-CHAMPION-W2-X | `/home/data4t2/lelinfeng/cann-w2-m1-epi` | `w2/m1/epi-arith` | UNKNOWN | PRE-EXISTING_REMOTE_HANDOFF; runtime not observable | NONE | NONE | HANDOFF_COMPLETE; no active runtime handle | YES | YES | `5031a5fa253407ab6f65f16b00e59e21312dd1e2` | YES | YES |
| SELECTIVE-FASTPATH-CHAMPION-X | `/home/data4t2/lelinfeng/cann-w2-m1-fastpath` | `w2/m1/selective-fastpath` | UNKNOWN | PRE-EXISTING_REMOTE_HANDOFF; runtime not observable | NONE | NONE | HANDOFF_COMPLETE; no active runtime handle | YES | YES | `9e5a573112a1dbed7b2450963b5e9961415f53b5` | YES | YES |
| SMALLMID-DATAFLOW-CHAMPION-X | `/home/data4t2/lelinfeng/cann-w2-m1-smallmid` | `w2/m1/smallmid-dataflow` | UNKNOWN | PRE-EXISTING_REMOTE_HANDOFF; runtime not observable | NONE | NONE | HANDOFF_COMPLETE; no active runtime handle | YES | YES | `47dacfa21f7da5158cb855567b81d8163576ff6a` | YES | YES |

## Handoff paths

- `研究/SYNC-TOPOLOGY-CHAMPION-X/TRACK-B-HANDOFF.md`
- `研究/STORE-EPILOGUE-W2-X/TRACK-B-HYPOTHESES.md`
- `研究/EPI-ARITH-CHAMPION-W2-X/TRACK-B-HANDOFF.md`
- `研究/SELECTIVE-FASTPATH-CHAMPION-X/TRACK-B-HANDOFF.md`
- `研究/SMALLMID-DATAFLOW-CHAMPION-X/track-b-handoff.md`

## Audit result

- All five documents are Track-B research only and contain 3-5 candidate hypotheses or an equivalent explicit candidate pool.
- Direct parents and source identities are recorded in the handoffs; STORE uses the explicitly approved `STORE-EPILOGUE-X V002` parent, while the other four use `R31B V011`.
- Duplicate audits cite prior routes/evidence and identify rejected or lower-confidence candidates; no implementation direction was selected by this record.
- `MAIN_SELECTED=NONE` for all five routes.
- `KERNEL_CHANGED=0`, `REVISION_CREATED=0`, `SERVER_RUN=0`, `PERFORMANCE_RUN=0`, `ONLINE_SUBMISSION=0`.
- The pre-existing remote branches were fetched and fast-forwarded locally. No reset, force push, deletion, or overwrite was used.

AGENT_STATE_RECONCILIATION: PASS_WITH_UNKNOWN_HISTORICAL_RUNTIME_IDS
