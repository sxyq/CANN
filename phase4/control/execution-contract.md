# Phase4 Execution Contract

This document is the persistent authority for route execution, ownership, provenance, source retention, Git workflow, local screening, and online submission. Before any Route Agent acts, read in order:

1. `phase4/control/project-experiment-playbook.md`
2. `phase4/control/execution-contract.md`
3. `phase4/control/local-timing-protocol.md`

## A. Repository and Worktree

Canonical integration point:

`/Users/sunyiyang/Desktop/Project/cann`

Each Route has exactly one Agent, one isolated mutable worktree, one branch, and one context. Multiple Agents must not share a mutable Candidate worktree. A Child must not modify source belonging to another Route.

## B. Main and Child Authority

Main owns scheduler updates, review, diff audit, Git integration, provenance, local decisions, the online-candidate gate, Judge coordination, and `PROMOTE` / `REJECT` / `PARK` decisions. Main must not write Candidate `.asc`, `.cpp`, `.h`, `.hpp`, Candidate CMake, Kernel source, runners, or host wrappers. Kernel changes belong to the corresponding Route Agent.

A Child may implement its Route, perform build or correctness fixes, compile and link, run targeted NPU correctness, run local measurements, and propose a next hypothesis for the same Route. A Child must not change the lineup, choose a replacement, modify another Route, run CANNJudge, or edit shared scheduler state.

## C. Revision Declaration

Before changing code for every performance revision, record:

```text
ROUTE
REVISION
DIRECT_PARENT
PARENT_SOURCE_SHA
PARENT_SCORE
SINGLE_HYPOTHESIS
CONTEXT_CLASS
WHY_NOT_DUPLICATE
```

For a historical mix, also record `WHY_THIS_COMBINATION_IS_NEW`.

## D. OFAT

One revision may contain one conceptual performance change. Multiple edits are valid only when they jointly implement that one hypothesis. Two independent performance mechanisms make `SINGLE_CHANGE_AUDIT=FAIL`; that revision is not eligible for formal Online submission.

## E. Build and Correctness Fixes

Build or correctness fixes may remain within the same hypothesis. They are limited to compile, ABI, API usage, alignment, synchronization, correctness, and build compatibility. A fix must not introduce a new performance mechanism.

## F. Local-First Flow

```text
Current Validated Local Best (or eligible route seed)
        -> Single Hypothesis
        -> Agent Code
        -> Compile
        -> Link
        -> Targeted NPU Correctness
        -> Local Measurement
        -> Child Handoff
        -> Main Review
```

Child conclusions are limited to:

```text
LOCAL_ACCEPTED
LOCAL_REJECTED
NEEDS_ONE_MORE_LOCAL
MEASUREMENT_BLOCKED
CORRECTNESS_FAILED
BUILD_FAILED
INVALID_SOURCE_IDENTITY
```

Main records `NOT_COMPLETE` if a performance revision has no local performance verdict. The only permitted exception is `MEASUREMENT_BLOCKED`, with `BLOCK_REASON`, `DATE`, `SHAPE`, `DEVICE`, `SAME_BINARY_RESULT`, and `RETRY_REQUIRED`. `PENDING` alone is not a verdict. Main separately decides whether a locally accepted revision is `ONLINE_WORTHY` or `KEEP_ACCUMULATING` and records the reason.

## G. Main Review Gate

Main reviews the Direct Parent, parent source SHA, Candidate SHA, diff, single-hypothesis scope, correctness, executable identity, local comparability, load quality, provenance, and duplicate status. Only `SINGLE_CHANGE_AUDIT=PASS` may proceed to the online-worthiness decision.

## H. Local Timing Authority

The measurement procedure is defined only in `phase4/control/local-timing-protocol.md`. MAIN-1 and MAIN-2 use that same authority. This contract does not duplicate the timing procedure.

Legacy records may be labelled `LEGACY_TIMING_METHOD` or `LOAD_CONTAMINATED`. Legacy raw samples must not be mixed with samples produced by the current protocol.

## I. Local Source Retention

Any revision with compile evidence, correctness evidence, local measurement, or Main Review receives a permanent revision directory and its version number must not be overwritten.

Local-only records live under:

`phase4/local/<ROUTE>/<REVISION>/`

At minimum, retain:

```text
submission.asc
submission.sha256
source-meta.json
diff.patch
local-result.json
```

Retain build logs, correctness logs, raw timing, handoff notes, and `MAIN-REVIEW.md` when present. Failed, rejected, and inconclusive Candidates remain retained.

## J. Online Source Retention

Every formal Online submission lives under:

`phase4/online/<ROUTE>/<REVISION>/`

At minimum, retain:

```text
submission.asc
submission.sha256
result.json
source-meta.json
diff.patch
```

`source-meta.json` includes at least:

```text
route
revision
direct_parent
parent_source_sha
parent_score
single_hypothesis
context_class
source_commit
source_path
submission_id
local_sha256
remote_sha256
official_score
decision
```

## K. Submission Identity

Formal Online eligibility requires:

```text
LOCAL_SHA == SIDECAR_SHA == REMOTE_SHA
```

Any mismatch is `INPUT_IDENTITY_MISMATCH` and `INVALID`; set `formalResultEligible=false` and exclude the result from formal scoring.

## L. Online Authority

Children must not submit to Online. The only formal command is:

```bash
npm run cannjudge:submit -- --yes --source <exact-file>
```

Clipboard, paste, and stdin submission are prohibited. In a multi-Main environment there is one unified Judge Owner. Other Mains freeze the exact source, record it in `online-candidate-pool.tsv`, and hand it to the Judge Owner; they must not submit the same Candidate independently.

## M. Promote and Reject

A Local Revision is measured against its `DIRECT_PARENT`. A new independent performance Revision may use `CURRENT_VALIDATED_LOCAL_BEST` as its parent only after that version has source-identity PASS, compile/link PASS, correctness PASS, same-binary PASS, valid paired local measurements, and Main verdict `LOCAL_ACCEPTED`. `LOCAL_REJECTED` returns the next independent Revision to the preceding Local Best. `NEEDS_ONE_MORE_LOCAL` or `MEASUREMENT_BLOCKED` keeps the Candidate unchanged until a Main disposition.

For Online, compare the submitted exact Candidate with its recorded `OFFICIAL_ANCHOR`, which is the latest Official Best accepted by `PROMOTE`. `PROMOTE` requires correctness PASS and Official Score greater than that anchor. A failed Official comparison does not erase a valid Local Best; it only leaves `OFFICIAL_BEST` unchanged.

## N. Route Ownership

The same Agent continues all revisions for a Route. For example:

```text
V001 -> V002 -> V003
```

Only an unrecoverable service failure permits a replacement worker. The replacement inherits the same Route, worktree, branch, revision, hypothesis, and evidence; this is not a new Route.

## O. Park and Replacement

An Explore Route normally receives two or three architecture-level experiments. If it produces no correctness breakthrough, meaningful local signal, useful new information, or Online gain over that budget, Main may `PARK` it.

The sequence is:

```text
retain exact source and results
        -> retain handoff
        -> PARK
        -> release Explore slot
        -> Main selects replacement
        -> create new Route, Agent, worktree, branch, and context
        -> start V001
```

The old Agent must not move directly to an unrelated Route. A Child must not choose a replacement.

## P. Git

Each Route uses its own branch. Route source and evidence are committed and pushed to that Route branch. After a push, verify local HEAD equals the remote Route branch HEAD. Never force-push, blindly reset, clean, or overwrite a dirty Candidate.

Before changing shared control, reread the latest HEAD and file contents. Make only the smallest line-level change to a Route owned by this Main. Do not regenerate a shared control file.

## Q. Multi-Main Control

The following are shared multi-Main state:

```text
phase4/control/scheduler.tsv
phase4/control/online-candidate-pool.tsv
phase4/control/local-online-calibration.tsv
phase4/control/server3-device-leases.tsv
```

When another Main changes a Route not owned by this Main, accept the newest state and continue. When another Main changes ownership of a Route owned by this Main, record `OWNERSHIP_CONFLICT` and pause only the affected Route. Updates to other Route status or device fields must not be overwritten. Do not stop unrelated Routes because another Route changed.

## R. Long-Horizon Parallel Exploration

Route Agents run continuous two-track work instead of one-probe-then-idle cycles.

```text
TRACK-A: CURRENT EXPERIMENT — keep the current Candidate unchanged
TRACK-B: NEXT-HYPOTHESIS RESEARCH — read-only, long-horizon
```

TRACK-A rules: no Candidate kernel edits, no new donors, no next performance Revision unless Main has issued `REJECT` / `PROMOTE` / `NEXT_HYPOTHESIS`. A measurement-blocked or judge-waiting Candidate does not put the Agent into idle.

TRACK-B rules: historical evidence, own-route code and logs, Ascend C API, public web research, bottleneck analysis, feasibility analysis, design drafts, pseudo-code, experiment matrices, duplicate detection. No kernel modification and no formal next Revision.

Each exploration cycle targets 3–5 genuinely different hypotheses. Every hypothesis records: MECHANISM, BOTTLENECK, EXPECTED_SHAPES, WHY_IT_MAY_HELP, WHY_IT_MAY_FAIL, ASCEND_FEASIBILITY, UB/CORE/DMA_IMPACT, SYNC_IMPACT, PRECISION_RISK, DUPLICATE_CHECK, MINIMAL_OFAT_DIFF, EXPECTED_LOCAL_PROBES. Classification is by readiness (`READY_FOR_MAIN_REVIEW` / `NEEDS_MORE_EVIDENCE` / `DUPLICATE` / `INFEASIBLE`), never by score.

Six Route Agents research in parallel; Main reviews only at batch handoffs. Per-Route research output is appended to `phase4/research/<ROUTE>/next-hypotheses.md`. An Agent returns to Main only after: (A) ≥3 screened hypotheses, (B) a complete local handoff for the current Candidate, (C) a correctness/build blocker needing Main decision, (D) substantive duplication with another active Route, or (E) architecture search budget reached with a PARK recommendation.

New Revisions require the full Research Track plus Main Review plus one explicit architecture hypothesis. No rapid micro-revision loops (V001→V002→V003 one small edit at a time) exist to keep an Agent busy. Research Track never blocks Track-A: when a device window opens or the timing protocol passes the exact shape, the Agent immediately runs same-binary → P/C → Main Review and keeps the research backlog.

Research does not change scoring authority: local percentages are never Official Score; only the unified Judge Owner submits to CANNJudge.

## S. Per-Revision Local Verdict and Best Chain

Every performance Revision receives exactly one local-verdict state: `LOCAL_ACCEPTED`, `LOCAL_REJECTED`, `NEEDS_ONE_MORE_LOCAL`, `MEASUREMENT_BLOCKED`, `CORRECTNESS_FAILED`, `BUILD_FAILED`, or `INVALID_SOURCE_IDENTITY`. Until a state is evidenced, record `NOT_COMPLETE`; do not use an unqualified `PENDING`. A blocked measurement must include its reason, date, shape, device, same-binary result, and retry requirement.

Maintain three route references independently: `OFFICIAL_BEST`, `LOCAL_BEST`, and `CURRENT_CANDIDATE`. Only `LOCAL_ACCEPTED` advances `LOCAL_BEST`. An accepted chain may accumulate one-variable changes, with every hop retaining its own direct parent and local verdict. A rejected revision cannot parent the next independent performance change.

After a local acceptance, Main records `ONLINE_WORTHY` or `KEEP_ACCUMULATING`. `ONLINE_WORTHY` may follow a single stable breakthrough beyond the shape noise floor, or accumulated accepted revisions whose total gain against the latest `OFFICIAL_ANCHOR` merits Official validation. Consider measurement noise, historical Local/Online calibration, shape coverage, and direction consistency. Do not set a fixed percentage threshold.

Every Official result is compared with its `OFFICIAL_ANCHOR` and appended to `phase4/control/local-online-calibration.tsv`. Record Local delta, Official delta, direction agreement, magnitude difference, shape, dtype, context, load quality, and decision. If local data is absent or invalid, record `LOCAL_DATA_INVALID` and why. Update Local evaluator evidence for every result; do not keep an Official score without its Local/Online comparison record.
