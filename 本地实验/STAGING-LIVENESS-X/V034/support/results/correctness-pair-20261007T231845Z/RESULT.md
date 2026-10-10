# STAGING-LIVENESS-X V034 Route Result

ROUTE: STAGING-LIVENESS-X  
REVISION: V034  
PARENT: V033  
CANDIDATE_SOURCE_SHA256: `75131108d32bc587c3c289d53ded04a15c5224417c675cb37a46df032e104d33`  
PARENT_SOURCE_SHA256: `0df982248c80c4f2e5541dc730b0a25ba97544efc3b3e3e3409c24deed2d2eff`

## Compile

V034 full direct-invocation runner compile and link passed. The runner is an AArch64 executable embedding the exact V034 source; executable SHA-256 is `b00c5ef56cdffff8d2acd99fdcfebabd8cd699815d886c50ce7f88d7f836b87e`. Parent V033 was built separately with the same wrapper. Build logs and identity checks are referenced by `BUILD-RESULT.md`.

## Correctness

The route-local synthetic T01-T15 suite completed for both binaries on device 1. It is not a claim of the unpublished official T01-T15 mapping.

| Revision | PASS cases | Failed case | Matched ratio | Max absolute error |
|---|---:|---|---:|---:|
| V033 Parent | 14/15 | T14 FP32 `[2,1,2,32768]` | 0.172195 | 3.5058006 |
| V034 Candidate | 14/15 | T14 FP32 `[2,1,2,32768]` | 0.155510 | 3.4189978 |

The V034 T14 runner returned successfully and reported the frozen source SHA; the failure is numerical. Per-case outputs, deterministic inputs, golden outputs, runner logs, summaries, and the device pre-run snapshot are retained in this directory.

## Result

- `COMPILE=PASS` for the full direct runner; the earlier object-only Compile PASS remains a separate historical record.
- `CORRECTNESS=FAIL` for V034 because T14 failed.
- `LOCAL_SCORE=NONE`; Local was not run because Candidate Correctness did not pass.
- `ONLINE=FORBIDDEN`.
- Candidate remains frozen. No V035 was created and no shared records were changed. Await Planning direction; this result does not close the Route.
