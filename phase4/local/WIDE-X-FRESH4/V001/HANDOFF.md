# WIDE-X-FRESH4 V001 Handoff

Status: V001 targeted correctness is retained as PASS_9_OF_9_NPU; the separate paired runner now compiles and links. Returned to Main review. No timing result.

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
- The separate Parent/Candidate paired runner also compiles and links on server3. Runner SHA256: `a74faf2fcf05819db15b5d0f96401f4aa1d9e09b6a54d88be870918a953eb3c4`; Parent and Candidate library SHA256 values and source identity are in `support/runner-validation.md` and `logs/server3-build-attempt-05.log`.
- The preflight captured at 2026-09-25T07:30:53+0000 reported device 4 AICore 0%, HBM 59186/65536 MB (6350 MB free), and no new non-VLLM NPU process. The executable started at 2026-09-25T07:32:55+0000, 122 seconds later; see `logs/npu-correctness-preflight.log`.
- Correctness executable compile/link: PASS and runtime dependencies resolve; see `logs/link.log`. It ran on device 4 and exited 0 after passing all 9 FP32/FP16/BF16 x 2048/16384/32768 cases; see `logs/npu-correctness.log` for exact errors.
- No latency timing was run; no exclusive device lease was granted.
- No Online submission was made.

## Evidence

- `submission.asc` and `submission.sha256` retain the exact candidate source.
- `diff.patch`, `source-meta.json`, and `local-result.json` retain declaration, source identity, and results.
- Build output directory on server3: `/tmp/WIDE-X-FRESH4-V001-build-20260925/cmake-build`.
- Paired-runner build output directory on server3: `/tmp/WIDE-X-FRESH4-V001-d0dd0e972ac48b07/WIDE-X-FRESH4/V001/support/build`.
- `logs/link-env-attempt.log` and `logs/link-include-attempt.log` retain two unsuccessful environment/setup attempts; the successful link is recorded in `logs/link.log`.
- `logs/npu-correctness-preflight.log` and `logs/npu-correctness.log` retain the device snapshot and full targeted NPU result.
- `logs/server3-build-attempt-01.log` through `logs/server3-build-attempt-06.log` retain the failed builds and successful compile/link. Attempt 06 confirms driver-library resolution from the installed server3 path. The runner was not executed; no NPU operation or timing was run for it.

Next action belongs to Main: review V001 with targeted NPU correctness complete. No timing or performance result is included.
