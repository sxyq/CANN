# V008 MAIN2-CANONICAL-V1 route-local result

ROUTE: `CROSSROW-EPI-PROLOGUE-X`  
REVISION: `V008`  
AGENT_ID: `W5-R01/Zeno`  
BRANCH: `research/w5-r01-crossrow-epi-prologue`  
WORKTREE: `/home/data4t2/lelinfeng/cann-w5-r01-crossrow`  
SOURCE_COMMIT: `5bea6f480e8359abb82a69506862f035aded8e21`  
DIRECT_PARENT: R31B/V011 (V007 was not Parent)  
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`  
CANDIDATE_SOURCE_SHA256: `7c6f620ce231e9ebad71b7c8a37bcc8e2ae45ce56086e6f978f02cd1ca9cce06`  
PARENT_ARTIFACT_SHA256: `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64`  
CANDIDATE_ARTIFACT_SHA256: `9c2e0df98f22e9fd4cd6c94bd2743dc08ee98448413014b145bbd96e24c6809c`  
RUNNER_REF_SHA256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`

## Measurement

Suite: `CANONICAL_LOCAL_SUITE_V1`; four scoring Core cases C12/C13/C14/C16. Device 7; 45 warmups, 31 device-event samples per side, three interleaved pairs per case in P-C / C-P / P-C order. This produced 24 side runs and 744 retained event samples. Every per-run RC=0 and `bad=0`; the pinned scorer validated all 24 raw files, shape/dtype, source and runner identities, and sample counts. Same-binary qualification: `NOT_RUN`.

| Core | Shape / dtype | Anchor median (us) | Parent run medians (us) | Parent median (us) | Candidate run medians (us) | Candidate median (us) | Candidate faster paired run medians | Candidate quality |
|---|---|---:|---|---:|---|---:|---:|---|
| C12 | 8x8192 BF16 | 6.58 | 31.34 / 17.16 / 14.74 | 17.16 | 14.28 / 22.08 / 31.68 | 22.08 | 1/3 | POOR |
| C13 | 128x16384 FP16 | 20.80 | 32.42 / 27.54 / 36.64 | 32.42 | 20.70 / 28.78 / 30.04 | 28.78 | 2/3 | POOR |
| C14 | 128x16384 BF16 | 25.96 | 51.64 / 33.72 / 25.40 | 33.72 | 35.36 / 25.74 / 34.96 | 34.96 | 2/3 | POOR |
| C16 | 9x32768 BF16 | 15.06 | 20.86 / 17.02 / 19.24 | 19.24 | 27.68 / 50.74 / 16.82 | 27.68 | 1/3 | POOR |

## Pinned scorer result

The existing published `MAIN2-CANONICAL-V1` pure `scoreVectors` function was imported from scorer commit `6d3ad0b30b21d2462b0fd8716c884f0ff6bdaabb`, scorer SHA256 `f43709695115da53e30e58204f8e7c4716486538397394d7e2cb441f3a1d6d7f`. It returned:

- Canonical score: `54.312261`
- Delta vs R31B/V011 anchor: `-45.687739`
- Paired-Parent diagnostic score: `87.528380`
- Empirical low/high: `23.969468 / 123.065795`
- Candidate faster than same-window Parent by run median: `6/12`; Candidate Core medians beat Parent in `1/4` cases and beat the anchor in `0/4` cases.
- Quality/verdict: `POOR / LOCAL_NEUTRAL`; reliable gain `NO`; `NO_PROMOTION=YES`; Local Best unchanged.

The score was not produced by the Main2 `scoreboard` command; no Main2 scoreboard or metrics were written. Exact scorer output is in `SCORER-OUTPUT.json`, with RC in `SCORER-RC.txt`.

## Device/load and raw evidence

NPU7 HBM use was 37% (65536 MB total; approximately 41 GB free) in all 48 per-run pre/post snapshots. AICore ranged 0–42%; AIVector ranged 0–90%. `npu-smi info proc -i 7` reported that process querying is unsupported, so process visibility is unavailable; no clean-device claim is made. Pre/post load and every sample's raw, stats, log, RC, and SHA are retained under `canonical-paired-20261011/`; `MEASUREMENT-INDEX.tsv` is the 24-run identity/raw manifest.

The Local wrapper reported status 1 although its internal totals were `LOCAL_RC_TOTAL=0`, `LOCAL_BAD_TOTAL=0`, `RESOURCE_ABORT=0`, and all 24 per-run RC files are 0. This discrepancy is preserved and is not hidden as a clean wrapper exit.
