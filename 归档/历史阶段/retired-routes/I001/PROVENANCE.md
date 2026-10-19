# I001 Provenance

## Isolation policy (as assigned)
- Role: Fresh Wide-D Specialist.
- Forbidden reads: historical R031, R31A compute, R31B compute.
- Allowed: own workspace, shared contracts, own online results, Main feedback for I001 only.
- V002/V003 may reference B001 **template shape only** for judge ABI.

## Evidence checklist

| Item | Finding | Evidence |
|------|---------|----------|
| Agent allowed to read historical R031? | NO | assignment prompt |
| Did agent read R031/R31A/R31B compute? | NOT OBSERVED | handoffs cite I001 + shared + B001 ABI shape |
| Main modified I001 candidate source? | NO | Main created empty `phase4/workspaces/I001/` directory only |
| Main-authored candidate revision? | NO | V001–V007 I001 Child |
| Multi-route same commit? | YES (batch) | `d7cac20` I001+A001+R31A; files workspace-scoped |
| Conversation/context isolation evidence | YES | actors `general-6` then `general-9` after stream failure; Fresh isolation retained on replacement spawn |
| Code similarity | NOT_AVAILABLE at first audit; later family still Fresh | `code-similarity.tsv` historically `NOT_AVAILABLE:I001`; no R031 special functions in handoffs |

## Status
**CLEAN_WITH_AVAILABLE_EVIDENCE**

Notes:
- Replacement agent after `general-6` `ERR_CONNECTION_CLOSED` received a Fresh isolation brief (no R031).
- B001 template read = INFRA_REUSE only.
- No `POSSIBLE_CONTEXT_LEAK` for I001.
