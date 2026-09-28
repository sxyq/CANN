# Worktree Consolidation MAIN-1 Progress

Scope for this pass: MODE-X-R015C and DTYPE-SPECIAL-X only.

Canonical: `/Users/sunyiyang/Desktop/Project/cann` branch `exp/independent-breadth`

## MODE-X-R015C

Source worktree: `/Users/sunyiyang/Desktop/Project/cann-sixlane/MODE-X-R015C`
Route branch: `exec/sixlane-20260923-mode-x-r015c` (no upstream)
Worktree dirty at start: 0

### Packages recovered

| Package | Files copied | Commit |
|---|---|---|
| `phase4/local/MODE-X-R015C/CURRENT/` | 0 (already identical, 10 files) | — |
| `phase4/local/MODE-X-R015C/R015C-r3/` | 16 | `5108a216` |
| `phase4/local/MODE-X-R015C/R015C-r4/` | 32 | `8ce9e2d7` |
| `phase4/workspaces/MODE-X-R015C/` | 9 added + 3 updated | `2890b5fa` |

R015C-r3 five-piece: diff.patch, local-result.json, source-meta.json, submission.asc, submission.sha256 — all present.
R015C-r4 five-piece: all present. source-meta.json records direct parent R015C-r3 with parent source SHA.

Workspace updates (canonical was older/incomplete; revision sources preserved under `local/`):

| File | Old SHA256 (canonical) | New SHA256 (worktree) |
|---|---|---|
| `op_host/row_copy_host.asc` | `c9939cb1a13c2aa883befdcdb1e13582a42f857fe68263ec5369c229d1c366a4` | `fbd3147221a36756d9fd65656635a4d5c92e8ad8be77405ed75c8de97e8726de` |
| `op_kernel/row_copy_kernel.asc` | `70ad9e17ca13f83f49e9b5dd8e7adad55f4c6f251a599fc4b87c9e935e78f04c` (CURRENT) | `e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903` (r3) |
| `README.md` | `4bc31c74e6ffc9e89ed71e77567b47922ed78b39d598e7e12c2aba899bb685cc` | `24043e444f820e3a7655bf94094e3bca4ae1fe04491607fb76fb03039ccf2102` |

`phase4/online/MODE-X-R015C/` does not exist in either tree.

Ledger rows for CURRENT / r1-gap / r2 / r3 / r4 already present in `main1-revision-ledger.tsv`; no append needed.

## DTYPE-SPECIAL-X

Source worktree: `/Users/sunyiyang/Desktop/Project/cann-sixlane/DTYPE-SPECIAL-X`
Route branch: `exec/sixlane-20260924-dtype-special-x` (no upstream)
Worktree dirty at start: 1 modified + 6 untracked

### Route-branch commit first

`d90a0f83` — committed dirty `local-result.json` and 6 untracked measurement logs on the route branch (no upstream, no push).

### Packages recovered

| Package | Files copied | Commit |
|---|---|---|
| V001 five-piece + docs + support scripts | 12 | `ac0eb066` |
| `V001/logs/` | 141 | `fbf983cb` |
| `V001/support/results/` deltas (R12 + admission) | 7 | `0e5c2511` |
| `phase4/workspaces/DTYPE-SPECIAL-X/V001/` | 12 | `07f05acd` |

V001 five-piece: all present. source-meta.json records direct parent `R31B-V011`, parent source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, parent score 45.16. No PARENT_UNRESOLVED.

Canonical already held 215 additional `support/results/` measurement files not in the worktree (parallel measurement runs); left untouched.

`phase4/online/DTYPE-SPECIAL-X/` does not exist in either tree.

Ledger row for V001 already present; no append needed.

## Verification

- `diff -rq` MODE local and workspaces: 0 missing, 0 differing.
- `diff -rq` DTYPE local: 0 missing from worktree, 0 differing, 215 extra in canonical (retained).
- `diff -rq` DTYPE workspaces: identical.
- SHA256 spot-check on 5 key files: all match worktree bytes.
- DTYPE `submission.sha256` references `submission.asc` hash `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f` — matches.

## R31B

Source worktree: `/Users/sunyiyang/Desktop/Project/cann-sixlane/R31B`
Route branch: `exec/sixlane-20260923-r31b` (no upstream)
Worktree dirty at start: 0
Worktree HEAD: `16249f38`

### Packages recovered

| Package | Files copied | Commit |
|---|---|---|
| `phase4/local/R31B/V016/` (missing + updated) | 20 added + 6 updated | `0c318ba4` |

V016 five-piece: submission.asc, submission.sha256, source-meta.json, diff.patch, local-result.json — all present. submission.asc SHA256 `9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5` preserved.

Parent verification: PROVEN — `phase4/online/R31B/V011/submission.asc` SHA256 matches `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.

source-meta.json fields filled: OFFICIAL_ANCHOR, SOURCE_SHA256, SOURCE_COMMIT, SOURCE_PATH, REMOTE_BRANCH, BUILD_STATUS, CORRECTNESS_STATUS, PARENT_VERIFICATION.

Content updates (canonical was older/incomplete): source-meta.json (missing SOURCE_SHA), LOAD_QUALITY (missing correctness/probe/window snapshots), compile-evidence.txt (missing final artifact SHAs and correctness results), support/runtime_probe.cpp, runtime_probe_fp32.cpp, CMakeLists.probe.txt (worktree has repeats=0 correctness-only mode and wider validation).

NOT updated: `phase4/workspaces/R31B/CMakeLists.txt` — canonical version is newer (includes V015 target absent from worktree).

`phase4/online/R31B/` — fully identical, nothing to copy.

Ledger rows for V001–V016 already present; consolidation note appended to V016 row.

## R31A

Source worktree: `/Users/sunyiyang/Desktop/Project/cann-sixlane/R31A`
Route branch: `exec/sixlane-20260923-r31a` (no upstream)
Worktree dirty at start: 1 modified (`V021/local-result.json`)
Worktree HEAD at start: `f0dc0cbc`

### Route-branch commit first

`c2604bf7` — committed dirty `V021/local-result.json` (D5 R15 paired timing block and verdict update) on the route branch (no upstream, no push).

### Packages recovered

| Package | Files copied | Commit |
|---|---|---|
| `phase4/local/R31A/V020/` + workspaces V020 sources | 12 | `9fac2bc8` |
| `phase4/local/R31A/V021/` + workspaces V021 sources + CMakeLists | 164 | `33f58a95` |

V020 five-piece: submission.asc, source-meta.json, local-result.json present. MISSING in source (not fabricated): submission.sha256, diff.patch. submission.asc SHA256 `3249033942141c24df66f7e97ffa96ffe193f8eec55e992f57fac2948fd70243` preserved.

V021 five-piece: submission.asc, submission.sha256, source-meta.json, diff.patch, local-result.json — all present. submission.asc SHA256 `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063` preserved.

Parent verification: PROVEN for both — `support/parent_v016_submission.asc` SHA256 matches `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` (also matches `workspaces/R31A/R31A-V016-submission.asc`).

source-meta.json fields filled for both: OFFICIAL_ANCHOR, SOURCE_SHA, SOURCE_SHA256, SOURCE_COMMIT, SOURCE_PATH, REMOTE_BRANCH, BUILD_STATUS, CORRECTNESS_STATUS, PARENT_VERIFICATION.

Workspaces: added R31A-V020-device.cpp, R31A-V021-device.cpp, compile_adapter_v020.{asc,cpp}, compile_adapter_v021.{asc,cpp}; updated CMakeLists.txt to V021 target (canonical was older, pointing to V017).

Canonical already held extensive `V021/support/results/` measurement files (D1/D3/D5/D6/D7 runs from 2026-09-27) not in the worktree — left untouched; `git add` on the V021 directory also brought those previously-untracked files under tracking without modifying their bytes.

`phase4/online/R31A/` — fully identical, nothing to copy.

V019 source-meta.json: semantically identical between worktree and canonical (JSON formatting only); canonical left unchanged.

Ledger rows for V001–V021 already present; consolidation notes appended to V020 and V021 rows.

## Notes and residual items (R31B/R31A pass)

- Commit `ac0eb066` accidentally included 19 already-staged `phase4/local/MIX-A/V007/` files from a parallel consolidation agent. Not amended (already pushed; amend rules not met). MIX-A content is valid evidence and is in-scope for a different consolidation pass.
- Parallel agents commit and push on the same canonical branch. Staging must be verified with `git diff --cached` before every commit.
- Canonical working tree still carries dirty/untracked files from other routes — untouched by this pass.
- Remaining dirty in source worktrees after this pass: R31B 0, R31A 0.
- R31B V020 is missing submission.sha256 and diff.patch in the source worktree (never produced); not fabricated.
- R31A V021 local-result.json reflects D5 R15 timing block; canonical also holds later D1/D3/D6/D7 measurement results under support/results/ that postdate the worktree local-result — those are preserved but not yet reflected in local-result.json.

---

# MIX-A and WIDE-X-FRESH4 consolidation pass

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

Canonical had only `CURRENT/`. Recovered two complete revision packages plus workspace assets.

### BUILD-FIX-001 (32 files)

Five-piece present (diff.patch, local-result.json, source-meta.json, submission.asc, submission.sha256). Parent CURRENT SHA `bc4608dc...` verified against `CURRENT/submission.asc`. Includes parent-control/ subpackage and logs. Commit `29273db5`.

### V001 (44 files)

Five-piece present. Parent BUILD-FIX-001 SHA `5d0ee011...` verified against `BUILD-FIX-001/submission.asc`. Candidate SHA `f7628795...` verified. Includes HANDOFF.md, MAIN-REVIEW.md, NPU correctness logs, server3 build attempts, runner revalidation (18/18). Commit `b418a831`.

### Workspaces

Added `npu_correctness.cpp` and `source-meta.json` (SHA256 verified). Commit `98449d21`. `wide_x_fresh4.asc` exists in both but differs (source has V001 tile 4096; canonical has prior tile 2048). Not overwritten: V001 bytes are already preserved at `phase4/local/WIDE-X-FRESH4/V001/submission.asc`.

`phase4/online/WIDE-X-FRESH4/` does not exist in either tree.

## Ledger

`main1-revision-ledger.tsv` already has complete rows for MIX-A V001–V007 and WIDE-X-FRESH4 CURRENT / BUILD-FIX-001 / V001. No append needed.

## Commits (this pass)

| SHA | Message |
|---|---|
| be376770 | consolidate MIX-A V007: fill source-meta with D7-R2 protocol block and load sessions 03-05 |
| 29273db5 | consolidate WIDE-X-FRESH4 BUILD-FIX-001: build-fix evidence, parent-control, logs, five-piece |
| b418a831 | consolidate WIDE-X-FRESH4 V001: wide-path tile 2048->4096 revision, NPU correctness, runner revalidation, five-piece |
| 98449d21 | consolidate WIDE-X-FRESH4 workspaces: npu_correctness.cpp and source-meta |

LOCAL_HEAD == REMOTE_HEAD after each push.

## Unresolved (this pass)

- MIX-A V007 `local-result.json` is divergent (canonical has later D5-R12 / D2-PERF attempts absent from source). Not overwritten. The D7-R2 raw evidence files are present in canonical.
- 4 MIX-A `*-output.bin` files are on disk but untracked (repo gitignore policy).
- `phase4/workspaces/WIDE-X-FRESH4/wide_x_fresh4.asc` differs from source (tile 2048 vs 4096); canonical version retained.

No PARENT_UNRESOLVED: both routes have verified parent SHAs.

Remaining dirty in source worktrees after this pass: MIX-A 0, WIDE-X-FRESH4 0.

## MAIN-1 worktree removal (2026-09-28)

All six cann-sixlane worktrees removed after consolidation audit (worktree-cleanup-audit.tsv):
R31B, R31A, MIX-A, WIDE-X-FRESH4, MODE-X-R015C, DTYPE-SPECIAL-X.
Local route branches and remote route branches KEPT as evidence.
Evidence lives in canonical phase4/local/<ROUTE>/ and phase4/online/<ROUTE>/.
