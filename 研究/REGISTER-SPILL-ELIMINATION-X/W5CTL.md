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

Each backend is run as one subprocess with its real exit code. A missing or
non-executable backend is `NOT_RUN`/`BLOCKED`, never `PASS`. `record` accepts
an explicit evidence backend and never performs `git add`, `commit`, `push`,
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
RAW_SAMPLE_COUNT
MEASUREMENT_QUALITY
NOISE_STATUS
```

`MEDIAN_RATIO_DELTA` is `(candidate_median / parent_median) - 1` and
`PAIRED_MEDIAN_DELTA` is candidate minus parent in microseconds. `--stats-only`
is for analyzing an already-produced raw file; it does not run a backend.

## Dedup and checkpoints

`dedup` compares normalized changed lines and explicitly subtracts files given
with `--common`, `--champion-scaffold`, or `--abi-scaffold` (for example shared
Champion or ABI scaffold). It reports
`PATCH_SIMILARITY`, `MECHANISM_OVERLAP`, and `DEDUP_STATUS`. Missing or empty
post-exclusion data is `SIMILARITY_INSUFFICIENT_DATA`.

`cycle --checkpoint <path>` writes an atomic JSON checkpoint after each stage.
Successful stages are skipped on repeat calls. A failed or `NOT_RUN` stage is
only retried by explicit `resume`; `resume` also preserves completed stages.
Neither command creates a Revision or changes Online state.

## Self-test

```bash
python3 -B 研究/REGISTER-SPILL-ELIMINATION-X/tests/test_w5ctl.py
```

The test uses temporary fake executables only to verify delegation, return-code
propagation, SHA guards, raw-stat rejection, dedup, and checkpoint idempotence;
it does not touch a CANN compiler, NPU, Kernel, CMake, shared scheduler, or
the tooling-runner worktree.
