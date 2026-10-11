# V007 c09344 pinned Canonical scorer call

SCORER: `MAIN2-CANONICAL-V1` pure `scoreVectors`
SCORER_SOURCE_COMMIT: `6d3ad0b30b21d2462b0fd8716c884f0ff6bdaabb`
SCORER_SHA256: `f43709695115da53e30e58204f8e7c4716486538397394d7e2cb441f3a1d6d7f`
SUITE: `CANONICAL_LOCAL_SUITE_V1`
ANCHOR: `R31B/V011`, source SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
CALL_SCOPE: pinned pure scoring function only; shared Main2 scoreboard/metrics were not modified.
CALL_RC: `0`

Formula: `100 * exp(mean(log(anchor_median_us / candidate_median_us)))` over C12/C13/C14/C16. Run medians use all 31 device-event samples; case medians are the median of three run medians. No sample trimming or filtering.

CORE_SCORE: `72.149204`
DELTA_VS_R31B_V011: `-27.850796`
PAIRED_PARENT_DIAGNOSTIC_SCORE: `118.369252`
CORE_GAIN_COUNT: `1/4` (C14 only)
CONSISTENT_GAIN: `NO`
MEASUREMENT_QUALITY: `POOR`
EMPIRICAL_LOW: `25.531372`
EMPIRICAL_HIGH: `203.886713`
LOCAL_VERDICT: `LOCAL_NEUTRAL`
RELIABLE_LOCAL_GAIN: `NO`
LOCAL_BEST: `UNCHANGED`
NO_PROMOTION: `YES`

Input checks: 4 Core cases; 6 side runs per case; 24/24 `RC=0`, `bad=0`; 24/24 raw files contain 31 positive finite event samples; runner identity matches `2aa1aa3b…`. Parent and Candidate are separate executables; `SAME_BINARY_QUALIFICATION=NOT_RUN`.
