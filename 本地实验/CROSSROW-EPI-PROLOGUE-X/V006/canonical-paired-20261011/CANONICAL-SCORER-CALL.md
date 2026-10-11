# V006 pinned Canonical scorer call

SCORER: `MAIN2-CANONICAL-V1` pure `scoreVectors`
SCORER_SOURCE_COMMIT: `6d3ad0b30b21d2462b0fd8716c884f0ff6bdaabb`
SCORER_SHA256: `f43709695115da53e30e58204f8e7c4716486538397394d7e2cb441f3a1d6d7f`
SUITE: `CANONICAL_LOCAL_SUITE_V1`
ANCHOR: `R31B/V011`, source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
CALL_SCOPE: route-local pure scoring only; Main2 `scoreboard` was not run and no shared scoreboard/metrics were modified.
CALL_RC: `0`

Formula: `100 * exp(mean(log(anchor_median_us / candidate_median_us)))` over C12/C13/C14/C16. Run medians are the median of all 31 device-event samples; case medians are the median of the three run medians. No samples were removed or filtered.

CORE_SCORE: `66.347725`
DELTA_VS_R31B_V011: `-33.652275`
PAIRED_PARENT_DIAGNOSTIC_SCORE: `100.671503`
CORE_GAIN_COUNT: `1/4` (C14 only)
CONSISTENT_GAIN: `NO`
QUALITY: `POOR`
EMPIRICAL_LOW: `27.989780`
EMPIRICAL_HIGH: `157.272427`
LOCAL_VERDICT: `LOCAL_NEUTRAL`
RELIABLE_LOCAL_GAIN: `NO`
LOCAL_BEST: `UNCHANGED`
NO_PROMOTION: `YES`

The scorer input is complete: four Core cases, six valid side runs per case, 31 device-event samples per side, all 24 `RC=0`, `bad=0`. The temporary `/tmp/w5-r01-canonical.Mk9W1S/V003` directory is only the existing runner staging path; its V006-bound Candidate artifact identity is recorded in the preflight and measurement index.
