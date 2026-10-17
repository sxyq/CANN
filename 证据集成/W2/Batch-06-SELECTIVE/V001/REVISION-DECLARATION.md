ROUTE=SELECTIVE-FASTPATH-CHAMPION-X
REVISION=V001
DIRECT_PARENT=R31B V011
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE=45.16 Official
SINGLE_HYPOTHESIS=Dispatch the complete STORE-EPILOGUE-X V003 wide-FP32 writeback implementation only for exact rank-2 inputs [8,16384], [1,32768], and [1,16384]; every other input retains the R31B V011 execution path.
CONTEXT_CLASS=SELECTIVE_HISTORICAL_DERIVED
DONOR_SOURCE=STORE-EPILOGUE-X V003
DONOR_SOURCE_SHA256=0cdef265459d4683a1813593a881a5cf25ae75246aab49279d121896b71184ca
ALLOWLIST_DTYPE=FP32
ALLOWLIST_EXACT_SHAPES=[[8,16384],[1,32768],[1,16384]]
CORRECTNESS_ONLY_FALLBACK_CONTROL=FP32 [2,16384]; must dispatch to the original R31B V011 ProcessWideFp32FullCacheRows helper; no performance measurement; not a donor target.
FALLBACK_SOURCE=R31B V011; SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
LOCAL_EVIDENCE_REFERENCE=STORE V003 measurements compare V003 with STORE V002 (source SHA256 59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839); they are not V011-versus-V001 evidence.
OFFICIAL_EVIDENCE=STORE V003 44.38; its Official anchor 45.07; R31B V011 45.16. Judge records do not map testcase IDs to input shapes or dtypes.
WHY_NOT_DUPLICATE=CASE47 and TINY published hypotheses change other direct code paths. Their mechanisms differ from wide-FP32 output writeback; Official workload overlap remains unknown. This Revision has one donor dispatch only and excludes all 2-row inputs.
