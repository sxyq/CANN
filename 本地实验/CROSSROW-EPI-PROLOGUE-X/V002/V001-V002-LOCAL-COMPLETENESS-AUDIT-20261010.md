# W5-R01 V001/V002 Local completeness audit — 2026-10-10

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
AUDIT_SCOPE: own V001/V002 evidence only; no other Route private source, worktree, or unpublished diff read
AUDIT_HEAD: 38b26f0d0e6e0aa4ab05846886f30634756ec558

## Rule and manifest result

The route-owned evidence was checked against the current Local requirements for raw samples, shape/dtype, runner, warmups, repeats, event method, Parent/Candidate identity, HBM/load context, and current Local status. No route-owned full-shape manifest, shape matrix, sweep manifest, coverage manifest, or full-score aggregation record exists under V001 or V002. The current Local specification requires per-measurement shape and numeric raw/median reporting, but supplies no basis for inventing missing shapes or weighting a total score.

FULL_LOCAL_MANIFEST: ABSENT
MANIFEST_PATH: NONE
SHAPE_LOCAL_ONLY: YES
FULL_LOCAL_SCORE: NOT_COMPUTABLE
SCORE_SCOPE: single-shape Local evidence only; no total/overall score inferred

## Shape coverage

Every V001 and V002 Local TSV header identifies the same probe:

SHAPE: rows=16,width=6144,blocks=8,dtype=fp16
BLOCKS: requested=8,effective=8, two rows per effective block
SAMPLE_LAYOUT: 21 paired rows, alternating PC/CP

V001 raw inventory:

- `local-v001-r16-d6144.tsv`: legacy `warmups=5`, `paired_samples=21`, SHA256 `d3becb243132985934b7c6c2774ba0fab68f84a20748abfafb3fcac8c50e09ca`.
- `local-v001-r16-d6144-w45.tsv`: `warmups=45`, `paired_samples=21`, device-event/same-binary PC/CP result, SHA256 `705bcccea7731bf7f3c4316de7e5cbba5b7b962318dd972b85ff4bfc693d4c`.
- `local-v001-r16-d6144-w45-label-fix-retry.tsv`: `warmups=45`, `paired_samples=21`, corrected-label event run, SHA256 `2a373b609921a89b1eb54c2358c01e1a51135737311f6a56e8efb5f833e8c443`; target-device load was contaminated and it was not promoted.

V002 raw inventory:

- `local-v002-r16-d6144-w45-20261010.tsv`: `warmups=45`, `paired_samples=21`, device-event PC/CP result, SHA256 `9cbb9bb254c2e2560021ccf8f9aebec3af3a509facaaef1b0a6bdc4452c0494c`.
- `local-v002-r16-d6144-w45-20261010T1345.tsv`: `warmups=45`, `paired_samples=21`, device-event PC/CP result, SHA256 `0124ff6d57981c58bb74be86937a05c7f46813e9797a069a44047da052bb3f4d`.
- `local-v002-r16-d6144-w45-recheck-20261010T153103.tsv`: `warmups=45`, `paired_samples=21`, device-event PC/CP result, SHA256 `a231b27b7653b2180c3fa10acf8239d5858fd5961bd0d88cf08f79b8fe6e4d8d`.
- `local-v002-r16-d6144-w45.tsv`: `warmups=45`, `paired_samples=21`, device-event PC/CP result, SHA256 `797d0e3ee39f1ff927041fb5c75ca63282b98a2e31650255c21f154cbd5dfeda`.

The repeated TSVs are repeated evidence at one shape, not coverage of additional Local shapes. No raw was overwritten or filtered by this audit.

## Identity and qualification

V001 identity:

- Parent source `V001/Parent.asc`: SHA256 `6075d390df86bb56f7db4d025f11c244791f9d8c722cb4091898fa01940cf794`.
- Candidate source `V001/Candidate.asc`: SHA256 `3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d`.
- Corrected-label V001 Runner source: SHA256 `e4c46069d21f16b863775461298b452b063430d7ca14fdad3c56ae8c0826d938`.
- Restored V001 artifacts: Parent `.so` `61eb3391924fabbc7b72231652914df2688ca28d376ed143b5d351042b93e944`; Candidate `.so` `278dbd1cd0866f30ce25b2ea9a3c5fd5214b641bc8ae78e3de52b6d35c3ff34b`; Runner `5210262b45c50878c24def38e108386b45bd017086a61a1293ab290d171a4d5a`.
- V001 45-warmup event record: `LOCAL-W45-RESULT.md` reports device-event/same-binary PC/CP, 45 warmups, 21 pairs, and `LOCAL_NO_PROMOTION`; the corrected-label retry is separately retained with target load contamination.

V002 identity:

- Parent source `V002/Parent.asc`: SHA256 `3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d`.
- Candidate source `V002/Candidate.asc`: SHA256 `426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f`.
- V002-bound rebuilt artifacts: Parent `.so` `3ad998adbb7a126f15b7da0a8f8e0a816ca719d92a4dc42aba97d7832ffa4db4`; Candidate `.so` `0ef46469d226225c47ca4c33286b505b48444175095572bc3abf0e8d49c30126`; paired Runner `c86666a71bcb8809dbaec90255bfad9ee15673b1725453b4ce950b949a7dec3d`; ABI `0897a04ba82bda39d53e77d548966eb54a6b7ec16c17387dccf26d86cbf577d2`.
- Restored current V001 support identity is retained in `restore-v001-support-after-recheck-20261010T153828.log`; tracked support diff is clean.
- V002 45-warmup runs use device-event timing, 21 alternating PC/CP pairs, and the existing paired Runner. The exact-shape Parent same-binary qualification for the final recheck was `NOT_RUN`; it is not a complete qualified Local and is not `LOCAL_ACCEPTED`.

## Status and decision

V001/V002 contain only `SHAPE_LOCAL_ONLY` evidence. Numeric single-shape medians and deltas remain valid raw measurements within their own receipts, but they must not be promoted to `FULL_LOCAL_SCORE` or treated as a total score. The latest V002 recheck remains:

LOCAL_VERDICT: NEEDS_ONE_MORE_LOCAL
NO_PROMOTION: YES
RETRY_REQUIRED: Planning follow-up decision; the current recheck quota is consumed
FULL_LOCAL_SCORE: NOT_COMPUTABLE

The latest V002 raw also records the R2 WIDE-X correctness PID on target NPU3 near the pre-snapshot; the target window was not clean. No further V002 P/C run is performed. No V003 or Runner is created.

NEXT_ACTION: Planning must choose a new mechanism-orthogonal hypothesis or make a route decision. Until then, preserve V002 as unpromoted `NEEDS_ONE_MORE_LOCAL` and do not infer a full Local score.
