# Historical Judge Payload Provenance Audit

Date: 2026-09-23
Scope: R31A-V016 priority; then R31B-V011 and other Bests.
Constraint: no kernel edits, no CANNJudge resubmission. EXACT_GIT_BLOB route-source recovery is **not** overturned.

## Executive conclusion

`EXACT_GIT_BLOB` proves the route file bytes in Git and in `phase4/online/**/submission.asc`. It does **not** by itself prove the judge compile unit. The CANNJudge pipeline uploads those bytes as `kernel.asc` and then compiles them **inside a multi-file direct-invoke template** (`main.asc` `#include "kernel.asc"` plus `CMakeLists.txt`, `data_utils.h`, `run.sh`, `scripts/*`). Therefore historical Judge payloads are recorded as `TRANSFORMED_VERIFIED`.

---

## R31A-V016

### ROUTE SOURCE
- path: `phase4/workspaces/R31A/R31A-V016-submission.asc`
- commit: `7792a255efdb80013d84b1e2c813388a640f2c3f`
- sha256: `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0`
- status: `EXACT_GIT_BLOB` (retained)

### HISTORICAL JUDGE (success 45.00)
- submission id: `6ab2c9790304f72a56a3c465`
- submit script: `脚本/cannjudge-submit.mjs` via `npm run cannjudge:submit -- --yes --source ...`
- working directory: `/Users/sunyiyang/Desktop/Project/cann`
- source argument: `/Users/sunyiyang/Desktop/Project/cann/phase4/workspaces/R31A/R31A-V016-submission.asc`
- transformation (client): UTF-8 decode + re-encode identity check only; **no wrapper on the client**
- transformation (server template): `main.asc` contains `#include "kernel.asc"` after predefined `TensorInfo`/`TensorGroupInfo`; CMake target `add_rms_norm_bias_custom` builds `main.asc`
- temporary generated source (client): none
- actual uploaded payload: `POST /api/submissions/submit` body `files[0] = { path: "kernel.asc", content: <UTF-8 source> }`
- payload sha256 (uploaded content): `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0`
- score: 45.00 / 15/15

Submit pipeline (both historical and current scripts):

```text
--source file
  → read bytes
  → UTF-8 decode / re-encode identity check
  → sha256(content)   # SUBMIT_CLIENT_HASH
  → files[0].content = content   # path kernel.asc
  → judge template assembly (main.asc #include kernel.asc + other template files)
  → bisheng/ASC compile of assembled project
```

### CURRENT FAILED SUBMISSION
- submission id: `6ab35deb0304f72a56db63d6` (as cited)
- source argument: recovered `phase4/online/R31A/V016/submission.asc` (as cited)
- input sha256 (as cited in failed-submit log): `ab5bb3085f3800bbd4f99b633c4bb272cf5636f2739fe4a99bb17ef1e5cd8b1c`
- payload sha256: **UNRESOLVED** (see SHA identities below)
- compile diagnostic: **MISSING**
  - read-only `python3 脚本/cannjudge.py poll 6ab35deb0304f72a56db63d6 --once` → HTTP 403
  - Chrome cache key exists for `https://cannjudge.cn/api/submissions/6ab35deb0304f72a56db63d6` but body is not extractable plaintext

### SHA identities

| digest | identity | evidence |
|--------|----------|----------|
| `dd13093823c885e785a650abff4863827e652eb8607ad0621a96eb31b6764fa0` | SHA256 of `R31A-V016-submission.asc` file bytes / UTF-8 content. Equals git blob at `7792a25`, online snapshot, worktree copy, and historical `result.json` `source.sha256`. That `result.json` field is **SUBMIT_CLIENT_HASH** from `cannjudge-submit.mjs`, not a judge-returned digest. | `sourceFromBytes` in `脚本/cannjudge-submit.mjs`; `git show 7792a25:...`; `phase4/online/R31A/V016/submission.sha256` |
| `ab5bb3085f3800bbd4f99b633c4bb272cf5636f2739fe4a99bb17ef1e5cd8b1c` | **UNRESOLVED artifact identity.** Not the V016 file, not V017 (`1f60dfc4…`), not V018 (`ee20861a…`), not assembled `main.asc`+kernel (`5f4e3a24…` with current `/tmp/cannjudge_template.json`), not common JSON payload serializations, not CRLF / no-final-newline encodings. Only appears in the operator-cited failed-submit log (not present as a workspace file). | filesystem hash scan; encoding variants |

### ROOT CAUSE (bounded by evidence)

1. **Not proven:** that `ab5bb308…` is the same bytes as `dd130938…`. If the failed submit really uploaded `ab5bb308…`, it was a **different payload**, and CE does not refute `EXACT_GIT_BLOB` for V016.
2. **Not proven:** that the judge template/compiler used for `6ab2c979…` is identical to the one used for `6ab35deb…`. Same `kernel.asc` bytes can CE if `main.asc` / toolchain / flags changed.
3. **Proven:** client upload is content-identical to the source file (no client wrapper). Judge compile is template-transformed (`#include "kernel.asc"`).
4. **Therefore:** `EXACT_GIT_BLOB` = exact **route/upload content**. Exact **judge compile unit** requires template provenance and is `TRANSFORMED_VERIFIED`, not `EXACT_VERIFIED`.

---

## R31B-V011

- route source: `phase4/workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc` commit `43a1049a…` sha256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- historical payload upload content: same sha256 (SUBMIT_CLIENT_HASH / result.json)
- judge compile unit: template `main.asc` + `#include "kernel.asc"` + template siblings
- payload status: `TRANSFORMED_VERIFIED`
- no resubmission performed in this audit

## OTHER BESTS

Same mechanism (single `cannjudge-submit.mjs` path; no per-route wrapper found):

| Route | Rev | route_source_sha256 (prefix) | judge_payload_status |
|-------|-----|------------------------------|----------------------|
| MIX-A | V003 | `1a1857a945ce…` | TRANSFORMED_VERIFIED |
| A001 | V017 | `c906bc45c0bf…` | TRANSFORMED_VERIFIED |
| G001 | V002 | `f5fc7a57131c…` | TRANSFORMED_VERIFIED (PROVENANCE_TAINTED kept) |
| H001 | V008 | `ec3a631d1b61…` | TRANSFORMED_VERIFIED |
| I001 | V004 | `1ef42c724b56…` | TRANSFORMED_VERIFIED |

Historical and recovery submits used the same script (`脚本/cannjudge-submit.mjs`) and the same payload construction (`files[0].path=kernel.asc`). No second submit pipeline was found in-repo.

## Migration hash provenance

| recorded field | origin class | meaning |
|----------------|--------------|---------|
| `result.json` → `source.sha256` | **SUBMIT_CLIENT_HASH** (historical submit-time) | hash of uploaded content string, computed by submit client |
| `source-meta.json` → `submission_sha256` / `route_source_sha256` | **RECOVERY_COMPUTED_HASH** matching git blob and SUBMIT_CLIENT_HASH when bytes match | migration-time recompute + cross-check |
| judge-returned source/payload digest | **HISTORICAL_JUDGE_HASH: MISSING** | API does not return a source digest (`cannjudge.py` notes remote source cannot be reverse-fetched) |

## Metadata fixes applied

Per record (`source-meta.json`):

- `route_source_status`
- `route_source_sha256`
- `judge_payload_status`
- `judge_payload_sha256`
- `judge_payload_generation`
- `hash_origin` (`result_json_source_sha256`, `migration_submission_sha256`, `judge_native_source_hash`)
- V016 extra: `sha_identity_note`, `historical_submit`, `current_failed_resubmission`

`EXACT_GIT_BLOB` / `source_recovery` fields retained unchanged.

## Migration assessment

- Git source recovery: **VALID**
- Judge payload recovery: **PARTIAL**
  - upload-content identity: recovered
  - compile-unit wrapper: documented (`TRANSFORMED_VERIFIED`)
  - judge-native payload hash: **MISSING**
  - `ab5bb308…` artifact: **UNRESOLVED**
  - `6ab35deb…` compiler diagnostic: **MISSING** (403 / cache body unavailable)

## Unresolved items

- `PARENT_UNRESOLVED`: none for Best diffs (already `EXACT_DIRECT_PARENT_SOURCE_DIFF`)
- `UNRESOLVED SOURCE PROVENANCE`: judge-native payload hash; `ab5bb308…` file identity; `6ab35deb…` compile diagnostic text
