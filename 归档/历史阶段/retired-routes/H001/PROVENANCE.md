# H001 Provenance

## Isolation policy (as assigned)
- Role: Fresh Small-D / High-R Specialist.
- Forbidden reads: historical R031, R31A compute, R31B compute.
- Allowed: own workspace, shared contracts, own online results, Main feedback for H001 only.
- Note: V003/V002 agent was allowed to read `phase4/workspaces/B001/kernel.txt` **for judge-template shape only**, not architecture.

## Evidence checklist

| Item | Finding | Evidence |
|------|---------|----------|
| Agent allowed to read historical R031? | NO | assignment prompt |
| Did agent read R031/R31A/R31B compute? | NOT OBSERVED | handoffs cite H001 + B001 template shape + shared contracts |
| Main modified H001 candidate source? | NO | Main only control/online/commits |
| Main-authored candidate revision? | NO | all revisions H001 Child |
| Multi-route same commit? | YES (batch) | `c9d801c` H001+A001; `d7cac20` not H001; files workspace-scoped |
| Conversation/context isolation evidence | YES | actor `general-5`; Fresh isolation in spawn |
| Code similarity | LOW_TEXT_SIMILARITY vs R031/R31A/R31B | `code-similarity.tsv` H001 rows: line≈0.008–0.009, shingle≈0.01–0.02, shared_special_functions=none |

## Status
**CLEAN_WITH_AVAILABLE_EVIDENCE**

Notes:
- B001 template-shape read is INFRA_REUSE (judge ABI), not COMPUTE_REUSE of R031 family.
- No `POSSIBLE_CONTEXT_LEAK` for H001.
