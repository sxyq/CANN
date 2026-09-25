# Phase4 Execution Contract

This document is the persistent authority for route execution, ownership, provenance, source retention, Git workflow, local screening, and online submission. Before any Route Agent acts, read this document and `phase4/control/local-timing-protocol.md`.

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
Best / Valid Parent
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
LOCAL_REJECTED
NEEDS_ONE_MORE_LOCAL
ONLINE_CANDIDATE
```

Main may additionally return `NEXT_HYPOTHESIS`, `MEASUREMENT_BLOCKED`, or `PARK`.

## G. Main Review Gate

Main reviews the Direct Parent, parent source SHA, Candidate SHA, diff, single-hypothesis scope, correctness, executable identity, local comparability, load quality, provenance, and duplicate status. Only `SINGLE_CHANGE_AUDIT=PASS` may proceed as `ONLINE_CANDIDATE`.

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

A Candidate is compared only with its Direct Parent. `PROMOTE` requires correctness PASS and an Official Score greater than the Direct Parent. Otherwise the result is `REJECT` or `INCONCLUSIVE` according to the available evidence.

After a rejected performance revision, a new independent hypothesis returns to the latest Best or Promoted Parent. Do not layer an unrelated optimization on a regression.

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

