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
discover, copy, or replace them. A selected backend must be executable, or an
explicit `.sh`/`.bash` file delegated through the system `/bin/bash`, and must
be inside the bound worktree. Its real return code is propagated. The backend
receives `W5CTL_ROUTE`, `W5CTL_STAGE`, source/artifact paths and SHA-256 values,
and for Local the raw timing path.

Each backend is run as one subprocess with its real exit code. A missing or
non-executable backend is `NOT_RUN`/`BLOCKED`, never `PASS`. `record` requires
`--source`, `--artifact`, `--raw`, `--manifest`, and one or more `--evidence`
paths. It writes a local JSON manifest containing source/artifact/raw SHA-256
values, evidence paths and SHA-256 values, and the complete Local result. With
no Record Owner backend, the complete manifest is still written and the
command returns `NOT_RUN`/78 with
`INTEGRATION_STATUS=INTEGRATION_INCOMPLETE`; it never claims Record PASS.
The same manifest and evidence identity are reused on repeated calls without
changing `recorded_utc`; changed identities fail rather than overwrite. A later
configured backend may finalize a prior local-only manifest once. A terminal
backend `PASS`/`FAILED` result is replayed with its original return code and the
backend is not invoked again. With a valid first-time backend, its real return
code is propagated. Missing or insufficient Local samples are retained with
their actual `INSUFFICIENT_DATA`/`REJECTED`/`UNCLASSIFIED` state and
`FULL_LOCAL_SCORE=NOT_COMPUTABLE`.
`record` never performs `git add`, `commit`, `push`, or Online submission. Git
history/evidence ownership remains governed by `项目规则/Git工作流程.md` and
the existing evidence roots.

## Identity and local data contract

`build`, `correctness`, and `local` require `--source` and `--artifact` and
compute SHA-256 before/after delegation. Optional expected SHA values are
checked; a changed or missing source/artifact is an identity failure.

Local raw input is TSV/CSV or JSON. Generic row-oriented TSV/CSV fields are:

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

The published CROSSROW paired runner's TSV is also recognized without editing
the raw file: its leading `# LOCAL_PROXY_ONLY` metadata line, followed by
`sample/order/parent_device_us/candidate_device_us/delta_device_us/...`, is
parsed as one paired row per sample (`order` must be `PC` or `CP`). `w5ctl`
computes the same medians and paired deltas from those device-event values, but
that schema has no per-sample quality/contamination provenance. It therefore
reports `MEASUREMENT_QUALITY=UNCLASSIFIED`,
`QUALITY_CLASSIFICATION=UNCLASSIFIED`, `PROMOTION_ELIGIBLE=NO`, and a nonzero
return code even when numeric noise checks are stable. This is a statistics
and parser integration result, not an accepted or promotable Local result.

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
The checkpoint retains source/artifact/raw identity, evidence SHA values, stage
return codes/reasons, and the complete Local result. Stage statuses are
preserved as `PASS`, `REJECTED`, `UNCLASSIFIED`, or
`INTEGRATION_INCOMPLETE`; Crossrow data remains
`UNCLASSIFIED_NO_PROMOTION`. Completed terminal stages are skipped on repeated
`cycle`/`resume` calls. Failed or missing stages require explicit `resume`.
Neither command creates a Revision or Runner, or changes Online state.

`report --checkpoint <path>` reads the checkpoint without invoking a backend.
`report --manifest <path>` queries an archived local manifest through the same
read-only report path. Both modes verify the current route root/branch and the
recorded source, artifact, raw, and evidence SHA-256 identities before printing
Local medians, paired deltas, quality classification, verdict, raw SHA and
promotion eligibility. A manifest query reports the archived Record status but
does not recreate Build or Correctness stages. An incomplete checkpoint or
manifest is reported as `REPORT_STATUS=INCOMPLETE`, never as a successful
score; a missing full Local suite remains `FULL_LOCAL_SCORE=NOT_COMPUTABLE`.

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
propagation, SHA guards, raw-stat rejection, dedup, manifest identity,
checkpoint/report/resume idempotence;
it does not touch a CANN compiler, NPU, Kernel, CMake, shared scheduler, or
the tooling-runner worktree. The current `9/9` result is not an E2E result:
real Build,
Correctness, Local event provenance, Runner identity, and evidence publication
backends must be supplied and accepted by the Tooling/Integration Owner.
