# WIDE-X-FRESH4 V001 Main Review

Date: 2026-09-25

## Decision

No local performance verdict has been formed. Keep V001 unchanged. Build, executable identity, and exact-source correctness are present. The Direct Parent and source-bound Parent/Candidate executable relation are explicit and verified; the only remaining provenance issue is that the declared `FRESH_BLIND` context is still unconfirmed because the archived analysis path, exposed material, and influence record are missing. Same-binary and timing have not run. No Online submission is supported.

## Lineage and source review

- Route: `WIDE-X-FRESH4`
- Revision: `V001`
- Direct Parent: `BUILD-FIX-001`
- Parent source SHA-256: `5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be`
- Parent Official Score: N/A
- Candidate source SHA-256: `f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895`
- Declared context: `FRESH_BLIND`
- Hypothesis: widen only the wide-path tile from 2048 to 4096 elements and size its temporary buffer for that tile, reducing per-row tile iterations.
- `SINGLE_CHANGE_AUDIT=PASS`. The source diff contains only the tile-width constant and corresponding wide-path temporary-buffer size. Fallback allocation, scheduling, arithmetic, precision, and synchronization remain unchanged.
- Candidate bytes match the retained `submission.asc`, `submission.sha256`, and source metadata.

## Build and correctness evidence

- Server3 Candidate build and link passed. The paired Parent/Candidate runner also compiled and linked after preserving the registered kernel name and separating the ACL host link from the registered kernel-library link.
- Parent and Candidate source digests, runner digest `a74faf2fcf05819db15b5d0f96401f4aa1d9e09b6a54d88be870918a953eb3c4`, and linked library identities are recorded in `support/runner-validation.md`.
- Targeted NPU correctness passed 9/9 cases across FP32, FP16, and BF16 at widths 2048, 16384, and 32768. The exact results and executable identity are in `source-meta.json` and `local-result.json`.
- The paired runner's correctness-only mode was executed on server3 d7: Parent and Candidate each passed 9/9 exact-source cases, for 18/18 RC=0 with zero mismatches and zero nonfinite outputs. Same-binary qualification and latency measurement were not performed.

## Provenance note

The retained runner record says an archived problem-analysis document containing information beyond the task semantics was opened. The Route owner could not identify its path or disclose the extra information, and could not establish whether it influenced the V001 hypothesis. Therefore `FRESH_BLIND` cannot be confirmed from the available record. Preserve the Candidate and its correctness evidence; do not make an Online eligibility claim from this review. Any later Main decision must account for this unresolved provenance.

## Provenance disposition

- Direct Parent: `BUILD-FIX-001`, source SHA-256 `5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be`.
- Parent executable is source-bound to that Parent: module SHA-256 `9f03f15232d984577e559a64e90ec4e550fbef9392d6edb7eedd47ddc530071e`.
- Candidate source SHA-256 `f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895`; Candidate module SHA-256 `c30be731387e0672359f47e748d65f67b7ce0e3f294dbc4f0fe986934b0890ab`; paired runner SHA-256 `6328cd48d92036aeb4266248ede569f08484580e0507e185520911ed1a7e6a8b`.
- Source path and Git evidence: V001 `phase4/local/WIDE-X-FRESH4/V001/submission.asc`, route commit `856ba8d35db2ba402213483b069592b662779ba5`; source, sidecar, Git and workspace bytes agree.
- Disposition: `SOURCE_PROVENANCE_PASS`; `FRESH_BLIND_CONTEXT_UNCONFIRMED`. Do not repeat the existing 18/18 correctness run.

## Next action

Keep V001 and its source digest unchanged. When an allowed performance lease and suitable load window are available, use the exact-source runner for same-binary Parent qualification before any Parent/Candidate comparison. Keep all results local until provenance has a defensible disposition.
