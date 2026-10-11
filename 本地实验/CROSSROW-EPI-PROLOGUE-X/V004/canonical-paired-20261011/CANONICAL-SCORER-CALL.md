# V004 pinned Canonical scorer call

SCORER: `MAIN2-CANONICAL-V1` pure scorer
SCORER_SHA256: `f43709695115da53e30e58204f8e7c4716486538397394d7e2cb441f3a1d6d7f`
SUITE: `CANONICAL_LOCAL_SUITE_V1`
SUITE_SHA256: `30b6edcd59f19f3e495bb23cb5cf2f41bf5e132a6d96f7b050e32fb6983b0a95`
CALL_SCOPE: route-local pure `scoreVectors`/geometric aggregation only; the shared `scoreboard` command was not run.
CALL_COMMAND: `node --input-type=module -e '<route-local parser of the 24 V004 raw TSVs>; import scoreVectors from pinned main2-canonical-scorer.mjs'`
CALL_RC: `0`

Formula:

```text
100 * exp(mean(log(anchor_median_us / candidate_median_us)))
```

Result:

```text
CORE_SCORE=57.186564
DELTA_VS_R31B_V011=-42.813436
PAIRED_PARENT_DIAGNOSTIC_SCORE=111.665582
CORE_GAIN_COUNT=1/4
CONSISTENT_GAIN=NO
QUALITY=POOR
STATUS=LOCAL_NEUTRAL
RELIABLE_LOCAL_GAIN=NO
LOCAL_BEST=UNCHANGED
NO_PROMOTION=YES
```

The scorer input was complete: four Core cases, six side runs per case, 31 device-event rows per side, all `RC=0`, all `bad=0`. C13 was the only candidate case whose median beat its frozen anchor; C12/C14/C16 remained slower. Parent/Candidate source and artifact identities are recorded in `RESULT-ROUTE-LOCAL.md` and `MEASUREMENT-INDEX.tsv`.

The device snapshots report NPU7 HBM usage around 37% before/after. AICore/AIVector and unsupported proc-query results are retained as observations; no process-clean claim is made.
