# A001 Provenance

## Isolation policy (as assigned)
- Role: Fresh Phase4 Control.
- Forbidden reads: historical R031, R31A compute source, R31B compute source, `归档/` R031 implementations.
- Allowed: own workspace, shared contracts, own online results, Main feedback for A001 only.

## Evidence checklist

| Item | Finding | Evidence |
|------|---------|----------|
| Agent allowed to read historical R031? | NO | assignment prompt; scheduler note `no R031/R31A/R31B source access` |
| Did agent read R031/R31A/R31B compute? | NOT OBSERVED | agent handoffs never cited R031 mode names or special function names |
| Main modified A001 candidate source? | NO for candidate compute | Main only wrote control TSVs, online result.json, commits of agent-produced files |
| Main-authored candidate revision? | NO | V001–V018 attributed to A001 Route Agent(s) |
| Multi-route same commit? | YES (mixed commits exist) | e.g. `c9d801c` (A001+H001), `d7cac20` (A001+R31A+I001) — Main batch commit for rate-limit efficiency; files remain workspace-scoped |
| Conversation/context isolation evidence | YES | separate actor `general-3`; isolation rules in every spawn; no R031 payload in A001 prompts |
| Code similarity | LOW_TEXT_SIMILARITY vs R031/R31A/R31B | `phase4/control/code-similarity.tsv` lines R031↔A001 / R31A↔A001 / R31B↔A001: line≈0.002, shingle≈0.003, shared_special_functions=none |

## Status
**CLEAN_WITH_AVAILABLE_EVIDENCE**

Notes:
- Multi-route commits are an operational batching artifact, not compute contamination. Diff scope stayed inside `phase4/workspaces/A001/`.
- No `POSSIBLE_CONTEXT_LEAK` flags recorded for A001.
