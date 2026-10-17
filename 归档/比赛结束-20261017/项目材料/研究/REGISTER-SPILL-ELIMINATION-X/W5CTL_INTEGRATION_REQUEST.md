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
