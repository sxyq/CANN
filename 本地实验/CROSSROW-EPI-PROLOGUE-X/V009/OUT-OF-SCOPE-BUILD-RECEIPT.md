# V009 out-of-scope build receipt

ROUTE: CROSSROW-EPI-PROLOGUE-X
AGENT_ID: W5-R01/Zeno
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
STATUS: OUT_OF_SCOPE_BUILD_ONLY
REVIEW_SOURCE: Main-2 read-only review, current task

## Preserved source attempt

SOURCE_COMMIT: 932bc3833244ed287f98d3d5c50eb2105093e6fe
CANDIDATE_SOURCE_SHA256: d8428ff7956242481ca5d2947ccd82934a9a007b13033e66a7d59db47677a473
PARENT: R31B/V011
PARENT_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CHANGE: two adjacent `SetL2CacheHint(CACHE_MODE_DISABLE)` calls for `xGm_` and `residualGm_` in `Init()`.
REVIEW_DISPOSITION: Main-2 identified this as R4's GM-L2 cache-policy mechanism and outside the R1 CROSSROW-EPI-PROLOGUE-X scope. The candidate is retained unchanged as a real out-of-scope attempt; it is not accepted as an R1 hypothesis.

## Build evidence

BUILD_COMMAND: `source /usr/local/Ascend/ascend-toolkit/set_env.sh`; export the existing HCC 7.3.0 include paths as `CPLUS_INCLUDE_PATH` and `CPATH`; `cmake --build /tmp/w5-r01-canonical.Mk9W1S/V003/build --clean-first --target clx_ref_parent_probe clx_ref_candidate_probe --parallel 1`
BUILD_RC: 0
BUILD_LOG: `V009/BUILD.log`
BUILD_LOG_SHA256: 1b692a327a91a60dca68b828bc0bf373e49d60386a896d0bbb2616c2873cf889
PARENT_ARTIFACT_SHA256: e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64
CANDIDATE_ARTIFACT_SHA256: 2bce29e1b4f93a5530da99c4269da9413f20267a1f6909ed704ce383ad8b5022
RUNNER_REF_SHA256: 2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7
HARNESS_NOTE: `/tmp/.../V003` is the pre-existing canonical CMake/adapter staging directory, not a Candidate revision or a second runner.

CORRECTNESS: NOT_RUN (per Main-2 review; no correctness files created)
LOCAL: NOT_RUN (per Main-2 review; no device timing started)

## Selector restoration and scope decision

During Build only, the existing `submission.asc` selector pointed to the committed V009 source. It has been restored to its exact prior symlink target, `/home/data4t2/lelinfeng/cann-w5-r01-crossrow/本地实验/CROSSROW-EPI-PROLOGUE-X/V005/../V004/Candidate.asc`, whose SHA256 remains `642b2ae9482ea14e6cde8ed04b457cffc0f1ae2c2d52b242128e85fe36cfd994`. V004 has no tracked diff. No V008 evidence or raw was modified.

R1_SCOPE_REVIEW: The declared generic cached-row prologue is in `V009/Candidate.asc`, `Process()`, lines 345-351. V001 and V002 already tested moving this same guarded x/residual prologue to two distinct windows (post-pass-1/pre-epilogue, then between epilogue tiles). Another nearby movement of this same block would repeat the existing MTE2 prologue-placement mechanism, not establish a distinct causal OFAT. The remaining examined R1 changes V003-V008 concern different paths/mechanisms; reusing them would not be a new cross-row-prologue hypothesis. No sufficiently supported, distinct in-scope R1 hypothesis is selected here.

NEXT_ACTION: Planning decision required on whether to broaden or redirect the R1 mechanism. Do not run correctness/local on this Candidate and do not create V010 absent a route-scope decision.
