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
