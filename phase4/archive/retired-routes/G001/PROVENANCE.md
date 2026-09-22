# G001 Provenance

## Isolation policy (as assigned)
- Role: Fresh One-Read / Resident-Y.
- Forbidden reads: historical R031, R31A compute, R31B compute.
- Allowed: own workspace, shared contracts, own online results, Main feedback for G001 only.
- Special rule: G001 Child owns all source fixes after V001 (Main must not patch).

## Evidence checklist

| Item | Finding | Evidence |
|------|---------|----------|
| Agent allowed to read historical R031? | NO | assignment prompt |
| Did agent read R031/R31A/R31B compute? | NOT OBSERVED | handoffs cite only G001 sources + judge contracts |
| Main modified G001 candidate source? | **YES for V001 integration** | `results.tsv` G001-V001 notes: `Main-authored integration`; `metadata.json` V001: `Main added launch ABI, Vector entry, target scalar sqrt intrinsic` |
| Main-authored candidate revision? | **YES — G001-V001 only** | commit `cd27db1` / results notes; V002+ Child-authored |
| Multi-route same commit? | YES (batch) | e.g. `c9d801c` (G001 not in that one); G001 commits generally route-scoped (`19593fa`, `b20d40b`, `43fc523`, `33deee9`) |
| Conversation/context isolation evidence | PARTIAL | later V002+ actors isolated; V001 workspace already contained Main edits before Child ownership |
| Code similarity | LOW_TEXT_SIMILARITY vs R031/R31A/R31B | `code-similarity.tsv`: line≈0.000, shingle≈0.001, shared_special_functions=none |

## Status
**PARTIALLY_TAINTED**

### Workspace provenance contamination (explicit)
**G001-V001: Main-authored integration, not purely Child-authored.**
- Main added: launch ABI, Vector `run_kernel` entry, target scalar sqrt intrinsic, build wiring.
- Child-owned compute body may have pre-existed, but the online V001 artifact is a Main-integrated revision.
- Recorded in `phase4/control/results.tsv` notes and `phase4/workspaces/G001/metadata.json` (`integration` field).

### Agent context contamination (explicit)
- **Not the same as workspace provenance.** No evidence that G001 agents read R031/R31A/R31B compute sources.
- Agent context isolation for V002+ is **CLEAN_WITH_AVAILABLE_EVIDENCE**.
- V001 agent context is **UNKNOWN** relative to Main’s earlier hands-on integration (Main had already touched the workspace).

### Combined
Workspace provenance for V001 is contaminated by Main-authored integration. Agent context isolation for later Child revisions is clean. Overall route status: **PARTIALLY_TAINTED**.

Do not weaken this record. Future one-read work should use a new G002 Fresh Vectorized Route without inheriting V001 provenance.
