# Local vs Online Calibration (reconfirmed 2026-09-23)

Do not treat compile PASS as local performance score.

## Method

For each major Route Best, look for:

1. local compile
2. local correctness
3. local latency
4. paired latency
5. local proxy score

Then compare to online Official Score and online testcase latency.

Evidence class:

- `COMPILE_ONLY`
- `CORRECTNESS_ONLY`
- `UNPAIRED_LATENCY`
- `PAIRED_LATENCY`
- `LOCAL_PROXY_SCORE`
- `FULL_LOCAL_BENCHMARK`

Result class:

- `MATCH_POSITIVE`
- `MATCH_NEGATIVE`
- `CONTRADICT`
- `NO_COMPARABLE_LOCAL_DATA`

Proxy quality:

- `GOOD` / `WEAK` / `MISLEADING` / `INCOMPARABLE` / `NO_DATA`

## Route Best rows

| route | rev | online_pass | online_score | local_compile | local_correctness | local_latency | paired_latency | local_proxy | evidence_class | online_vs_local | result_class | proxy |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| R31A | V016 | 15/15 | 45.00 | PASS (server3 full_link) | NOT_RUN (server3) | NO | NO | NO | COMPILE_ONLY | no paired local numbers | NO_COMPARABLE_LOCAL_DATA | NO_DATA |
| R31A | V017 | 15/15 | 44.45 | PASS (workspace logs) | probe PASS single shape | unpaired D=24576 -4.33% | NO | single-probe only | LOCAL_PROXY_SCORE | local faster, official **down** 0.55 | CONTRADICT | **MISLEADING** |
| R31B | V011 | 15/15 | 45.16 | PASS (server3 full_link) | NOT_RUN (server3) | NO | NO | NO | COMPILE_ONLY | no paired local numbers | NO_COMPARABLE_LOCAL_DATA | NO_DATA |
| MIX-A | V003 | 15/15 | 44.69 | PASS (server3 full_link) | NOT_RUN (server3) | NO | NO | NO | COMPILE_ONLY | no paired local numbers | NO_COMPARABLE_LOCAL_DATA | NO_DATA |
| MIX-A | V006 local | — | — | PASS | NOT_RUN | NO | NO (device busy) | NO | COMPILE_ONLY | not submitted | NO_COMPARABLE_LOCAL_DATA | NO_DATA |
| A001 | V017 | 15/15 | 36.41 | PASS (server3) | NOT_RUN | NO | NO | NO | COMPILE_ONLY | — | NO_COMPARABLE_LOCAL_DATA | NO_DATA |
| G001 | V002 | 11/15 | — | PASS (server3) | NOT_RUN | NO | NO | NO | COMPILE_ONLY | — | NO_COMPARABLE_LOCAL_DATA | NO_DATA |
| H001 | V008 | 15/15 | 29.04 | PASS (server3) | NOT_RUN | NO | NO | NO | COMPILE_ONLY | — | NO_COMPARABLE_LOCAL_DATA | NO_DATA |
| I001 | V004 | 15/15 | 15.53 | PASS (server3) | NOT_RUN | NO | NO | NO | COMPILE_ONLY | — | NO_COMPARABLE_LOCAL_DATA | NO_DATA |

## Local slot evidence at review time

The rows below preserve evidence as recorded for this calibration review. Current slot ownership is in `scheduler.tsv`: EPI-X-FRESH is PARKED and EXT-ASCEND-X is ACTIVE / NOT_STARTED. This synchronization does not change local measurements or correctness records.

| route | local_compile | local_correctness | latency | decision | evidence_class | note |
|---|---|---|---|---|---|---|
| WIDE-X-FRESH4 | FAIL (server3) | NOT_RUN | NO | NEEDS_LOCAL_SCREEN / COMPILE_FAIL | COMPILE_ONLY(FAIL) | `sqrtf` not viable in aicore |
| MODE-X-R015C | PASS | host PASS; device NOT_RUN | NO | NEEDS_ONE_MORE_LOCAL | CORRECTNESS_ONLY(partial) | NPU probes pending device load |
| EPI-X-FRESH | PASS | **FAIL** 6490/8 mismatches | NO | LOCAL_REJECTED | CORRECTNESS_ONLY | do not promote to online |
| MIX-A/V006 | PASS | NOT_RUN | NO | NEEDS_ONE_MORE_LOCAL | COMPILE_ONLY | not submitted |

## Required pattern for future rows

When a candidate claims local win:

```text
paired_latency + local_proxy_score + online Official
→ MATCH_POSITIVE | MATCH_NEGATIVE | CONTRADICT | NO_COMPARABLE_LOCAL_DATA
```

Until paired local numbers exist for champion routes, proxy remains `NO_DATA`.

## Main-1 historical calibration update (2026-09-26)

The Main-1 ledger covers 53 revisions across eight routes and one explicit numbering-gap placeholder. Its 34 retained Official submissions reconcile to 28 passing scores and six invalid outcomes: one compile error, three runtime errors, and two wrong answers.

All 34 submissions now have a calibration row. Thirty-three have no usable local performance result. R31A V017 is the only recorded local proxy: its single unpaired FP32 case was 4.33% faster, while its Official score fell from 45.00 to 44.45. This is a false positive and does not qualify as a Local performance result. No repeated comparable local/Official samples exist, so the evaluator remains unchanged.

The calibration table retains its original eleven columns and adds direction, magnitude, context, timing quality, decision, false-positive, and false-negative fields. Existing non-Main-1 values remain unchanged; the added fields identify those rows as outside this audit.

### Main-1 local validation queue

| Priority | Route candidate | Direct parent | Next work |
|---|---|---|---|
| P0 | R31A V021 | V016 | Rebuild exact parent/candidate sources, qualify both binaries, then paired timing |
| P0 | R31B V016 | V011 | Repeat exact-shape qualification and paired timing under a clean device lease |
| P0 | MIX-A V007 | V003 | Verify parent runner and kernel identities, qualify binaries, then paired timing |
| P0 | WIDE-X-FRESH4 V001 | BUILD-FIX-001 | Prove the parent control executable, then qualify and pair |
| P0 | MODE-X-R015C r4 | r3 | Reconcile the r4 scheduler entry with the r3 CURRENT pointer, verify both binaries, then qualify and pair |
| P0 | DTYPE-SPECIAL-X V001 | R31B V011 | Qualify the 24 in-domain shapes, then pair under a fresh exclusive lease |
| P1 | None additional |  | Each current candidate's direct parent is included in its P0 comparison |
| P2 | No missing calibration rows |  | Preserve missing or invalid local data labels; do not rerun the full historical set |
| P3 | Retired historical versions |  | No rerun selected; ASYNC-TRIPLE-X remains collision-frozen and EXT-ASCEND-X retains its correctness failure |

No live server3 device query or performance timing was run for this update. The queue records evidence work only; it does not reserve a device.
