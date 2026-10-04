# ADDR V001 Online Package Identity Audit — 2026-10-04

## Scope

Read-only audit of the requested `HOTLOOP-ADDR-HOIST-CHAMPION-X V001 / H3` candidate. This records package identity and eligibility evidence; it does not submit to CANNJudge or modify Route source/evidence.

## Source identity

| Check | Result | Evidence |
|---|---|---|
| Candidate source | `26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602` | SHA-256 of `本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/submission.asc` |
| Sidecar | MATCH | `submission.sha256` contains the candidate SHA above |
| Source commit | `d1de6208d9c52b78c79db59c2cfd1d79ffbe340f` | Its committed `submission.asc` blob hashes to the same candidate SHA |
| `source-meta.json` local SHA | MATCH | `LOCAL_SHA256` matches the candidate; `SOURCE_COMMIT` matches the source commit |
| Direct Parent | `R31B V011` | `REVISION-DECLARATION.md` |
| Parent source SHA | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` | Recomputed from `线上结果/R31B/V011/submission.asc`; matches declaration/source metadata |

Identity of the local candidate and its recorded parent is verified. No remote Judge SHA exists because no submission was made.

## Eligibility evidence

- Candidate build: **PASS**, exact source SHA above; bounded support/build-fix record is `本地实验/HOTLOOP-ADDR-HOIST-CHAMPION-X/V001/BUILD-FIX-2.md` (build-fix commit `8ef756a385d57548f447ded85964a5a0bcb947ec`).
- BF16-D32768 local timing path: the score audit records Parent/Candidate correctness as passing for this selected shape.
- Required BF16-D40960 correctness: **INCOMPLETE / SHARED_RUNTIME_BLOCKER**. The known-good runner reports Parent and Candidate both terminate with `aclrtSynchronizeStream=507035`; the final classification explicitly says `ONLINE=NOT_PERMITTED` and `PERFORMANCE=NOT_ELIGIBLE` (`CORRECTNESS-D40960-FINAL-CLASSIFICATION.md`, commit `53db80381963ab624efd95b204c96bf11b0a4876`).
- Local score: **-24.193320%, QUALITY=POOR**, recorded in `LOCAL-SCORE-AUDIT-20261003.md` and score commit `14dd05d8097ce814c0070da79be29370fb803ece`. This is an engineering local delta, not an Official score and not `LOCAL_ACCEPTED`.
- Stale snapshot warning: the existing `source-meta.json` and `local-result.json` still contain the initial `BUILD_FAILED` / `CORRECTNESS=NOT_RUN` snapshot. Later build, correctness-diagnostic, and score evidence exists in separate commits. Those historical files were not overwritten; the discrepancy must not be mistaken for the latest state.

## Gate

```text
SOURCE_IDENTITY=PASS
PARENT_IDENTITY=PASS
BUILD=PASS
CORRECTNESS=INCOMPLETE
LOCAL_SCORE_EVIDENCE=PRESENT_QUALITY_POOR
ONLINE_READY_PACKAGE=NO
READY_FOR_EXTERNAL_JUDGE_OWNER=NO
DIRECT_ONLINE_SUBMISSION=0
```

The source bytes are identifiable, but a complete required correctness PASS is absent and the Route's own final correctness classification prohibits Online. Therefore this candidate cannot truthfully be labelled `ONLINE_READY_PACKAGE` or handed off as Judge-ready in this audit. No new qualification campaign was started; resolving the shared D40960 runtime gate remains outside this bounded identity check.
