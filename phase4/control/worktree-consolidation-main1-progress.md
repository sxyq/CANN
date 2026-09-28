# Worktree Consolidation MAIN-1 Progress

Scope: MIX-A and WIDE-X-FRESH4 only.

## Sources

| Worktree | Branch | HEAD | Dirty at start |
|---|---|---|---|
| cann-sixlane/MIX-A | exec/sixlane-20260923-mix-a | b902f814 | 0 |
| cann-sixlane/WIDE-X-FRESH4 | exec/sixlane-20260923-wide-x-fresh4 | 9ffabbdb | 0 |

Both source worktrees were clean, so no route-branch commit was required.

## MIX-A

Canonical `phase4/local/MIX-A/V007/` already contained the five-piece minimum and 215 additional result artifacts. Source-only files (23) were checked against HEAD:

- 19 files (MAIN-REVIEW.md, handoff.md, build-unified.log, runner_unified.asc, runner_validation.h, runner_validation_test.cpp, load-window sessions 03-05, D7-R2 paired/same result TSVs) were already tracked in HEAD with identical SHA256. They were missing from the working tree only; restored byte-identical, no content change.
- 4 files (`*-output.bin`) are gitignored repo-wide (`.gitignore:28:*.bin`; 0 tracked `.bin` in the entire repo). Copied to disk with SHA256 verified, not added to git.

`source-meta.json` was incomplete in canonical (missing CURRENT_PROTOCOL_20260927 block, load sessions 03-05, and the D7-R2 LOCAL_DELTA text). Filled from source (commit be376770).

`local-result.json` differs between source and canonical: canonical working tree carries later attempts (D5-R12, D2-PERF) not present in the sixlane worktree. Left untouched per no-overwrite rule.

`submission.sha256` format differs (source includes filename suffix) but the recorded hash matches `submission.asc`; left untouched.

Five-piece in V007: diff.patch, local-result.json, source-meta.json, submission.asc, submission.sha256 — all present. Parent MIX-A-V003 SHA `1a1857a9...` verified.

## WIDE-X-FRESH4

Canonical had only `CURRENT/`. Recovered two complete revision packages plus workspace assets:

### BUILD-FIX-001 (32 files)
Five-piece present (diff.patch, local-result.json, source-meta.json, submission.asc, submission.sha256). Parent CURRENT SHA `bc4608dc...` verified against `CURRENT/submission.asc`. Includes parent-control/ subpackage and logs.

### V001 (44 files)
Five-piece present. Parent BUILD-FIX-001 SHA `5d0ee011...` verified against `BUILD-FIX-001/submission.asc`. Candidate SHA `f7628795...` verified. Includes HANDOFF.md, MAIN-REVIEW.md, NPU correctness logs, server3 build attempts, runner revalidation (18/18).

### Workspaces
Added `npu_correctness.cpp` and `source-meta.json` (SHA256 verified). `wide_x_fresh4.asc` exists in both but differs (source has V001 tile 4096; canonical has prior tile 2048). Not overwritten: V001 bytes are already preserved at `phase4/local/WIDE-X-FRESH4/V001/submission.asc`.

`online/WIDE-X-FRESH4/` does not exist in either tree.

## Ledger

`phase4/control/main1-revision-ledger.tsv` already has complete rows for MIX-A V001–V007 and WIDE-X-FRESH4 CURRENT / BUILD-FIX-001 / V001. No append needed.

## Commits

| SHA | Message |
|---|---|
| be376770 | consolidate MIX-A V007: fill source-meta with D7-R2 protocol block and load sessions 03-05 |
| 29273db5 | consolidate WIDE-X-FRESH4 BUILD-FIX-001: build-fix evidence, parent-control, logs, five-piece |
| b418a831 | consolidate WIDE-X-FRESH4 V001: wide-path tile 2048->4096 revision, NPU correctness, runner revalidation, five-piece |
| 98449d21 | consolidate WIDE-X-FRESH4 workspaces: npu_correctness.cpp and source-meta |

LOCAL_HEAD == REMOTE_HEAD after each push.

## Unresolved

- MIX-A V007 `local-result.json` is divergent (canonical has later D5-R12 / D2-PERF attempts absent from source). Not overwritten. The D7-R2 raw evidence files are present in canonical.
- 4 MIX-A `*-output.bin` files are on disk but untracked (repo gitignore policy).
- `phase4/workspaces/WIDE-X-FRESH4/wide_x_fresh4.asc` differs from source (tile 2048 vs 4096); canonical version retained.

No PARENT_UNRESOLVED: both routes have verified parent SHAs.
