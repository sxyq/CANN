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

## Official Input-Mapping Follow-up (2026-10-03)

- The read-only public Judge query returned the problem ID, 15 testcase IDs and current public `tbest` values. The submission-detail query for R31B V011 (`6ab2c10c0304f72a56a0c5cb`) returned HTTP 403. The public response contains no per-case input shape, dtype, `availableCoreNum`, dispatch, or core-ownership fields; testcase latency is not used to infer them.
- R31B V011 source behavior is known, but no testcase can be assigned to a branch from the available Judge data. The entry point flattens leading dimensions into `rowCount`, takes the final dimension as `rowWidth`, clamps requested blocks to `rowCount`, and dispatches dtype IDs 0/1/2 to FP32/FP16/BF16. Widths above 8192 use the wide path; narrower inputs select among existing paths using dtype, width and per-core row count.
- The retained C001 source confirms four vector cores cooperate across a row's D dimension. Its Official result records case 1 TLE and cases 2–15 skipped; it supplies no execution evidence for case 14 and no separate TLE diagnostic log. Case14 applicability therefore remains unproven.
- CASE47 case4/case7, CASE14 case14, FASTPATH donor coverage, and TINY case1/3/5 remain unmapped. Keep all four at `MAIN_SELECTED=NONE`; no performance Revision, NPU run, or Online submission follows from this data.
- Evidence references: `线上结果/R31B/V011/result.json`, `线上结果/R31B/V011/submission.asc`, `线上结果/R31B/V011/source-meta.json`, `线上结果/C001/result.json`, and `归档/历史工作区/C001/kernel.txt`.

## SYNC W60 Evidence Retrieval Follow-up (2026-10-03)

- The existing W60 same-binary result remains PASS in the route record, but its raw and jitter files have not been copied from server3. The assigned read-only retrieval attempt and a separate read-only SSH attempt both timed out; no remote query or transfer ran.
- Remote evidence remains under `/home/data4t2/lelinfeng/cann/w2/SYNC-TOPOLOGY-CHAMPION-X/V001/results/SYNC-TOPOLOGY-CHAMPION-X-V001-D4-WARMUP60-20261003T020523Z/`. Required files are `SAME-BINARY-raw.tsv` and `SAME-BINARY-jitter.txt`; local SHA values are not available yet.
- Parent window and Candidate P/C remain NOT_RUN. The earlier lease is RELEASED. After SSH access returns, retrieve the existing files first, then perform a live device read and record a new lease before any timing stage. No Candidate source or timing harness changed in this follow-up.

## CASE47 Active-Core Follow-up (2026-10-03)

- Route report commit: `d2af2ff29fddf9bb1384f38341a8dcba2b83bd99`. The case4/case7 input table still has UNKNOWN shape, dtype, M, D, runtime `availableCoreNum`, dispatch and ownership; Support-A's public data did not supply those fields.
- Main review: H1's D-split is not implementable through the current call interface; H2–H4 duplicate existing R016/SCHED row-granularity, row-group, core-scaling or ownership work; H5 remains conditional on dispatch reachability. These classifications do not select an implementation.
- Keep `MAIN_SELECTED=NONE`. No performance Revision, Build, correctness, device timing, or lifecycle decision follows. Exact testcase input metadata and a source-bound runtime core-count record remain missing.

## SELECTIVE-FASTPATH Donor Follow-up (2026-10-03)

- Route audit commit: `f8e28d86f268e07cc49de558fe7e51aa9a70c8d7`. STORE V003 source, its STORE V002 parent, the R31B V011 fallback, sidecar and saved Judge identity were matched in the route audit.
- The audited mechanism is wide-FP32 output writeback split into two contiguous chunks, with the first chunk issued ahead. Recorded Local deltas compare STORE V003 with STORE V002; they do not measure V003 against R31B V011. Full-donor Official score is `44.38`, below V011 `45.16`; the reported case14 latency is about 0.76% slower and does not identify that case's input or dispatch.
- CASE47 H4 tile-width and H2 active-core changes can interact with V003's tile count or block row grouping; TINY H1 can also affect row grouping if the input overlaps. The Official testcase-to-input and runtime path map remains unavailable, so donor reachability and selective-score benefit are unproven.
- Main review remains `NEEDS_MORE_EVIDENCE`; keep `MAIN_SELECTED=NONE`. No performance Revision, Build, correctness, timing, submission, or route lifecycle change is authorized by this audit.
