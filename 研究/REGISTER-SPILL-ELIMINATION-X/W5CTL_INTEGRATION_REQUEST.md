# W5CTL Tooling/Integration Publication Request

REQUEST_ID=W5-R02-W5CTL-PUBLISH-001
REQUEST_TIME_UTC=2026-10-10
REQUESTER=W5-R02-REGISTER-SPILL-ELIMINATION-X
SOURCE_BRANCH=research/w5-r02-register-spill
PUBLISH_BASELINE_COMMIT=c24cdb7ef2823e795d0b2cb9faea0ffeae6e40ad
PUBLISH_BASELINE_STATUS=PUBLISHED_AND_VERIFIED_ON_ROUTE_BRANCH

## Request

To the existing canonical Tooling/Integration Owner: please review and publish
the route-owned `w5ctl` from the exact baseline commit above through the
existing shared tooling/integration path. Do not copy it into a second runner,
scorer, or execution chain. Keep the route-owned implementation unchanged
until the Owner supplies the canonical publication receipt.

The proposed entrypoint is:

```text
研究/REGISTER-SPILL-ELIMINATION-X/w5ctl
```

The Owner should decide the shared canonical location and record the resulting
publication commit or receipt. This route does not modify a shared entrypoint.

## Interface Contract

The published wrapper must expose these subcommands and preserve their real
status/return-code behavior:

- `doctor [--dry-run]`: read-only root, branch, Champion SHA, scope, and backend
  readiness audit.
- `build`: delegate to the caller-supplied existing build backend; verify source
  and artifact identity before and after execution.
- `correctness`: delegate to the caller-supplied existing correctness backend;
  propagate its real return code and verify source/artifact SHA.
- `local`: delegate measurement or use `--stats-only`; emit the fixed raw-data
  statistics contract including `MEDIAN_PARENT_US`, `MEDIAN_CANDIDATE_US`,
  `MEDIAN_RATIO_DELTA`, `PAIRED_MEDIAN_DELTA`,
  `PAIRED_MEDIAN_PERCENT_DELTA`, `RAW_SAMPLE_COUNT`,
  `MEASUREMENT_QUALITY`, and `NOISE_STATUS`.
- `record`: delegate evidence recording and SHA checks; never commits, pushes,
  or submits Online by itself.
- `dedup`: exclude explicit common Champion/ABI scaffold and emit
  `PATCH_SIMILARITY`, `MECHANISM_OVERLAP`, gate status, and `DEDUP_STATUS`; the
  hard gate is `PATCH_SIMILARITY <= 0.60`.
- `cycle`: run the existing delegated stages through an atomic checkpoint with
  idempotent completed-stage handling.
- `resume`: resume only an explicit checkpoint and do not rerun passed stages.
- `report`: read a checkpoint and emit its structured receipt.

All execution paths must retain current-worktree/expected-branch isolation,
source/artifact SHA validation, backend return-code propagation, and the
no-second-runner/no-second-scorer constraints.

## Main-2 Acceptance Gaps

The following read-only review findings are part of this request and must be
closed by the Tooling/Integration Owner before claiming a five-route canonical
entrypoint:

1. The route implementation currently fixes `EXPECTED_REPO_ROOT`,
   `EXPECTED_BRANCH`, and `ROUTE` to this R2 private tree. It cannot be copied
   unchanged as a five-route canonical entry. The Owner must design one unique
   published entrypoint that binds each invocation to its caller-supplied
   route root and expected branch, while still rejecting cross-worktree paths.
2. `dedup` blocks only when `PATCH_SIMILARITY > 0.60`. A low patch similarity
   can still produce a high numeric `MECHANISM_OVERLAP` and
   `DEDUP_STATUS=DISTINCT`. The canonical gate must treat mechanism overlap as
   an independent blocking or explicit manual-acceptance signal, with its
   threshold and receipt fields defined by the Owner.
3. `local` validates raw pair/quality/CV data only. It does not establish
   device-event provenance, the required 45 warmup samples, PC/CP interleaving,
   or unique Runner identity. E2E acceptance must supply real backend output,
   raw event provenance, interleaving/warmup evidence, and contamination
   rejection evidence.
4. `record` must leave a traceable link from each evidence artifact to the
   exact Parent/Candidate identity, including the relevant source/artifact
   SHA and the publication or evidence receipt. A successful backend return
   alone is not sufficient.

R2 is requesting these design and acceptance changes only; it is not creating
an alternate wrapper or a second script.

## Verification Boundary

The route self-test is:

```text
python3 -B 研究/REGISTER-SPILL-ELIMINATION-X/tests/test_w5ctl.py
```

Recorded result: `Ran 5 tests in 2.750s`, `OK` (`5/5`). This is a contract
self-test with route-local fake backends and read-only checks. It is not an
end-to-end CANN build, correctness, Local measurement, or evidence publication
test.

The prior `doctor --dry-run` result was `DOCTOR_STATUS=PASS` with
`EXECUTION_READINESS=BLOCKED`; `BUILD_BACKEND`, `CORRECTNESS_BACKEND`,
`LOCAL_BACKEND`, and `RECORD_BACKEND` were all `NOT_CONFIGURED`. Therefore no
real backend has been accepted by this route and no E2E PASS may be inferred.

## Publication Receipt Status

REQUEST_STATUS=ISSUED; AWAITING_OWNER_ACK
REQUEST_UPDATE_SOURCE=MAIN-2_READ_ONLY_REVIEW
REQUEST_ARRIVAL=UNCONFIRMED_FROM_THIS_WORKTREE
REQUEST_REACHED=UNCONFIRMED_FROM_THIS_WORKTREE
REQUEST_ACCEPTED=UNCONFIRMED
TOOLING_OWNER=Existing canonical Tooling/Integration Owner
RESPONSIBLE_OWNER=Existing canonical Tooling/Integration Owner
W5CTL_INTEGRATED=NO
W5CTL_CANONICAL_ENTRYPOINT=PENDING_OWNER_PUBLICATION; PROPOSED=研究/REGISTER-SPILL-ELIMINATION-X/w5ctl
PUBLICATION_COMMIT=NONE
PUBLICATION_RECEIPT=NONE
W5CTL_END_TO_END_TEST=BLOCKED_NOT_RUN; REAL_BACKENDS_NOT_CONFIGURED

BLOCKERS_WITH_RESPONSIBLE_OWNER:

1. `Tooling/Integration Owner`: acknowledge this request and publish the
   `c24cdb7e` baseline through the existing canonical tooling path.
2. `Tooling/Integration Owner`: provide or explicitly bind the already-approved
   Build, Correctness, Local, and evidence-recording backends; this route does
   not create or copy any backend.
3. `Tooling/Integration Owner`: run and receipt the integrated E2E sequence
   after backend wiring. Until then, the 5/5 self-test remains non-E2E.
4. `Tooling/Integration Owner`: replace the R2-fixed root/branch/route
   assumptions with per-invocation route binding in the unique canonical
   entrypoint and retain cross-worktree rejection.
5. `Tooling/Integration Owner`: define and enforce the independent mechanism
   overlap gate/manual-review signal in `dedup`.
6. `Tooling/Integration Owner`: provide device-event provenance, 45-warmup,
   PC/CP interleaving, unique Runner identity, and contamination rejection in
   the real Local E2E receipt.
7. `Tooling/Integration Owner`: make `record` evidence traceable to exact
   Parent/Candidate identities and their SHA metadata.

No resource probe, Candidate edit, V001, Runner change, shared-entrypoint
change, or Online submission is authorized by this request.

## R2 Real Backend Adaptation Request — 2026-10-10

This addendum supersedes stale implementation-gap descriptions above where the
route-owned CLI has since changed. It does not claim canonical publication or
end-to-end acceptance.

```text
REQUEST_UPDATE_COMMIT_BASE=5008dab25507d211b83a20a4acc99e337f3761a3
W5CTL_IMPLEMENTATION_COMMIT=7946399307959a9dd8fdeab984032a5a884519ca
TOOLING_OWNER=Unique canonical Tooling/Integration Owner
TOOLING_OWNER_NAMED_IDENTITY=UNKNOWN_FROM_PUBLISHED_R2_CONTEXT
REQUEST_CHANNEL=ROUTE_OWNED_REQUEST_ON_RESEARCH_BRANCH
OWNER_ACK=NOT_RECEIVED
W5CTL_INTEGRATED=NO
CANONICAL_ENTRYPOINT=NONE_CONFIRMED
PUBLICATION_COMMIT=NONE
E2E=BLOCKED_NOT_RUN
```

The concrete request to that single Owner is to publish/adapt the current
route-owned entrypoint once, at the canonical location selected by the Owner,
and bind each call to the invocation's own `W5CTL_REPO_ROOT`,
`W5CTL_EXPECTED_BRANCH`, `W5CTL_ROUTE`, and `W5CTL_ROUTE_ROOT`. Preserve the
R2 CLI's SHA guards, real return-code propagation, scoped inputs, checkpoint
identity, and no-second-runner/no-second-scorer behavior. Do not make R2 copy
or modify shared Tooling.

The already documented historical executables are discoverable and executable
in this worktree, but are not route-agnostic adapters:

- `本地实验/REDUCE-INVSCALE-X/V002/support/compile.sh` derives `ROOT` from its
  own script location, configures/builds that route's fixed CMake targets, and
  writes its own `build/` and `compile.log`; it does not consume
  `W5CTL_SOURCE` / `W5CTL_ARTIFACT` as build inputs/outputs.
- `本地实验/REDUCE-INVSCALE-X/V002/support/run_correctness.sh` uses that
  route's `$ROOT/build/reduce_invscale_{parent,candidate}_probe` binaries and
  writes its own `results/`; it does not consume the R2 source/artifact
  identity.
- `本地实验/REDUCE-INVSCALE-X/V002/support/run_probes.sh` uses that route's
  fixed `v001ref` and candidate probe binaries. Its paired sequence is four
  pairs and its default output is that route's `results/`; it does not provide
  the required R2 binary identity, 45-warmup/device-event provenance, unique
  Runner identity, or a complete contamination receipt.
- The published Record Owner skill defines an asynchronous shared-record role,
  not an executable `W5CTL_RECORD_BACKEND`. `dedup` is implemented inside the
  route-owned `w5ctl`; it has no separate backend.

Please provide or bind only existing approved Build, Correctness, Paired Local,
and evidence-recording paths that honor the following caller contract, or
specify the already-existing compatible paths and exact invocation:

```text
INPUTS: W5CTL_ROUTE, W5CTL_STAGE, W5CTL_SOURCE, W5CTL_SOURCE_SHA256,
        W5CTL_ARTIFACT, W5CTL_ARTIFACT_SHA256, W5CTL_BACKEND_WORKDIR
LOCAL:  W5CTL_RAW_PATH plus exact Parent/Candidate artifact identities,
        unique existing Runner identity, device-event provenance,
        45 warmup samples, PC/CP interleaving and contamination fields
OUTPUTS: all artifacts, logs, raw data and evidence remain under the bound
         caller's worktree/route evidence directory; return the backend's
         real process status
RECORD: evidence paths linked to exact Parent, Candidate and Artifact SHA,
        route/branch and resulting evidence receipt; use the existing Git /
        Record Owner workflow, not a second ledger
DEDUP: keep the existing in-wrapper path; exclude explicit shared Champion/ABI
       scaffold; PATCH_SIMILARITY must be <= 0.60; mechanism overlap is an
       independent manual/Owner review signal and numeric overlap never proves
       orthogonality
```

The route's real command results are:

```text
doctor --dry-run: RC=0; root/branch/scope and Parent SHA checks PASS;
  Build/Correctness/Local executables READY; Record backend NOT_CONFIGURED;
  EXECUTION_READINESS=BLOCKED.
build: RC=65; pre-delegation SOURCE_MISSING for the intentionally absent
  R2 Candidate path; historical backend was NOT invoked.
correctness: RC=65; same R2 source identity preflight; historical backend was
  NOT invoked.
local: RC=65; SOURCE_MISSING; raw sample count 0, all latency/statistics NONE,
  MEASUREMENT_QUALITY=NOT_RUN; historical backend was NOT invoked.
record: RC=78; evidence receipt SHA was read, then BACKEND_NOT_CONFIGURED.
dedup: RC=78; parent/candidate patch inputs missing;
  SIMILARITY_INSUFFICIENT_DATA, gates NOT_EVALUATED.
```

No backend was passed off as executed: invoking the listed historical scripts
would build/run their fixed route binaries and write their route-local result
directories, not validate R2. R2 has no authorized Candidate or Artifact
because the register-spill hypothesis remains `FEASIBILITY_UNPROVEN`; do not
use the top Champion as a donor or create V001 to satisfy backend wiring.
This is a source/identity and backend-contract limitation, not an HBM/load
stop. At the recorded current snapshot, device 3 had 65,536 MB HBM capacity,
3,429 MB used (about 62,107 MB free), AICore 0%, and no running process shown;
external processes, device utilization, load, and leases are not execution
gates.

```text
REQUESTED_RESPONSIBLE_OWNER=Unique canonical Tooling/Integration Owner
OWNER_PERSON_NAME=UNKNOWN; OWNER_ACK_REQUIRED
R1_R5_WORK=NOT_BLOCKED_BY_THIS_R2_REQUEST
R2_HYPOTHESIS=FEASIBILITY_UNPROVEN
R2_CANDIDATE=NONE; V001=NO
NEXT_ACTION=OWNER_ACK_AND_ROUTE_AWARE_EXISTING_BACKEND_BINDING
BLOCKER=NO_R2_CANDIDATE_OR_ARTIFACT; NO_ROUTE_AWARE_BACKEND_CONTRACT;
         NO_RECORD_BACKEND; NO_CANDIDATE_PATCH_FOR_DEDUP
```
