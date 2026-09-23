# Six-Lane Execution State

This is the current six-slot handoff. It supersedes older route lists in dated review notes; their measured results remain historical evidence. This synchronization turn changes documentation only: no candidate source, server3 run, local experiment, or CANNJudge submission was performed.

## Fixed Slots

| Slot | Lane | Route | Parent | Current route state |
|---:|---|---|---|---|
| 1 | Exploit | R31B | V011, 15/15, 45.16 | ACTIVE; awaiting a new Agent-capable session |
| 2 | Exploit | R31A | V016, 15/15, 45.00 | ACTIVE; awaiting a new Agent-capable session |
| 3 | Explore | MIX-A | V003, 15/15, 44.69 | ACTIVE; isolate one donor/mechanism |
| 4 | Explore | WIDE-X-FRESH4 | Fresh | ACTIVE; repair recorded build errors, then correctness |
| 5 | Explore | MODE-X-R015C | Standalone current candidate | ACTIVE; NPU correctness is next |
| 6 | Explore | EXT-ASCEND-X | None | ACTIVE / NOT_STARTED |

EPI-X-FRESH is PARKED. Its source and local result remain in `phase4/workspaces/EPI-X-FRESH/` and `phase4/local/EPI-X-FRESH/`.

The six route owners, isolated worktrees, branches, and contexts are not created in this documentation-only turn. `scheduler.tsv` records them as `NOT_CREATED`; do not treat old Agent IDs or shared source-seed directories as live ownership.

## Parent Rules

| Route | Permitted parent | Parent SHA256 | Parent result | Disallowed parent |
|---|---|---|---|---|
| R31B | V011 or a later revision explicitly PROMOTED by Main | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | 15/15, 45.16 | V015 is not a valid parent |
| R31A | V016 or a later revision explicitly PROMOTED by Main | `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` | 15/15, 45.00 | V017 scored 44.45 and is REJECTED |
| MIX-A | V003 until a later revision is explicitly PROMOTED by Main | `1a1857a945ce1f04e3877437882b21a3d5071dddcd697103d7b539a0feb5c706` | 15/15, 44.69 | Do not treat V004-V006 local artifacts as promoted parents |

These values are corroborated by retained online `source-meta.json`, `submission.sha256`, and `result.json` records. A newer version number does not change the permitted parent.

## Ownership Loop

There are exactly six fixed Route slots: two Exploit and four Explore. Each Route has one Agent, one isolated worktree, one branch, and one context for its entire lifetime. The same Route Agent continues V001 → V002 → V003 and later revisions. A Route Agent does not transfer to another Route.

Before an Agent starts, record the created Agent identity, worktree path, branch, and context in `scheduler.tsv`. Until a session can create all six isolated owners, each remains `NOT_CREATED`; do not create partial shared-workspace substitutes.

When a Route is PARKED: retain its source and results, release its Explore slot, wait for upper-level Review to select a replacement, then create a new Agent, worktree, branch, and context for the replacement's V001. The previous Agent must not take over the replacement Route.

## Authority

Child Agents may implement the current Route hypothesis, repair that Route's build or correctness, run its server3 local work, and propose its next hypothesis. They may not change the six-route lineup, select replacements, assign work to other Agents, change provenance class, read another isolated Route's source, or submit to CANNJudge.

Main owns scheduling, review, Git, results, Judge submissions, provenance, and route status. Main does not write Kernel or candidate implementation files, including `.asc`, `.cpp`, `.h`, `.hpp`, CMake, runners, or host wrappers.

## Route Instructions

- **R31B:** exploit from V011 or a later PROMOTED revision. V015 is excluded as a parent.
- **R31A:** exploit from V016 or a later PROMOTED revision. Do not continue from V017.
- **MIX-A:** V003 is the route Best. Since V003 combines multiple changes, each new revision isolates exactly one donor or mechanism.
- **WIDE-X-FRESH4:** preserve its Fresh Blind boundary. First resolve the recorded `sqrtf` and `size_t` to `uint32_t` build failures, then complete build/link and NPU correctness. Compile failure alone does not disprove the architecture.
- **MODE-X-R015C:** do not add performance changes first. Run NPU correctness on the current candidate; after PASS, proceed to paired local probes.
- **EXT-ASCEND-X:** `CONTEXT_CLASS=GUIDED_FRESH / EXTERNAL_DERIVED`. V001 tests only whether tiling-driven `rowFactor`/`ubFactor` and per-core row ownership improve on fixed mode thresholds. Each core owns consecutive rows; gamma/bias are loaded once and reused across its row group; UB work size adapts to dtype and D; row batches use natural TQue/TBuf CopyIn, Compute, and CopyOut. Implement from the task ABI. Do not copy external code, add triple pipeline, redesign reduction, add a wide special case, or change numerical precision in V001.
- **EPI-X-FRESH:** PARKED. Keep every existing source and result artifact.

External references, when used by a Route Agent, may contribute architecture, tiling, buffering, scheduling, or reduction ideas only. Do not import their source code.

## Revision and Review Loop

Before each code change, record `ROUTE`, `REVISION`, `DIRECT_PARENT`, `PARENT_SOURCE_SHA`, `PARENT_SCORE`, `SINGLE_HYPOTHESIS`, and `CONTEXT_CLASS`. One revision tests one conceptual change. Correctness repairs do not add a performance variable.

The order is compile/link on server3 → targeted NPU correctness → 2–4 paired local probes → Main review. After local results, the Agent stops for Main's decision: `LOCAL_REJECTED`, `NEEDS_ONE_MORE_LOCAL`, `ONLINE_CANDIDATE`, or `NEXT_HYPOTHESIS`. An Agent must not start an online submission or another revision before that decision.

Compare a candidate only with its Direct Parent. Main may mark it PROMOTED only when correctness passes and the official score exceeds that parent's score. Otherwise mark it REJECTED or INCONCLUSIVE. A rejected performance revision is not the parent for another performance change; return to the latest permitted parent.

## Online Records and Retention

The only formal submission command is:

```bash
npm run cannjudge:submit -- --yes --source <exact-file>
```

Clipboard, paste, and stdin submission are prohibited. Require `LOCAL_SHA == SIDECAR_SHA == REMOTE_SHA`; any mismatch makes the submission INVALID and excludes it from formal results.

Retain every formal online revision under `phase4/online/<ROUTE>/<REVISION>/` with `submission.asc`, `submission.sha256`, `result.json`, `source-meta.json`, and `diff.patch`. Record route, revision, direct parent and its source SHA/score, single hypothesis, context class, source commit/path, submission ID, local and remote SHA256, official score, and decision in `source-meta.json`.

Retain every local-only revision under `phase4/local/<ROUTE>/<REVISION>/` with `submission.asc`, `submission.sha256`, `local-result.json`, `source-meta.json`, and `diff.patch`. Keep required support files and their SHA256 values beside multi-file candidates. Rejected and inconclusive artifacts remain available; do not delete or overwrite prior revisions.

## Replacement Queue

Only upper-level Review selects a replacement after a Route is PARKED. Current candidates, in order, are ASYNC-TRIPLE-X, CORE-SCHED-X, UB-LAYOUT-X, DTYPE-SPECIAL-X, PARAM-REUSE-X, and NEW-EXTERNAL-DERIVED-X. No replacement selection is made by a Child Agent.
