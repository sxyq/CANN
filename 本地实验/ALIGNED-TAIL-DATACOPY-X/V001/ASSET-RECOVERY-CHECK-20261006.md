# ALIGNED-TAIL-DATACOPY-X V001 - Replacement Asset Recovery Check

- Checked at: `2026-10-06T22:08:16Z`
- Worktree: `/home/data4t2/lelinfeng/cann-r-w4-5-aligned-tail`
- Branch / HEAD: `route/r-w4-5-aligned-tail-datacopy-x` / `52b54316a0d55e9cd17ef996f019be454bf9d781`
- Canonical route skill: `origin/main:.agents/skills/cann-route-executor/SKILL.md`
- Canonical skill ref: `origin/main` / `d8c3b380276df5549f057dee19031e04fdd874a9`

## Exact route-owned inventory

The V001 directory contains only:

- `submission.asc` (3554 lines, 190020 bytes)
- `COMPILE-BLOCKER-20261006.md`

The unchanged source SHA256 is:

`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`

No route-owned `CMakeLists.txt`, build/compile script, host/device wrapper,
correctness runner, parent runner, link recipe, or executable exists in this
worktree's `ALIGNED-TAIL-DATACOPY-X` paths.

The reachable route history contains only the blocker commit after the branch
was created from `origin/main`; no exact support asset can be recovered from
that route history. Other route worktrees and other route assets were not read.

## Why an exact compile cannot start

The source includes `kernel_operator.h` and defines `run_kernel` with
runner-provided `TensorGroupInfo` / `TensorInfo` metadata and `aclrtStream`.
It is an include-style direct-invocation source, not a standalone translation
unit. Without its matching wrapper and build/link contract, a guessed compile
would not establish source identity or an attributable executable.

## State and next action

- `BUILD=INCOMPLETE` (`COMPILE=NOT_RUN`; no exact command exists)
- `CORRECTNESS=INCOMPLETE`
- `EXECUTABLE_IDENTITY=MISSING`
- `LOCAL=NOT_COMPLETE`
- `BLOCKER=ROUTE_LOCAL_EXACT_BUILD_AND_RUNNER_ASSETS_MISSING`
- `ONLINE=NOT_RUN`

The bounded resolution is to restore the exact route-owned wrapper/build
assets in this worktree. Once present, compile/link this unchanged V001
source, then run correctness, then device-4 Local with HBM/load/raw evidence.
No speculative runner, shared-record write, or Online action is authorized.
