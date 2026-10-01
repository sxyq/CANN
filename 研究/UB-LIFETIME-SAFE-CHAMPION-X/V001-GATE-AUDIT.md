# UB-LIFETIME-SAFE-CHAMPION-X V001 Gate Audit

- Audit date: 2026-10-01 UTC
- Scope: read-only gate audit for `UBX-H1-FP16-RETAINED-Y-INPLACE-OUTPUT`.
- Planning state remains `MAIN_SELECTED=YES`; no shared ledger or dispatch file was changed.

## Gate results

- Source identity: PASS. `submission.asc` matches `submission.sha256` and the
  `candidate_source_sha` / `local_sha256` values in `source-meta.json`:
  `9b73bb5626b5e98be53faeb91eba024b2589bf0b8b5b46e36c48ec18e009f989`.
  The parent sidecar also verifies the declared parent SHA
  `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Static lifetime proof: PASS_STATIC_LIFETIME_PROOF remains supported by the
  committed proof and the V001 patch. The FP16 retained `yTile` read is before
  the same indexed `yStore` range is assigned to `outputLocal`; the existing
  store and `SyncMTE3ToV` ordering remains. The BF16 branch retains its
  `outputBuf_` allocation and Get.
- Build: PASS. `build-result.json` records exact-source compile and AICore link
  return codes 0. The recorded device/submission artifact sizes and SHA-256
  values match the files under `support/alink-build/`.
- Executable identity: PASS, as recorded in `source-meta.json` and
  `build-result.json`.
- Correctness: NOT_RUN. No correctness result, pass/fail report, or NPU
  correctness log exists in V001. The untracked
  `support/correctness/` and `support/correctness-build/` directories are
  retained unchanged.
- Performance and Online: NOT_ELIGIBLE / NOT_AUTHORIZED. No timing,
  profiling, HBM, lease, or Online fact was generated.

## Blocker

Server3 is unavailable for this audit. No remote task was started, and no
correctness, HBM, or lease result is inferred. Correctness remains an open
gate; this record does not authorize timing, profiling, V002, or Online.
