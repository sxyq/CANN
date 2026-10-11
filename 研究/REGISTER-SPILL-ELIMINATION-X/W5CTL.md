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

For a published historical build whose executable bytes are not present in the
public Git object, `--artifact-receipt <build.json>` binds the recorded
`EXE_SHA` without pretending that the receipt is an executable. The receipt is
route-scoped and its SHA is retained in the local manifest. `record` and
`report` also accept optional `--build-receipt` and `--correctness-receipt`;
they verify the published PASS/RC and correctness cell counts and expose the
results as `BUILD_STATUS` and `CORRECTNESS_STATUS`.

Each backend is run as one subprocess with its real exit code. A missing or
non-executable backend is separately reported as `BACKEND_STATUS=NOT_CONFIGURED`
or a configuration error; it does not prevent a valid route-local manifest from
being a completed local Record. `record` requires
`--source`, `--artifact` (or `--artifact-receipt`), `--raw`, `--manifest`, and one or more `--evidence`
paths. It writes a local JSON manifest containing source/artifact/raw SHA-256
values, evidence paths and SHA-256 values, and the complete Local result. With
no Record Owner backend, the complete manifest is still written with
`RECORD_STATUS=PASS` and `BACKEND_STATUS=NOT_REQUESTED` (or
`NOT_CONFIGURED`); shared publication remains separate and is not claimed.
The same manifest and evidence identity are reused on repeated calls without
changing `recorded_utc`; changed identities fail rather than overwrite. A later
configured backend may finalize a prior local-only manifest once. A terminal
backend `PASS`/`FAILED` result is replayed with its original return code and the
backend is not invoked again. With a configured backend failure, the local
manifest remains valid and `BACKEND_STATUS=FAILED` plus the real RC are retained.
Missing or insufficient Local samples are retained with
their actual `INSUFFICIENT_DATA`/`REJECTED`/`UNCLASSIFIED` state and
`FULL_LOCAL_SCORE=NOT_COMPUTABLE`.
`record` never performs `git add`, `commit`, `push`, or Online submission. Git
history/evidence ownership remains governed by `项目规则/Git工作流程.md` and
the existing evidence roots.

## Identity and local data contract

`build`, `correctness`, and `local` require `--source` and an executable
`--artifact` (or a published `--artifact-receipt` for read-only historical
validation) and compute/verify SHA-256 identity before/after delegation.
Optional expected SHA values are checked; a changed or missing source/artifact
is an identity failure.

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

### Published Host/backend compatibility finding

The R2-owned `public-crossrow-v001` source and support CMake/runner files are
byte-identical to the published R1 commit
`d2412ab5cb2d091679947c65d90d45912541b1a5`. Its existing route-local
`crossrow_public_build_backend.sh` is a valid thin Build adapter for the
published support CMake target; recorded w5ctl evidence has Build RC 0 and
artifact SHA `70fce83a...`. Correctness can invoke the resulting executable
directly when the required flags are passed with `--correctness-arg`; the
recorded explicit 16x6144 FP16 invocation returned RC 0 and
`bit_differences=0`. Local can likewise pass explicit `--local-arg` values,
including `--output <raw-path>`; its historical 45-warmup/21-pair result is
shape-only, `UNCLASSIFIED`, and not a Canonical score. Those Parent/Candidate
identities are W3 CROSSROW V012 and W5-R01 V001, not R31B V011.

The published MAIN2 Canonical host is not directly interchangeable with that
paired Local backend. Its runner ABI takes positional
`device, rows, width, dtype, prefix, warmup, samples, blocks, gap, batch_n`
arguments and emits one-side raw rows (`block, rep, device_us, host_wall_us`)
per invocation. The published benchmark orchestrator pairs Parent/Candidate
and loops the frozen suite/runs; w5ctl Local delegates one backend subprocess
and expects a paired-row raw input. Passing the host's per-side TSV directly
does not parse as a paired measurement (tested: `RAW_ROW_INVALID`,
`FULL_LOCAL_SCORE=NOT_COMPUTABLE`). Calling the full benchmark orchestrator
from R2 is also not a safe direct backend: it is rooted at its canonical
campaign paths and performs its own route/lease/evidence writes. The smallest
valid future connection is an accepted stage interface on that existing
Canonical backend that binds caller source/artifact identity and route-local
output paths, then returns the official four-Core/three-run measurements
index for w5ctl's pinned scorer. No runner or score formula is added here.

## Dedup and checkpoints

`dedup` compares normalized changed lines and explicitly subtracts files given
with `--common`, `--champion-scaffold`, or `--abi-scaffold` (for example shared
Champion or ABI scaffold). It reports
`PATCH_SIMILARITY`, `MECHANISM_OVERLAP`, `PATCH_SIMILARITY_GATE`,
`PATCH_SIMILARITY_GATE_STATUS`, `MECHANISM_OVERLAP_REVIEW_THRESHOLD`,
`MECHANISM_OVERLAP_GATE_STATUS`, `MECHANISM_ORTHOGONALITY`,
`MECHANISM_REVIEW`, and `DEDUP_STATUS`. The hard W5 gate is
`PATCH_SIMILARITY <= 0.60`; a value such as `0.61` is
`BLOCKED_PATCH_SIMILARITY`, never `DISTINCT`. Mechanism overlap is a read-only
review signal for Main: any nonzero token overlap emits
`MECHANISM_REVIEW=MAIN_READ_ONLY_REVIEW_SIGNAL` and does not require Owner
acknowledgement or block an otherwise passing patch-similarity gate. Only zero
token overlap returns `DISTINCT`, and even that reports
`MECHANISM_ORTHOGONALITY=UNPROVEN`; the numeric gate never proves semantic
orthogonality. Missing or empty post-exclusion data is
`SIMILARITY_INSUFFICIENT_DATA`.

`cycle --checkpoint <path>` writes an atomic JSON checkpoint after each stage.
The checkpoint retains source/artifact/raw identity, evidence SHA values, stage
return codes/reasons, the route-local manifest SHA, and the complete Local
result. Stage statuses are preserved as `PASS`, `REJECTED`, `UNCLASSIFIED`, or
`INTEGRATION_INCOMPLETE`; Crossrow data remains
`UNCLASSIFIED_NO_PROMOTION`. Completed terminal stages are skipped on repeated
`cycle`/`resume` calls. Failed or missing stages require explicit `resume`.
Neither command creates a Revision or Runner, or changes Online state.
When `--record-manifest` is supplied without `--stages`, the default stage
order is explicitly `build,correctness,local,record` and the checkpoint emits
`E2E_STATUS=PASS` only when all four stages pass, the Local result is a
complete `route-kernel` 4/4 Canonical score, and the route-local Record contains
the same score and raw identity. Historical-tool-validation scores and
shape-only measurements remain `E2E_STATUS=INCOMPLETE`; score quality and
promotion eligibility do not gate completion. A three-stage
`build,correctness,local` cycle remains a valid control-layer cycle but emits
`E2E_STATUS=INCOMPLETE`; it must not be reported as E2E.

`report --checkpoint <path>` reads the checkpoint without invoking a backend.
`report --manifest <path>` queries an archived local manifest through the same
read-only report path. Both modes verify the current route root/branch and
source, artifact, and raw identities; manifest mode also verifies evidence
SHA-256 values, while checkpoint mode verifies the route-local manifest SHA,
content, and its evidence identities. Reports print Local medians, paired
deltas, quality classification, verdict, raw SHA and promotion eligibility.
Canonical reports also re-run the pinned scorer and
compare its score, quality, scorer/suite identities, measurements SHA, and raw
manifest SHA with the persisted values; checkpoint reports also verify that
the recorded manifest still matches its checkpoint SHA and contents. A
manifest query reports the archived Record status but does not recreate Build
or Correctness stages; its E2E status
is incomplete unless matching Build/Correctness receipts and a complete
route-kernel Canonical score are present. An incomplete checkpoint or manifest
is reported as `REPORT_STATUS=INCOMPLETE`, never as a successful score; a
historical-only or missing full Local suite remains
`FULL_LOCAL_SCORE=NOT_COMPUTABLE`.

`report --route-summary` scans only JSON manifests below the bound
`ROUTE_ROOT`, revalidates each `route-kernel` Canonical score through the pinned
published scorer, and prints latest revision/score/quality plus the highest
reliable Local Best. Shape-only, historical-tool-validation, checkpoint, and
invalid manifests are excluded; no shared scoreboard is written. With no
verified R2 Kernel Revision it prints `NONE`.

For `route-kernel` Canonical scoring, all Parent rows must carry one consistent
non-empty artifact SHA matching the published V011 Parent artifact identity:
`eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71`.
Historical validation invocations should pass
`--canonical-purpose historical-tool-validation` together with
`--expected-parent-artifact-sha` set to that measured public identity; a
historical one-shape input still remains `FULL_LOCAL_SCORE=NOT_COMPUTABLE`.

The pinned public commit also contains the complete published
`MAIN2-CANONICAL-V1/versions/VECTOR-MATH-X__V003` source/receipt/measurement
bundle. R2's route-owned historical validation directory records its candidate
source SHA `f019d099...`, published candidate `EXE_SHA`
`da52970a...`, Parent source SHA `a8c19a19...`, Parent artifact SHA
`eed03216...`, and the published scorer result `60.477249` for C12/C13/C14/C16.
This is `score_scope=historical-tool-validation`: it is not an R2 Kernel
Revision, does not enter `report --route-summary`, and does not become a
promotion or Official score. The published Build/Correctness receipts are
validated as evidence; because executable bytes and a legal R2-selected backend
invocation are not present in this route, this receipt must not be called a
fresh w5ctl Build/Correctness/Local E2E run.

This Vector-Math bundle is not W5-R01 CROSSROW V001. The public CROSSROW
materials currently lack the complete C12/C13/C14/C16, three-run paired raw
bundle together with verified artifact and Canonical scorer metadata, so the
older one-shape `UNCLASSIFIED` CROSSROW record remains incomplete. A separate
`r1-public-build-w5ctl.log` is WIDE-X-FRESH4 and is not R1 CROSSROW evidence.

All execution commands verify the current Git root and expected branch supplied
for that invocation, and every source, artifact, raw, log, checkpoint,
evidence, patch, common-scaffold, backend, and backend-workdir path resolves
inside that bound worktree. Violations fail with
`OUT_OF_SCOPE`/`WORKTREE_ISOLATION` before delegation.

## Self-test

```bash
python3 -B 研究/REGISTER-SPILL-ELIMINATION-X/tests/test_w5ctl.py
```

The test uses temporary fake executables and read-only public scorer fixtures only to verify delegation, return-code
propagation, SHA guards, raw-stat rejection, dedup, manifest identity,
checkpoint/report/resume idempotence;
it does not touch a CANN compiler, NPU, Kernel, CMake, shared scheduler, or
the tooling-runner worktree. The current `14/14` result is not an E2E result:
real Build,
Correctness, Local event provenance, Runner identity, and evidence publication
backends must be supplied and accepted by the Tooling/Integration Owner.
