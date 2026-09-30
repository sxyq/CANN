# MAIN-1 Wave-2 Track-B Handoff Review

- Review scope: handoff completeness, parent, route boundary, duplicate evidence, one-factor scope and proposed minimum probes.
- Canonical head: `ed860e392d7604694ac6664da60aff1fc1f4c04f`.
- Official Champion: `R31B V011 / 45.16`.
- `MAIN_SELECTED=NONE` for all five routes. No implementation or lifecycle decision is made here.

## Route review

| ROUTE | HANDOFF_COMMIT | VALID_HYPOTHESES | REVIEW |
|---|---|---:|---|
| SYNC-TOPOLOGY-CHAMPION-X | `9071292b` | 3 | H1-H3 each move one existing wait or existing vector segment. The route is limited to synchronization order. The handoff compares R31A/R31B, MIX, prior synchronization work and the other Main-1 routes. H1 is adjacent to the R31B V017 drain change; H2 and H3 have different event/work order from STORE and EPI. Actual shape reachability and event coverage remain unknown. Child preference for H2 is not selection. |
| STORE-EPILOGUE-W2-X | `415a2429` | 3 | H1 moves one Store issue point; H3 changes the final chunk boundary; H4 changes only the existing eligibility predicate. H2 is marked infeasible and H5 duplicate-rejected, with an exact prior commit and a matching SYNC H1 location cited. H4 depends on a reachable multi-row t=3 shape. |
| EPI-ARITH-CHAMPION-W2-X | `0ce5441e` | 1 | H1, H2, H4 and H5 are explicitly duplicate-rejected with evidence. H3 groups independent row arithmetic while preserving per-element operation order. It may touch barriers also considered by SYNC, so ownership remains unresolved. `ROUTE_HYPOTHESIS_POOL_EXHAUSTED=YES` is recorded for Planning; no route lifecycle action follows from it. |
| SELECTIVE-FASTPATH-CHAMPION-X | `4d8e073e` | 3 | Each candidate dispatches one frozen donor with V011 fallback and records donor/fallback source identity and coverage limits. R31A V028 is excluded because its parent path differs and there is no direct comparable V011 evidence. Candidate probes must compare the entire donor with V011; prior donor local results are not transferred. |
| SMALLMID-DATAFLOW-CHAMPION-X | `7f51d250` | 3 | H2, H5 and H6 remain distinct; the padded tiny-D batch proposal is explicitly duplicate-rejected against H001/MID-X. The route forbids changing row-to-core assignment and limits work to D<=4096. H2 is adjacent to UB lifetime work, H5 may overlap interpass/crossrow scheduling, and H6 is adjacent to parameter residency; all require Main/Planning boundary resolution before any later selection. |

## Common findings

- Each handoff is committed on its own route branch; the worktree is clean and local HEAD equals its remote branch.
- Direct parents match the approved map: SYNC/EPI/FASTPATH/SMALLMID use R31B V011; STORE uses STORE-EPILOGUE-X V002.
- Cross-route candidate overlap is identified where present. Shape, dtype and Official testcase mapping are unavailable for some proposals and remain open questions.
- Every route remains `MAIN_SELECTED=NONE`. The child recommendations are review input only.
- `REVISION_CREATED=0`, `KERNEL_FILES_CHANGED=0`, `SERVER_RUNS=0`, `ONLINE_SUBMISSIONS=0`, `CANONICAL_SHARED_LEDGER_CHANGES=0`.

## Planning questions

1. EPI H3 may share `PIPE_V` barrier ownership with SYNC; resolve route ownership before considering it.
2. SMALLMID H2 and H6 are adjacent to UB lifetime and parameter residency routes; confirm their boundaries before later selection.
3. SMALLMID H5 overlaps same-core batch timing with INTERPASS/CROSSROW scope; determine which route owns any future overlap study.
4. FASTPATH donor shape coverage and Official case mapping remain incomplete; no broad benefit can be inferred from donor Local data.
5. EPI has one distinct candidate after duplicate rejection; Planning should decide whether this pool is sufficient for further route research. Main makes no lifecycle decision.

No build, correctness run, performance run, profiling, server3 job or Online submission was performed for this review.
