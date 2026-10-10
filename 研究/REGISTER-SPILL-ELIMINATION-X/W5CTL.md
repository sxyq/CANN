# W5-R02 route-owned `w5ctl`

`w5ctl` is an orchestration layer only. It never compiles source, launches an
NPU runner, calculates a performance score, creates a Revision, commits Git,
or submits Online. Build, Correctness, Local, and evidence recording are
explicit delegated executables.

## Entry point

```text
研究/REGISTER-SPILL-ELIMINATION-X/w5ctl
```

The executable accepts `doctor`, `build`, `correctness`, `local`, `record`,
`dedup`, `cycle`, `resume`, and `report`.

Every invocation must bind its caller explicitly with `--expected-branch` and
`--route` (or `W5CTL_EXPECTED_BRANCH` and `W5CTL_ROUTE`). `--repo-root`
(`W5CTL_REPO_ROOT`) defaults to the current Git worktree root, never to an
embedded route path. `--route-root` (`W5CTL_ROUTE_ROOT`) optionally narrows
the changed-file audit and must remain inside that worktree. All source,
artifact, raw, log, checkpoint, evidence, patch, scaffold, backend, and
backend-workdir paths are still rejected when they resolve outside the bound
worktree. Running from a different worktree or branch is blocked.

## Existing backends

The repository audit found these existing backend scripts. They are examples
of valid caller-supplied backends; `w5ctl` does not copy or select them by
default, and this route does not run them as part of its self-test.

```text
Build:       本地实验/REDUCE-INVSCALE-X/V002/support/compile.sh
Build/link:  本地实验/EXT-ASCEND-X/V001/build_server3.sh
Correctness: 本地实验/REDUCE-INVSCALE-X/V002/support/run_correctness.sh
Local:       本地实验/REDUCE-INVSCALE-X/V002/support/run_probes.sh
```

Point the existing runner at the command line or environment:

```text
W5CTL_BUILD_BACKEND
W5CTL_CORRECTNESS_BACKEND
W5CTL_LOCAL_BACKEND
W5CTL_RECORD_BACKEND
```

The stage-specific command form is also supported: `build --backend <path>`,
`correctness --backend <path>`, and `local --backend <path>`. The verified
historical backend examples above are caller-selected only; `w5ctl` does not
discover, copy, or replace them. A selected backend must be executable and
inside the bound worktree, and its real return code is propagated. The backend
receives `W5CTL_ROUTE`, `W5CTL_STAGE`, source/artifact paths and SHA-256 values,
and for Local the raw timing path.

Each backend is run as one subprocess with its real exit code. A missing or
non-executable backend is `NOT_RUN`/`BLOCKED`, never `PASS`. `record` accepts
an explicit evidence backend, passes source/artifact SHA-256 identity alongside
evidence paths, and never performs `git add`, `commit`, `push`,
or Online submission. Git history/evidence ownership remains governed by
`项目规则/Git工作流程.md` and the existing evidence roots.

## Identity and local data contract

`build`, `correctness`, and `local` require `--source` and `--artifact` and
compute SHA-256 before/after delegation. Optional expected SHA values are
checked; a changed or missing source/artifact is an identity failure.

Local raw input is TSV/CSV or JSON. TSV/CSV fields are:

```text
pair  variant  latency_us  quality  contaminated
```

`variant` must identify `parent` or `candidate`. A structured contamination
flag or non-clean quality rejects the measurement. With enough clean paired
samples, noise is rejected when either side exceeds the default CV threshold
of `0.20`; use `--max-cv` only when the measurement policy explicitly allows
another threshold.

Every local statistics result emits these fields, including rejected input:

```text
MEDIAN_PARENT_US
MEDIAN_CANDIDATE_US
MEDIAN_RATIO_DELTA
PAIRED_MEDIAN_DELTA
PAIRED_MEDIAN_PERCENT_DELTA
RAW_SAMPLE_COUNT
MEASUREMENT_QUALITY
NOISE_STATUS
```

`MEDIAN_RATIO_DELTA` is `(candidate_median / parent_median) - 1` and
`PAIRED_MEDIAN_DELTA` is candidate minus parent in microseconds.
`PAIRED_MEDIAN_PERCENT_DELTA` is the median of per-pair
`(candidate - parent) / parent` values. Parent latency must be positive. Every
pair must contain exactly one parent and one candidate row with an explicit
clean quality value; missing quality or unpaired rows are rejected. `--stats-only`
is for analyzing an already-produced raw file; it does not run a backend.

## Dedup and checkpoints

`dedup` compares normalized changed lines and explicitly subtracts files given
with `--common`, `--champion-scaffold`, or `--abi-scaffold` (for example shared
Champion or ABI scaffold). It reports
`PATCH_SIMILARITY`, `MECHANISM_OVERLAP`, `PATCH_SIMILARITY_GATE`,
`PATCH_SIMILARITY_GATE_STATUS`, `MECHANISM_OVERLAP_GATE`,
`MECHANISM_OVERLAP_GATE_STATUS`, `MECHANISM_ORTHOGONALITY`,
`MECHANISM_REVIEW`, and `DEDUP_STATUS`. The hard W5 gate is
`PATCH_SIMILARITY <= 0.60`; a value such as `0.61` is
`BLOCKED_PATCH_SIMILARITY`, never `DISTINCT`. Mechanism overlap is an
independent gate: overlap at or above the configured threshold blocks, while a
nonzero overlap below it still returns `MANUAL_REVIEW_MECHANISM_OVERLAP` and
requires Owner review. Only zero token overlap returns `DISTINCT`, and even
that reports `MECHANISM_ORTHOGONALITY=UNPROVEN`; the numeric gate never proves
semantic orthogonality. Missing or empty post-exclusion data is
`SIMILARITY_INSUFFICIENT_DATA`.

`cycle --checkpoint <path>` writes an atomic JSON checkpoint after each stage.
Successful stages are skipped on repeat calls. A failed or `NOT_RUN` stage is
only retried by explicit `resume`; `resume` also preserves completed stages.
Neither command creates a Revision or changes Online state.

All execution commands verify the current Git root and expected branch supplied
for that invocation, and every source, artifact, raw, log, checkpoint,
evidence, patch, common-scaffold, backend, and backend-workdir path resolves
inside that bound worktree. Violations fail with
`OUT_OF_SCOPE`/`WORKTREE_ISOLATION` before delegation.

## Self-test

```bash
python3 -B 研究/REGISTER-SPILL-ELIMINATION-X/tests/test_w5ctl.py
```

The test uses temporary fake executables only to verify delegation, return-code
propagation, SHA guards, raw-stat rejection, dedup, and checkpoint idempotence;
it does not touch a CANN compiler, NPU, Kernel, CMake, shared scheduler, or
the tooling-runner worktree. The current `7/7` result is not an E2E result:
real Build,
Correctness, Local event provenance, Runner identity, and evidence publication
backends must be supplied and accepted by the Tooling/Integration Owner.
