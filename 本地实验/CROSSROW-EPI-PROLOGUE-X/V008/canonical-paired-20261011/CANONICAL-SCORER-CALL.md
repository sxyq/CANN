# V008 pinned canonical scorer call

SCORER: `MAIN2-CANONICAL-V1` published pure `scoreVectors`  
SCORER_SOURCE_COMMIT: `6d3ad0b30b21d2462b0fd8716c884f0ff6bdaabb`  
SCORER_SHA256: `f43709695115da53e30e58204f8e7c4716486538397394d7e2cb441f3a1d6d7f`  
SUITE: `CANONICAL_LOCAL_SUITE_V1`  
CALL_RC: `0`

Invocation used Node ESM stdin to import the existing published scorer module and its existing assets module from `/tmp/w5-r01-canonical.Mk9W1S/score-root/工具/`. It called the existing `frozenSuite`, `summarizeRuns`, and pure `scoreVectors` exports; input rows came from this V008 `MEASUREMENT-INDEX.tsv`, its raw/stats files, and the published R31B/V011 canonical vector. It verified all 24 raw SHA entries before scoring. The `scoreboard` function/command was not invoked, and no Main2 scoreboard or metrics file was written.

OUTPUT: `SCORER-OUTPUT.json`  
CORE_SCORE: `54.312261`  
DELTA_VS_R31B_V011: `-45.687739`  
PAIRED_PARENT_DIAGNOSTIC_SCORE: `87.528380`  
MEASUREMENT_QUALITY: `POOR`  
LOCAL_VERDICT: `LOCAL_NEUTRAL`  
NO_PROMOTION: `YES`
