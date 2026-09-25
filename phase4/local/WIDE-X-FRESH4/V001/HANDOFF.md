# WIDE-X-FRESH4 V001 Handoff

Status: `NEEDS_ONE_MORE_LOCAL`; stopped for Main review.

## Revision declaration

| Field | Value |
|---|---|
| ROUTE | WIDE-X-FRESH4 |
| REVISION | V001 |
| DIRECT_PARENT | BUILD-FIX-001 |
| PARENT_SOURCE_SHA | `5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be` |
| PARENT_SCORE | N/A |
| SINGLE_HYPOTHESIS | Widen the wide-path tile from 2048 to 4096 and grow only `tmp_` for that instantiation; expected to halve per-row wide-path tile iterations. |
| CONTEXT_CLASS | FRESH_BLIND |
| WHY_NOT_DUPLICATE | Changes tile granularity only; two-pass data flow and single-tile scheduling remain unchanged. |

## Result

- Candidate source SHA256: `f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895`.
- Source diff: two lines in `phase4/workspaces/WIDE-X-FRESH4/wide_x_fresh4.asc` (`kWideTile` and the tile-dependent `tmp_` allocation). Fallback allocation remains 8192 bytes.
- Server3 compile: PASS; see `logs/build.log`.
- Kernel shared-library link: PASS. Library SHA256: `b5abb78cdb2e99383d5266dd3d5b76c0a1c3b022613b53cc1b2a17c5d7868a6c`.
- Correctness executable compile/link: PASS and runtime dependencies resolve; see `logs/link.log`. It was not executed because Main reported the device snapshot as busy/loaded. Planned cases are FP32/FP16/BF16 at widths 2048/16384/32768.
- No latency timing was run; no exclusive device lease was granted.
- No Online submission was made.

## Evidence

- `submission.asc` and `submission.sha256` retain the exact candidate source.
- `diff.patch`, `source-meta.json`, and `local-result.json` retain declaration, source identity, and results.
- Build output directory on server3: `/tmp/WIDE-X-FRESH4-V001-build-20260925/cmake-build`.
- `logs/link-env-attempt.log` and `logs/link-include-attempt.log` retain two unsuccessful environment/setup attempts; the successful link is recorded in `logs/link.log`.

Next action belongs to Main: review this revision and decide when targeted NPU correctness can run under an acceptable device state.
