# V005 pinned Canonical scorer call

SCORER: `MAIN2-CANONICAL-V1` pure `scoreVectors`
SCORER_SHA256: `f43709695115da53e30e58204f8e7c4716486538397394d7e2cb441f3a1d6d7f`
SUITE_SHA256: `30b6edcd59f19f3e495bb23cb5cf2f41bf5e132a6d96f7b050e32fb6983b0a95`
CALL_SCOPE: route-local pure scoring only; shared `scoreboard` was not run.
CALL_RC: `0`

Formula: `100 * exp(mean(log(anchor_median_us / candidate_median_us)))`

CORE_SCORE: `66.438931`
DELTA_VS_R31B_V011: `-33.561069`
PAIRED_PARENT_DIAGNOSTIC_SCORE: `115.952791`
CORE_GAIN_COUNT: `1/4`
CONSISTENT_GAIN: `NO`
QUALITY: `POOR`
STATUS: `LOCAL_NEUTRAL`
RELIABLE_LOCAL_GAIN: `NO`
LOCAL_BEST: `UNCHANGED`
NO_PROMOTION: `YES`

The scorer input is complete: 4 Core cases, 6 valid side runs per case, 31 device-event samples per side, all `RC=0`, `bad=0`. Candidate medians are C12/C13/C14/C16=`22.30/27.56/25.68/17.40 us`; only C14 beats its frozen anchor. Parent/Candidate SHA and artifact identity are in `MEASUREMENT-INDEX.tsv`.

NPU7 pre/post HBM usage was about 37%. AICore/AIVector, bandwidth and unsupported proc-query results are retained as load context; no process-clean claim is made.
